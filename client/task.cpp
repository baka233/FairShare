#include "fileindex.h"
#include <fstream>
#include <sstream>
#include <cstring>
#include <QCryptographicHash>
#include <sstream>
#include <QVariantMap>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QObject>
#include <QEventLoop>
#include <QTcpServer>
#include <QMutexLocker>
#include <QHostInfo>
#include "task.h"
#include "worker.h"


Task::Task(QString _file_path, QString _info, Task::TaskStartStatus _type, QObject* parent)
    : QObject(parent), file_path(_file_path), type(_type)
{
    add_time = QDateTime::currentDateTime();
    net_manager = std::make_shared<QNetworkAccessManager>();
    status = TaskStatus::VERIFYING;
    switch(type) {
    case Task::UPLOAD:
        description = _info;
        upload();
        break;
    case Task::DOWNLOAD:
        md5sum = _info;
        download();
        break;
   	default:
        break;
    }
}

Task::~Task()
{
    if (server != nullptr) {
        server->close();
        delete server;
    }
    delete worker;
}

void Task::upload()
{
    std::ifstream file_stream(file_path.toStdString());
    QFile f(this->file_path);
    QFileInfo finfo(f);
    this->filename = finfo.fileName();
    qDebug() << "filename is " << finfo.fileName();

    file_stream.seekg(0, std::ios_base::end);
    size = file_stream.tellg();
    block_size = (size + (per_block_size) - 1) / (per_block_size);
    downloaded_size = block_size;
    qDebug() << "size is " << size;

    file_verify_worker = new FileVerifyWorker(file_path, this);
    qRegisterMetaType<std::shared_ptr<FileIndex>>("std::shared_ptr<FileIndex>");
    QObject::connect(file_verify_worker, &FileVerifyWorker::sum_calculated, this, [=] (QString md5sum, std::shared_ptr<FileIndex> index) {
        qDebug() << "connect thread id is " << QThread::currentThreadId();
        this->md5sum = md5sum;
        this->index = index;
        this->downloaded_size = block_size;
        for (uint64_t i = 0; i < block_size; i++) {
            this->bitmap.push_back(BLOCK_FULL);
            this->downloaded_blocks.push_back(i);
        }
        emit upload_prepared();
        //delete file_verify_worker;
        if (put_info() != 0) {
            this->status = TaskStatus::FAILED;
            return;
        } else {
            this->status = TaskStatus::SEEDING;
        }
        start();
    });
    file_verify_worker->start();

    this->status = TaskStatus::READY;
}

bool Task::check_block(uint64_t block_num, QByteArray& bytes)
{
    qDebug() <<  "real bytes is " << index->get(block_num) << " check bytes is " << bytes;
    if (index->get(block_num) == bytes) {
        return true;
    } else {
        return false;
    }
}



bool Task::pull_ip_list()
{
    QJsonObject torrent;
    QJsonDocument document;
    QNetworkRequest request;

    QNetworkReply* reply;
    request.setUrl(QUrl(base_url + "/seeder/get/" + md5sum));
    reply = net_manager->get(request);

    // 创建事件循环
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    QByteArray bytes = reply->readAll();
    qDebug() << "bytes is :" << bytes;
    QJsonParseError err_rpt;
    QJsonDocument reply_Doc = QJsonDocument::fromJson(bytes, &err_rpt);
    if (err_rpt.error != QJsonParseError::NoError)
    {
        qDebug() << "json解析错误:" << err_rpt.errorString();
        status = TaskStatus::FAILED;
        return -1;
    }

    QJsonObject reply_json_object = reply_Doc.object();
    if (reply_json_object.value("code").toInt() != 200) {
        QString cause = reply_json_object.value("message").toString();
        qDebug() << "获取数据失败，原因为:" << cause;
        status = TaskStatus::FAILED;
        return -1;
    } else {
        auto object = reply_json_object.value("data").toArray();
        seeders.clear();
        for (const auto& each : object) {
            auto tmp = each.toObject();
            seeders.insert(std::make_pair(tmp.value("ip").toString(), tmp.value("port").toInt()));
        }
        qDebug() << "size is " << size;
    }
    return 0;
}

bool Task::set_block_full(uint64_t block)
{
    QMutexLocker lock(&bitmap_lock);
    if (bitmap[block] != BLOCK_FULL) {
        bitmap[block] = BLOCK_FULL;
        downloaded_size++;
        downloaded_blocks.push_back(block);
        qDebug() << "已下载块" << downloaded_size << "/" << block_size;
        if (downloaded_size == block_size) {
            finish_time = QDateTime::currentDateTime();
            status = TaskStatus::SEEDING;
            emit status_change();
        }
        return true;
    } else {
        return false;
    }
}


bool Task::try_to_check_block(uint64_t block_num)
{
    QMutexLocker locker(&bitmap_lock);
    if (bitmap[block_num] == BLOCK_EMPTY) {
        bitmap[block_num] = BLOCK_LOCK;
        return true;
    } else {
        return false;
    }
}


void Task::start()
{
    if (status != TaskStatus::READY && status != TaskStatus::PAUSED) {
        return;
    }
    if (downloaded_size < block_size) {
        pull_ip_list();
        if (worker == nullptr) {
            QThread* thread = new QThread;
            worker = new DownloadWorker(this);
            worker->moveToThread(thread);
            connect(thread, &QThread::started, worker, &DownloadWorker::start);
            thread->start();
        }
        status = TaskStatus::DOWNLOADING;
        emit status_change();

    } else {
        status = TaskStatus::SEEDING;
        emit status_change();
    }
}


void Task::pause()
{
    if (status != TaskStatus::FAILED) {
        emit status_change();
        status = TaskStatus::PAUSED;
    }
}

int Task::put_info()
{
    QJsonObject torrent;
    QJsonDocument document;
    QNetworkRequest request;
    QFile file(file_path);

    torrent.insert("hash", md5sum);
    torrent.insert("filename", filename);
    torrent.insert("description", description);
    torrent.insert("filesize",  QString::number(size));
    torrent.insert("index", index->get_hex());
    document.setObject(torrent);

    qDebug() << "torrent is " <<  torrent;


    QNetworkReply* reply;
    request.setUrl(QUrl(base_url + "/tracker/upload"));
    request.setRawHeader("Content-Type", "application/json");
    reply = net_manager->post(request, document.toJson());

    // 创建事件循环
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    QByteArray bytes = reply->readAll();
    qDebug() << "bytes is :" << bytes;
    QJsonParseError err_rpt;
    QJsonDocument reply_Doc = QJsonDocument::fromJson(bytes, &err_rpt);
    if (err_rpt.error != QJsonParseError::NoError)
    {
        qDebug() << "json解析错误:" << err_rpt.errorString();
        status = TaskStatus::FAILED;
        return -1;
    }

    QJsonObject reply_json_object = reply_Doc.object();
    if (reply_json_object.value("code").toInt() != 200) {
        QString cause = reply_json_object.value("message").toString();
        qDebug() << "插入数据失败，原因为:" << cause;
        status = TaskStatus::FAILED;
        return -1;
    }
    this->status = TaskStatus::READY;
    return 0;
}


int Task::pull_info()
{
    // need implement
    QJsonObject torrent;
    QJsonDocument document;
    QNetworkRequest request;

    QNetworkReply* reply;
    request.setUrl(QUrl(base_url + "/tracker/get/" + md5sum));
    reply = net_manager->get(request);

    // 创建事件循环
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    QByteArray bytes = reply->readAll();
    qDebug() << "bytes is :" << bytes;
    QJsonParseError err_rpt;
    QJsonDocument reply_Doc = QJsonDocument::fromJson(bytes, &err_rpt);
    if (err_rpt.error != QJsonParseError::NoError)
    {
        qDebug() << "json解析错误:" << err_rpt.errorString();
        status = TaskStatus::FAILED;
        return -1;
    }

    QJsonObject reply_json_object = reply_Doc.object();
    if (reply_json_object.value("code").toInt() != 200) {
        QString cause = reply_json_object.value("message").toString();
        qDebug() << "获取数据失败，原因为:" << cause;
        status = TaskStatus::FAILED;
        return -1;
    } else {
        QJsonObject object = reply_json_object.value("data").toObject();
        filename = object.value("filename").toString();
        description = object.value("description").toString();
        size = object.value("filesize").toString().toLongLong();
        qDebug() << "size is " << size;
        block_size = (size + ((per_block_size) - 1)) / (per_block_size);
        index = std::make_shared<FileIndex>(QByteArray::fromStdString(object.value("index").toString().toStdString()), (uint64_t)block_size);
    }
    this->status = TaskStatus::READY;
    return 0;
}

void Task::remove_block_lock(uint64_t block_num)
{
    QMutexLocker lock(&bitmap_lock);
    if (bitmap[block_num] == BLOCK_LOCK) {
        bitmap[block_num] = BLOCK_EMPTY;
    }
}



void Task::download()
{
    if (pull_info() != 0) {
        this->status = TaskStatus::FAILED;
        return;
    }
    while (true) {
        qDebug() << "file_path is " <<  file_path + "/" + filename;
        if (QFile(file_path + "/" + filename).exists() == false) {
            file_path = file_path + "/" + filename;
            QFile file(file_path);
            file.open(QIODevice::WriteOnly);
            if (file.resize(size) == false) {
                qDebug() << "file resize failed";
                this->status = TaskStatus::FAILED;
                return;
            }
            file.close();
            break;
        }
        filename += ".new";
    }
    block_size = (size + (per_block_size - 1)) / per_block_size;
    downloaded_size = 0;
    for (uint64_t i = 0; i < block_size; i++) {
        bitmap.push_back(BLOCK_EMPTY);
    }
    qDebug() << "prepare download to" << file_path;
    this->status = TaskStatus::READY;
    start();
}

QString Task::get_speed_string(double num) const
{
    double tmp = num;
    QString table[] = {"B/s", "KB/s", "MB/s", "GB/s"};
    for (size_t i = 0; i < 3; i++) {
        if ((uint64_t)tmp / 1024 == 0) {
            if ((uint64_t)tmp % 1024 >= 768) {
                return QString(QString::number(tmp/1024) + table[i+1]);
    	    } 
    	    else {
                return QString(QString::number(tmp) + table[i]);

    	    }
    	}
        tmp /= 1024;
    }
    return QString(QString::number(tmp) + table[3]);

}

QString Task::get_size_beautify() const
{
    return size_beautify(this->size);
}

QString Task::size_beautify(uint64_t _size) const
{
    double tmp = _size;
    QString table[] = {"B", "KB", "MB", "GB"};
    for (size_t i = 0; i < 3; i++) {
        if ((uint64_t)tmp / 1024 == 0) {
            if ((uint64_t)tmp % 1024 >= 768) {
                return QString(QString::number(tmp/1024) + table[i+1]);
            }
            else {
                return QString(QString::number(tmp) + table[i]);
            }
        }
        tmp /= 1024;
    }
    return QString(QString::number(tmp) + table[3]);
}

QString Task::get_status_string() const {
    switch(this->status) {
    case DOWNLOADING:
        return "下载中";
    case SEEDING:
        return "作种中";
    case VERIFYING:
        return "校验中";
    case PAUSED:
        return "暂停";
    case FAILED:
        return "任务失败";
    case READY:
        return "就绪";
    default:
        break;
    }    
    return "";
}


const QVector<uint64_t>& Task::get_full_blocks()
{
    return downloaded_blocks;
}


bool Task::get_block(uint64_t block_num, uint32_t* read_num, char* array)
{
    bitmap_lock.lock();
    if (static_cast<int>(block_num) >= bitmap.length()) {
        return false;
    }

    if (bitmap[block_num] != BLOCK_FULL) {
        return false;
    }
    bitmap_lock.unlock();

    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "open file failed";
        return false;
    }

    file.seek(block_num * per_block_size);
    *read_num = file.read(array, per_block_size);
    return true;
}

 QString Task::get_remain_time()
 {
     QString ans = "";
     if (current_download_speed == 0) {
         return "未知";
     }
     if (size < real_readed && downloaded_size < size) {
        return "未知";
     } else {
        uint64_t sec = (size -real_readed) / current_download_speed;
        ans = QString::number(sec % 60) + "秒" + ans;
        if (sec / 60 == 0) {
            return ans;
        }
        sec /= 60;
        ans = QString::number(sec % 60) + "分" + ans;
        if (sec / 60 == 0) {
            return ans;
        }
        sec /= 60;
        ans = QString::number(sec % 24) + "时" + ans;
        if (sec / 24 == 0) {
            return ans;
        }
        sec /= 24;
        ans = QString::number(sec % 24) + "天" + ans;
        if (ans > 30) {
            return "超过一个月";
        }
        return ans;
     }
 }


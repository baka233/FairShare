#include "downloadworkerconnection.h"
#include <QFile>
#include <QDataStream>
#include <QEventLoop>
#include "task.h"

DownloadWorkerConnection::DownloadWorkerConnection(Task* _task, std::pair<QString, int> _address, QTcpSocket* _socket, QObject *parent)
    : QObject(parent), task(_task), socket(_socket), address(_address)
{

}

void DownloadWorkerConnection::process()
{
    process_connect(socket);
    socket->disconnectFromHost();
    delete socket;
}

char* DownloadWorkerConnection::read_until(QTcpSocket* socket, uint64_t size, bool is_download)
{
    char* bytes = new char[size];
    uint64_t real = 0;
    while (real < size) {
        char* tmp = bytes + real;
        uint64_t to_read = size - real ;
        socket->waitForReadyRead(3000);
        uint64_t readed = socket->read(tmp, to_read);
        real += readed;
        if (is_download) {
            task->add_readed(readed);
        }
        qDebug() << "real is " << real;
    }
    return bytes;
}




void DownloadWorkerConnection::process_connect(QTcpSocket *socket)
{
    auto hash = task->get_md5().toUtf8();
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(1000 * 2);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(task, &Task::status_change, &loop, &QEventLoop::quit);
    while (true) {
        auto status = task->get_status();
        if (status != Task::TaskStatus::DOWNLOADING) {
            return;
        }

        QVector<uint64_t> active_list;
        request_bitmap(active_list, hash, socket);
        for (const auto each : active_list) {
            auto status = task->get_status();
            if (status != Task::TaskStatus::DOWNLOADING) {
                return;
            }
            if (task->try_to_check_block(each)) {
                if (request_block(hash, each, socket) == true) {
                    task->set_block_full(each);
                } else {
                    task->remove_block_lock(each);
                }
            }
        }
        // try to sleep 2 seconds to avoid too many request
        timer.start();
        loop.exec();
        timer.stop();
    }
}

bool DownloadWorkerConnection::request_block(QByteArray& hash, uint64_t block_num, QTcpSocket* socket)
{
    packet _packet;
    QCryptographicHash md(QCryptographicHash::Md5);
    _packet.action_type = ACTION_DOWNLOAD;
    _packet.hash 		= hash;
    _packet.packet_num  = block_num;
    _packet.offset		= 0;
    _packet.length		= 0;
    send_packet(socket, _packet);
    packet reply_packet = receive_packet(socket);

    switch (reply_packet.action_type)
    {
    case ACTION_UPLOAD: {
        QFile file(task->file_path);
        file.open(QIODevice::ReadWrite);
        char* array_raw = read_until(socket, reply_packet.length, true);
        auto array = QByteArray::fromRawData(array_raw, reply_packet.length);
        size_t size = array.length();
        if (array.length() < static_cast<int>(per_block_size)) {
            array.append(per_block_size - array.length(), '\0');
        }
        md.addData(array);
        auto result = md.result().toHex();
        if (task->check_block(block_num, result) == false) {
            delete[] array_raw;
            task->sub_real_readed(size);
            file.close();
            return false;
        }
        if (file.seek(per_block_size * block_num) == false) {
            qDebug() << "seek failed";
        }
        qDebug() << "seek at " << per_block_size * block_num << " size is " << size;
        file.write(array, size);
        file.close();
        delete[] array_raw;
        break;
    }
    default: {
        return false;
    }
    }
    return true;
}

void DownloadWorkerConnection::request_bitmap(QVector<uint64_t>& data, QByteArray& hash, QTcpSocket* socket)
{
    packet _packet;
    _packet.action_type = ACTION_BITMAP;
    _packet.hash		= hash;
    _packet.offset		= 0;
    _packet.length		= 0;
    send_packet(socket, _packet);
    packet reply_packet = receive_packet(socket);

    switch(reply_packet.action_type)
    {
    case REPLY_BITMAP: {
        char* array_raw = read_until(socket, reply_packet.length, false);
        auto array = QByteArray::fromRawData(array_raw, reply_packet.length);
        QDataStream stream(array);
        while (!stream.atEnd()) {
            unsigned long long tmp;
            stream >> tmp;
            data.push_back(tmp);
        }
        delete[] array_raw;
        break;
    }
    default:
        break;
    }
}

packet DownloadWorkerConnection::receive_packet(QTcpSocket* socket)
{
    char* buffer = new char[32];
    uint size = 32;
    char* request_raw = read_until(socket, packet_size, false);
    auto request = QByteArray::fromRawData(request_raw, packet_size);
    qDebug() << "request length is " << request.length() << "size of packet is " ;
    QDataStream request_stream(request);
    packet _packet;
    request_stream >> _packet.action_type;
    request_stream.readRawData(buffer, size);
    QByteArray hash(buffer, size);
    delete[] buffer;
    request_stream >> _packet.packet_num >>  _packet.offset >> _packet.length;
    qDebug() << "action_type: " << _packet.action_type
             << "task_hash:   " << hash
             << "packet_num:  " << _packet.packet_num
             << "offset:  	  " << _packet.offset
             << "length:      " << _packet.length;
    delete[] request_raw;
    return _packet;
}

void DownloadWorkerConnection::send_packet(QTcpSocket* socket, packet _packet)
{
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::ReadWrite);
    qDebug() << "_packet.hash is " << _packet.hash << "_packet.length is " << _packet.length;
    out << _packet.action_type;
    qDebug() << "now is 0x" << bytes.toHex();
    out.writeRawData(_packet.hash.constData(), 32);
    qDebug() << "now is 0x" << bytes.toHex();
    out << _packet.packet_num << _packet.offset << _packet.length;
    qDebug() << "send_packet header is 0x" <<  bytes.toHex();
    socket->write(bytes);
}

#include "searchtable.h"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QEventLoop>
#include <QHeaderView>
#include <QMessageBox>
#include "task.h"

SearchTable::SearchTable(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout;

    net_manager = new QNetworkAccessManager(this);
    search_bar = new QLineEdit;
    search_button = new QPushButton(tr("搜索"));

    QHBoxLayout* hlayout = new QHBoxLayout;
    search_bar->setMaximumHeight(30);
    search_button->setMaximumWidth(80);
    search_button->setMaximumHeight(30);

    connect(search_bar, &QLineEdit::returnPressed, this, &SearchTable::search);
    connect(search_button, &QPushButton::clicked, this, &SearchTable::search);

    hlayout->addWidget(search_bar);
    hlayout->addWidget(search_button);

    table = new QTableView();
    model = new QStandardItemModel();
    QStringList labels = QObject::tr("文件名,文件大小,文件校验码,在线人数, 下载").simplified().split(",");
    model->setHorizontalHeaderLabels(labels);

    layout->addLayout(hlayout);
    layout->addWidget(table);

    table->setModel(model);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    this->setLayout(layout);
}

void SearchTable::search()
{
    QString search_str = search_bar->text();

    QJsonObject torrent;
    QJsonDocument document;
    QNetworkRequest request;

    QNetworkReply* reply;
    request.setUrl(QUrl(base_url + "/tracker/search/" + search_str));
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
        model->removeRows(0, model->rowCount());
        return;
    }

    QJsonObject reply_json_object = reply_Doc.object();
    if (reply_json_object.value("code").toInt() != 200) {
        QString cause = reply_json_object.value("message").toString();
        qDebug() << "获取数据失败，原因为:" << cause;
        model->removeRows(0, model->rowCount());
    } else {
        auto array = reply_json_object.value("data").toArray();
        uint64_t i = 0;
        model->removeRows(0, model->rowCount());
        for (const auto& each : array) {
            auto tmp = each.toObject();
            auto ans = get_size_beautify(tmp.value("filesize").toString().toLongLong());
            auto hash = tmp.value("hash").toString();
            QPushButton* downloadButton = new QPushButton(tr("下载"));
            connect(downloadButton, &QPushButton::clicked, [=] {
                emit download_event(hash);
            });
            model->setItem(i, 0, new QStandardItem(tmp.value("filename").toString()));
            model->setItem(i, 1, new QStandardItem(ans));
            model->setItem(i, 2, new QStandardItem(hash));
            model->setItem(i, 3, new QStandardItem(tmp.value("count").toString()));
            table->setIndexWidget(model->index(i, 4), downloadButton);
            i++;
        }
    }
}


QString SearchTable::get_size_beautify(uint64_t size) const
{
    double tmp = size;
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




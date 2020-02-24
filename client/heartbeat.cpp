#include "heartbeat.h"
#include <QTcpServer>
#include <QObject>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHostInfo>
#include <QEventLoop>
#include <QNetworkReply>

HeartBeat::HeartBeat(QTcpServer* _server, std::shared_ptr<TaskList> _list)
    : server(_server), list(_list)
{

}

void HeartBeat::start()
{
    timer = new QTimer;
    timer->setInterval(1000 * 3);
    QObject::connect(timer, &QTimer::timeout, [=] {
        send_heart_beat();
        timer->start();
    });

    timer->start();
    net_manager = new QNetworkAccessManager;
}

void HeartBeat::send_heart_beat()
{
    QJsonObject heart_packet;
    QJsonDocument document;
    QNetworkRequest request;

    QHostInfo info = QHostInfo::fromName(QHostInfo::localHostName());
    QHostAddress to_address;

    // 找出一个IPv4地址即返回
    foreach(QHostAddress address,info.addresses())
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol)
        {
            to_address = address;
            break;
        }
    }

    for (const auto& each : list->get_tasks()) {
        auto status = each->get_status();
        if (status != Task::TaskStatus::SEEDING
           && status != Task::TaskStatus::DOWNLOADING)
        {
            continue;
        }
        heart_packet.insert("hash", each->get_md5());
        heart_packet.insert("ip", to_address.toString());
        heart_packet.insert("port", server->serverPort());
        document.setObject(heart_packet);


        qDebug() << "heart_packet is " <<  heart_packet;


        QNetworkReply* reply;
        request.setUrl(QUrl(base_url + "/seeder/fresh"));
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
            return;
        }

        QJsonObject reply_json_object = reply_Doc.object();
        if (reply_json_object.value("code").toInt() != 200) {
            QString cause = reply_json_object.value("message").toString();
            qDebug() << "心跳包发送失败，原因为:" << cause;
            return;
        }
    }
    return;

}

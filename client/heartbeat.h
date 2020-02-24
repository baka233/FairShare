#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include <QTcpServer>
#include <memory>
#include <QtNetwork/QNetworkAccessManager>
#include "tasklist.h"


class HeartBeat
{
public:
    HeartBeat(QTcpServer* _server, std::shared_ptr<TaskList> _list);
    void start();
private:
    QString base_url = "http://127.0.0.1:8000";
    void send_heart_beat();
    QTcpServer* server;
    std::shared_ptr<TaskList> list;
    QTimer* timer;
    QNetworkAccessManager* net_manager;
};

#endif // HEARTBEAT_H

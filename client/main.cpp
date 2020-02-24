#include "mainwindow.h"

#include <QApplication>
#include <QObject>
#include <QDebug>
#include <QTcpServer>
#include "worker.h"
#include "tasklist.h"
#include "heartbeat.h"

QTcpServer* startServer(std::shared_ptr<TaskList> list, int& port, QVector<QTcpSocket*>* sockets)
{
    QTcpServer* server = new QTcpServer;
    if (server->listen(QHostAddress::AnyIPv4) == false)
    {
        qDebug() << "try to create server error";
        server->close();
        exit(-1);
    }
    HeartBeat* heart_beat = new HeartBeat(server, list);
    heart_beat->start();
    QObject::connect(server, &QTcpServer::newConnection, [=] {
        QTcpSocket* socket = server->nextPendingConnection();
        Worker* worker = new Worker(list, socket, Worker::UPLOADER);
        sockets->push_back(socket);
        worker->start();
    });
    port = server->serverPort();
    return server;
}


int main(int argc, char *argv[])
{
    int port = 8800;
    QVector<QTcpSocket*> sockets;
    QApplication a(argc, argv);
    std::shared_ptr<TaskList> list = std::make_shared<TaskList>();
    auto server = startServer(list, port, &sockets);
    MainWindow w(list);

    w.show();
    QObject::connect(&a, &QApplication::lastWindowClosed, &a, &QApplication::quit);
    int ret = a.exec();
    for (auto each : sockets) {
        each->disconnectFromHost();
    }
    server->close();
    return ret;
}

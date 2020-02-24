#include "downloadworker.h"
#include "task.h"
#include <QEventLoop>
#include <QObject>
#include <QDataStream>
#include <QThreadPool>
#include <QFile>


DownloadWorker::DownloadWorker(Task* _task, QObject *parent)
    : QObject(parent), task(_task)
{

}

void DownloadWorker::run()
{
    QEventLoop loop;
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(task, &Task::status_change, &loop, &QEventLoop::quit);


    while (true) {
        auto status = task->get_status();
        if (status == Task::TaskStatus::DOWNLOADING) {
            auto seeders = task->get_seeders();
            for (auto each : seeders) {
                if (connected.contains(each)) {
                    continue;
                }
                QTcpSocket* socket = new QTcpSocket();
                qDebug() << connect(socket, &QTcpSocket::connected, [=] {
                    this->connected.insert(each);
                    sockets.push_back(socket);
                    DownloadWorkerConnection* connect = new DownloadWorkerConnection(task, each, socket);
                    QThread* thread = new QThread;
                    connect->moveToThread(thread);
                    socket->moveToThread(thread);
                    QObject::connect(thread, &QThread::started, connect, &DownloadWorkerConnection::process);
                    thread->start();
                });
                socket->connectToHost(each.first, each.second);
            }
        } else {
            connected.clear();
        }
        if (status == Task::TaskStatus::SEEDING || status == Task::TaskStatus::DELETED)
        {
            break;
        }
        timer.setInterval(1000 * 3);
        timer.start();
        loop.exec();
    }
}





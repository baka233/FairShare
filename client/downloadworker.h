#ifndef DOWNLOADWORKER_H
#define DOWNLOADWORKER_H

#include <QObject>
#include <QThread>
#include <memory>
#include <QTcpSocket>
#include <QList>
#include "packet.h"
#include "downloadworkerconnection.h"

class Task;

class DownloadWorker : public QObject
{
    Q_OBJECT
public:
    explicit DownloadWorker(Task* _task, QObject *parent = nullptr);
    void run();
    QByteArray read_until(QTcpSocket* socket, uint64_t size, bool is_download);

private:
    Task* task = nullptr;
    QList<QTcpSocket*> sockets;
    QSet<std::pair<QString, int>> connected;
public slots:
    void start() {
        run();
    }


signals:

};

#endif // DOWNLOADWORKER_H

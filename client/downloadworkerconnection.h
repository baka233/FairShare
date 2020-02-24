#ifndef DOWNLOADWORKERCONNECTION_H
#define DOWNLOADWORKERCONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include "packet.h"

class Task;

class DownloadWorkerConnection : public QObject
{
    Q_OBJECT
public:
    explicit DownloadWorkerConnection(Task* _task, std::pair<QString, int> _address, QTcpSocket* _socket, QObject *parent = nullptr);

public slots:
    void process();

private:
    Task* task;
    QTcpSocket* socket;
    std::pair<QString, int> address;
    void process_connect(QTcpSocket* socket);
    void send_packet(QTcpSocket* socket, packet _packet);
    void request_bitmap(QVector<uint64_t>& data, QByteArray& hash, QTcpSocket* socket);
    packet receive_packet(QTcpSocket* socket);
    bool request_block(QByteArray& hash, uint64_t block_num, QTcpSocket* socket);
    char* read_until(QTcpSocket* socket, uint64_t size, bool is_download);




signals:

};

#endif // DOWNLOADWORKERCONNECTION_H

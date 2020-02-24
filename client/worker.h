#ifndef WORKER_H
#define WORKER_H


#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <memory>
#include "packet.h"

const uint64_t per_block_size = 1 << 22;
const uint64_t buffer_size = 8192;
const int packet_size = 48;

class TaskList;

class Worker : public QThread
{
    Q_OBJECT
public:

    enum action_role {
        UPLOADER,
        DOWNLOADER
    };

    explicit Worker(std::shared_ptr<TaskList> _tasklist, QTcpSocket* _socket, action_role _role, QObject *parent = nullptr);

    Worker() = delete;
    Worker(const Worker&) = delete;

    void process();

    void process_download(QByteArray& hash, uint64_t packet_num);
    void process_bitmap(QByteArray& hash);

private:
    void run();
    void send_heart_beat();
    void send_packet(packet _packet);
    char* read_until(uint64_t size);
    std::shared_ptr<TaskList> list;
    QTcpSocket* socket;
    action_role role;


signals:

};

#endif // WORKER_H

#include "worker.h"
#include "tasklist.h"
#include <QDataStream>
#include <QEventLoop>

Worker::Worker(std::shared_ptr<TaskList> _tasklist, QTcpSocket* _socket, action_role _role,  QObject *parent)
    : 	QThread(parent),
        list(_tasklist),
        socket(_socket),
        role(_role)
{
}

void Worker::run()
{
    process();
}

void Worker::process()
{
    if (role == UPLOADER) {
        while (true) {
            char* buffer = new char[32];
            uint size = 32;
            char* request_raw = this->read_until((uint64_t)packet_size);
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

            switch (_packet.action_type) {
            case ACTION_DOWNLOAD: {
                auto status = list->get_task(hash)->get_status();
                if (status != Task::TaskStatus::SEEDING && status != Task::TaskStatus::DOWNLOADING) {
                    packet wrong_reply;
                    wrong_reply.action_type = REPLY_WRONG;
                    send_packet(wrong_reply);
                    break;
                }
                process_download(hash, _packet.packet_num);
                break;
            }
            case ACTION_BITMAP: {
                qDebug() << "access bitmap for file " << hash;
                process_bitmap(hash);
                break;
            }
            case ACTION_UPLOAD:
            case REPLY_BITMAP:
            case REPLY_WRONG:
            default:
                packet wrong_reply;
                wrong_reply.action_type = REPLY_WRONG;
                send_packet(wrong_reply);
                break;
            }
        }
    }
}

void Worker::process_download(QByteArray& hash, uint64_t packet_num)
{
    packet _packet;
    _packet.action_type = ACTION_UPLOAD;
    _packet.packet_num = packet_num;
    _packet.hash = hash;
    char* bytes = new char[per_block_size];
    _packet.length = 0;
    if (!list->exist(hash) || list->get_task(hash)->get_block(packet_num, &_packet.length, bytes) == false)
    {
        delete[] bytes;
        _packet.action_type = REPLY_WRONG;
        send_packet(_packet);
        return;
    }
    qDebug() << "packet length is " << _packet.length;
    QByteArray bytes_array(bytes, _packet.length);
    QDataStream stream(bytes_array);
    send_packet(_packet);

    char* buffer = new char[buffer_size];
    auto task = list->get_task(hash);
    QEventLoop* loop = new QEventLoop;
    QObject::connect(socket, &QTcpSocket::bytesWritten, [=] (qint64 byte_size) {
        task->add_uploaded(byte_size);
        loop->quit();
    });
    while (!stream.atEnd())
    {
        uint64_t real_size = stream.readRawData(buffer, buffer_size);
        qDebug() << "write block " << real_size;
        socket->write(buffer, real_size);
        loop->exec();
    }
    disconnect(socket, &QTcpSocket::bytesWritten, 0, 0);

    delete[] buffer;
    delete[] bytes;
}

void Worker::send_packet(packet _packet)
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

char* Worker::read_until(uint64_t size)
{
    char* bytes = new char[size];
    uint64_t real = 0;
    while (real < size) {
        char* tmp = bytes + real;
        uint64_t to_read = size - real;
        QEventLoop eloop;
        connect(socket, SIGNAL(readyRead()), &eloop, SLOT(quit()));
        eloop.exec();
        real += socket->read(tmp, to_read);
        qDebug() << "real is " << real;
    }
    return bytes;
}


void Worker::process_bitmap(QByteArray& hash)
{
    uint64_t size = 0;
    uint64_t downloaded_blocks_num = 0;
    packet _packet;
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);

    _packet.action_type = REPLY_BITMAP;
    _packet.hash = hash;
    if (!list->exist(hash))
    {
        qDebug() << "hash " << hash << " not in list";
        _packet.action_type = REPLY_WRONG;
        send_packet(_packet);
        return;
    }
    for (const auto each : list->get_task(hash)->get_full_blocks()) {
        stream << (unsigned long long)each;
        downloaded_blocks_num++;
    }
    size = downloaded_blocks_num * sizeof(uint64_t);
    _packet.packet_num = 0;
    _packet.length = size;
    _packet.offset = 0;
    send_packet(_packet);
    char* buffer = new char[buffer_size];
    uint64_t writed_size = 0;
    QDataStream read_stream(&bytes, QIODevice::ReadOnly);
    while (!read_stream.atEnd())
    {
        uint64_t  writed, to_write;
        to_write = (size - writed_size) > buffer_size ? buffer_size : (size - writed_size);
        read_stream.readRawData(buffer, buffer_size);
        writed = socket->write(buffer, to_write);
        writed_size += writed;
    }

    delete[] buffer;
}




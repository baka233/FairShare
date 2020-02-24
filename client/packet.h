#ifndef PACKET_H
#define PACKET_H

#include <QByteArray>

struct packet {
    uint32_t action_type;
    QByteArray hash;
    uint32_t packet_num;
    uint32_t offset;
    uint32_t length;
};

 enum action_type_enum {
     ACTION_UPLOAD,
     ACTION_DOWNLOAD,
     ACTION_BITMAP,
     REPLY_BITMAP,
     REPLY_WRONG,
 };



#endif // PACKET_H

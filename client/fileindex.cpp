#include "fileindex.h"
#include <string>
#include <sstream>
#include <ostream>
#include <cstring>
#include <QCryptographicHash>
#include <QDataStream>
#include <QIODevice>
#include <QDebug>

const uint64_t block_size = 1 << 22;

FileIndex::FileIndex(QByteArray str, uint64_t block_num)
{
    QDataStream stream(&str, QIODevice::ReadOnly);
    uint len = 32;
    for (uint64_t i = 0; i < block_num; i++) {
        char* tmp = new char[32];
        stream.readRawData(tmp, len);
        sum_tree.push_back(QByteArray::fromRawData(tmp, 32));
    }
}

FileIndex::FileIndex(std::ifstream& stream)
{
    QByteArray ans;
    QCryptographicHash md(QCryptographicHash::Md5);

    char* byte_buffer = new char[block_size];
    while (!stream.eof())
    {
        // clear the byte_buffer in order to calculate correct md5sum
        memset(byte_buffer, 0, block_size);
        stream.read(byte_buffer, block_size);
        md.reset();
        md.addData(byte_buffer, block_size);
        ans = md.result();
        sum_tree.push_back(ans.toHex());
    }
    delete[] byte_buffer;
}


bool FileIndex::verify_block(uint64_t block_num, QByteArray& data)
{
    QByteArray ans;
    QCryptographicHash md(QCryptographicHash::Md5);
    if (static_cast<int>(block_num) >= sum_tree.size()) {
        return false;
    }

    md.addData(data);
    ans = md.result();
    if (ans == sum_tree[block_num]) {
        return true;
    } else {
        return false;
    }
}

QString FileIndex::get_hex()
{
    QByteArray ans;
    for (const auto& each : sum_tree) {
        ans += each;
    }
    return ans;
}

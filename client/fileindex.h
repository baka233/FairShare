#ifndef FILEINDEX_H
#define FILEINDEX_H

#include <QVector>
#include <string>
#include <fstream>
#include <QByteArray>

class FileIndex
{
public:
    explicit FileIndex(QByteArray str, uint64_t block_num);
    FileIndex(std::ifstream& stream);
    
    inline QByteArray get(uint64_t block_num) {
        return sum_tree[block_num];
    }
    
    bool verify_block(uint64_t block_num, QByteArray& data);
    QString get_hex();
    
private:
    QVector<QByteArray> sum_tree;
};

#endif // FILEINDEX_H

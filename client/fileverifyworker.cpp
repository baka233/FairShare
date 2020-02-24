#include "fileverifyworker.h"
#include <fstream>
#include <QCryptographicHash>
#include <QDebug>

FileVerifyWorker::FileVerifyWorker(QString _file_name, QObject *parent)
    : QThread(parent), file_path(_file_name)
{

}

void FileVerifyWorker::run()
{
    std::ifstream stream(file_path.toStdString());
    auto md5 = calculate_sum(stream).toHex();

    qDebug() << "calculate result is " << md5;
    qDebug() << "fileverifyworker thread is " << currentThreadId();

    // move to the file begin
    stream.clear();
    stream.seekg(std::ios_base::beg);
    auto index = std::make_shared<FileIndex>(stream);
    emit sum_calculated(md5, index);
}

FileVerifyWorker::~FileVerifyWorker()
{
    requestInterruption();
    quit();
}


QByteArray FileVerifyWorker::calculate_sum(std::ifstream& stream)
{
    char* buf = new char[8192];
    QCryptographicHash md(QCryptographicHash::Md5);

    while (!stream.eof())
    {
        memset(buf, 0, 8192);
        stream.read(buf, 8192);
        md.addData(buf, 8192);
    }
    delete[] buf;

    return md.result();
}

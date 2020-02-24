#ifndef FILEVERIFYWORKER_H
#define FILEVERIFYWORKER_H

#include <QObject>
#include <QThread>
#include "fileindex.h"

class FileVerifyWorker : public QThread
{
    Q_OBJECT
public:
    explicit FileVerifyWorker(QString _file_path, QObject *parent = nullptr);
    ~FileVerifyWorker();
    QByteArray calculate_sum(std::ifstream& stream);
    void run();


private:
    QString file_path = "";

signals:
    void sum_calculated(QString sum, std::shared_ptr<FileIndex> index);
};

#endif // FILEVERIFYWORKER_H

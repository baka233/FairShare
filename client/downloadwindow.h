#ifndef DOWNLOADWINDOW_H
#define DOWNLOADWINDOW_H

#include "tasklist.h"
#include <QMainWindow>
#include <QLineEdit>

class DownloadWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit DownloadWindow(std::shared_ptr<TaskList> _list, QWidget *parent = nullptr);
    explicit DownloadWindow(QString hash, std::shared_ptr<TaskList> _list, QWidget *parent = nullptr);

private:
    std::shared_ptr<TaskList> list;
    QLineEdit* file_line;
    QLineEdit* hash_line;

signals:
};

#endif // DOWNLOADWINDOW_H

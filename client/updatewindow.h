#ifndef UPDATEWINDOW_H
#define UPDATEWINDOW_H

#include "tasklist.h"
#include <QMainWindow>
#include <QLineEdit>
#include <QTextEdit>

class UpdateWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit UpdateWindow(std::shared_ptr<TaskList> _list, QWidget *parent = nullptr);


private:
    QLineEdit* file_line;
    QTextEdit* description_edit;
    std::shared_ptr<TaskList> list;


signals:

};

#endif // UPDATEWINDOW_H

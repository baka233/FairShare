#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "tasklist.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(std::shared_ptr<TaskList> list, QWidget *parent = nullptr);
    ~MainWindow();
};
#endif // MAINWINDOW_H

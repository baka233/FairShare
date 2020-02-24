#ifndef TASKINFOPANEL_H
#define TASKINFOPANEL_H

#include <QWidget>
#include <QTextBrowser>
#include "task.h"

class TaskInfoPanel : public QTextBrowser
{
    Q_OBJECT
public:
    explicit TaskInfoPanel(QWidget *parent = nullptr);

public slots:
    void show_item_info(std::shared_ptr<Task> task);

signals:

};

#endif // TASKINFOPANEL_H

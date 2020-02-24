#ifndef TASKLIST_H
#define TASKLIST_H

#include "task.h"
#include <QMap>
#include <QEventLoop>
#include <QTimer>

class TaskList
{
public:
    TaskList();
    bool addTask(QByteArray& hash, std::shared_ptr<Task> task);
    QMap<QByteArray, std::shared_ptr<Task>>& get_tasks();
    std::shared_ptr<Task> get_task(QByteArray& hash);
    QMap<QString, std::shared_ptr<Task>>& get_tmp_tasks() {
         return this->tmp_tasks;
    }
    void remove_task(QByteArray& hash) {
        if (hash == "") {
            return;
        }
        // remove the task from the task list first
        if (!tasks.contains(hash)) {
            return;
        }
        std::shared_ptr<Task> task = tasks[hash];
        tasks.remove(hash);

        // change status to PAUSED
        task->set_status(Task::TaskStatus::DELETED);

        // wait for all of the resource released
        QEventLoop loop;
        QTimer::singleShot(1000 * 5, &loop, &QEventLoop::quit);
        loop.exec();
    }
    bool exist(QByteArray& hash);
    bool addTmpTask(std::shared_ptr<Task> task);
    void removeTmpTask(std::shared_ptr<Task> task) {
        auto uuid = task->get_uuid();
        if (tmp_tasks.contains(uuid)) {
            tmp_tasks.remove(uuid);
        }
    }
private:
    QMap<QByteArray, std::shared_ptr<Task>> tasks;
    QMap<QString, std::shared_ptr<Task>> tmp_tasks;

};

#endif // TASKLIST_H

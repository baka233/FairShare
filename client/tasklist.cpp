#include "tasklist.h"
#include <QUuid>

TaskList::TaskList()
{
}

bool TaskList::addTask(QByteArray& hash, std::shared_ptr<Task> task)
{
    if (tasks.contains(hash)) {
        // task has exist
        return false;
    }
    tasks.insert(hash, task);
    return true;
}

bool TaskList::addTmpTask(std::shared_ptr<Task> task)
{
    auto uuid = QUuid::createUuid();
    tmp_tasks.insert(uuid.toString(), task);
    task->set_uuid(uuid.toString());
    return true;
}


QMap<QByteArray, std::shared_ptr<Task>>& TaskList::get_tasks() {
    return this->tasks;
}

std::shared_ptr<Task> TaskList::get_task(QByteArray& hash)
{
    if (this->tasks.contains(hash)) {
        return this->tasks[hash];
    } else {
        return std::shared_ptr<Task>(nullptr);
    }
}

bool TaskList::exist(QByteArray& hash)
{
    return this->tasks.contains(hash);
}

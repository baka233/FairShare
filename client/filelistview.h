#ifndef FILELISTVIEW_H
#define FILELISTVIEW_H

#include <QWidget>
#include <QTableView>
#include <memory>
#include <QTimer>
#include <tasklist.h>
#include <QStandardItemModel>

class FileListView : public QTableView
{
    Q_OBJECT
public:
    explicit FileListView(std::shared_ptr<TaskList> _list, QWidget *parent = nullptr);
    void addItem(std::shared_ptr<Task> task);
    ~FileListView();

private:
    std::shared_ptr<TaskList> list;
    QStandardItemModel* model;
    QTimer* timer;
    int count = 0;
public slots:
    void delete_task() {
        QModelIndexList selected = this->selectionModel()->selectedIndexes();
        if (!selected.isEmpty()) {
            auto hash_item = model->item(selected.takeFirst().row(), 7);
            if (hash_item != nullptr) {
                auto text = hash_item->text().toUtf8();
                list->remove_task(text);
            }
        }
    }

    void start_task() {
        QModelIndexList selected = this->selectionModel()->selectedIndexes();
        if (!selected.isEmpty()) {
            auto hash_item = model->item(selected.takeFirst().row(), 7);
            if (hash_item != nullptr) {
                auto text = hash_item->text().toUtf8();
                auto task = list->get_task(text);
                if (task.get() != nullptr) {
                    task->start();
                }
            }
        }
    }
    void pause_task() {
        QModelIndexList selected = this->selectionModel()->selectedIndexes();
        if (!selected.isEmpty()) {
            auto hash_item = model->item(selected.takeFirst().row(), 7);
            if (hash_item != nullptr) {
                auto text = hash_item->text().toUtf8();
                auto task = list->get_task(text);
                if (task.get() != nullptr) {
                    task->pause();
                }
            }
        }
    }


private slots:
    void update_table();
signals:
    void select_item(std::shared_ptr<Task>);
};

#endif // FILELISTVIEW_H

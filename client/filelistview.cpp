#include "filelistview.h"
#include <QStandardItem>
#include <memory>
#include <QProgressBar>
#include "tasklist.h"
#include "task.h"

FileListView::FileListView(std::shared_ptr<TaskList> _list, QWidget *parent)
    : QTableView(parent), list(_list)
{
    timer = new QTimer(this);
    model = new QStandardItemModel();
    QStringList labels = QObject::tr("文件名,下载比率,文件大小,任务状态,下载速度,上传速度,剩余时间,文件校验码").simplified().split(",");
    model->setHorizontalHeaderLabels(labels);


    for (const auto& each : list->get_tasks()) {
        addItem(each);
    }

    timer->setInterval(300);
    connect(timer, &QTimer::timeout, this, &FileListView::update_table);
    timer->start();

    this->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
//    this->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

    this->setModel(model);
}

void FileListView::update_table()
{
    count = 0;
    QModelIndexList selected = this->selectionModel()->selectedIndexes();
    model->removeRows(0, model->rowCount());
    this->setUpdatesEnabled(false);
    int row = -1;
    if (!selected.isEmpty()) {
        row = selected.takeFirst().row();
    }
    int i = 0;
    for (const auto& each : list->get_tasks()) {
        if (i == row) {
            emit select_item(each);
        }
        addItem(each);
        i++;
    }
    for (const auto& each : list->get_tmp_tasks()) {
        if (i == row) {
            emit select_item(each);
        }

        addItem(each);
        i++;
    }
    if (row != -1) {
        this->selectRow(row);
    }
    this->setUpdatesEnabled(true);
    timer->start();
}

void FileListView::addItem(std::shared_ptr<Task> task)
{
    int i = count++;
    QProgressBar* progress = new QProgressBar;
    progress->setMaximum(100);
    progress->setMinimum(0);
    progress->setValue(task->get_progress());
    task->calculate_download_speed(300);
    task->calculate_uploaded_speed(300);

    model->setItem(i, 0, new QStandardItem(task->get_filename()));
    this->setIndexWidget(model->index(i, 1), progress);
//    model->setItem(i, 1, new QStandardItem(task->get_progress()));
    model->setItem(i, 2, new QStandardItem(task->get_size_beautify()));
    model->setItem(i, 3, new QStandardItem(task->get_status_string()));
    model->setItem(i, 4, new QStandardItem(task->get_download_speed()));
    model->setItem(i, 5, new QStandardItem(task->get_upload_speed()));
    model->setItem(i, 6, new QStandardItem(task->get_remain_time()));
    model->setItem(i, 7, new QStandardItem(task->get_md5()));
}




FileListView::~FileListView()
{
}


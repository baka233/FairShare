#include "filebitmapview.h"
#include <QPixmap>
#include <QSize>
#include <QLabel>
#include <QHBoxLayout>

const int row_len = 25;

FileBitMapView::FileBitMapView(QWidget *parent) : QWidget(parent)
{
    QGridLayout* tmp_grid = new QGridLayout;
    QScrollArea *area = new QScrollArea(this);
    area->setWidgetResizable(true);
    area->setWidget(new QWidget);
    grid = new QGridLayout;
    area->widget()->setLayout(grid);
    tmp_grid->addWidget(area);

    grid->setSpacing(0);
    grid->setContentsMargins(0, 0, 0, 0);


    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < row_len; j++) {
            QSize size(20, 20);
            QPixmap icon = QPixmap(":/icons/undownloaded_block.svg");
            auto new_icon = icon.scaled(20, 20, Qt::KeepAspectRatio);
            QLabel* label = new QLabel();
            label->setPixmap(new_icon);
            label->setMaximumSize(size);
            grid->addWidget(label, i, j);
        }
    }
    setLayout(tmp_grid);
}

void FileBitMapView::clearLayout(QLayout *layout)
{
    QLayoutItem *item;
    while((item = layout->takeAt(0)) != 0){
        if(item->widget()){
            delete item->widget();
            //item->widget()->deleteLater();
        }
        QLayout *childLayout = item->layout();
        if(childLayout){
            clearLayout(childLayout);
        }
        delete item;
    }
}

void FileBitMapView::show_blocks(std::shared_ptr<Task> task)
{
    if (count != 10) {
        count++;
        return;
    } else {
        count = 0;
    }
    QVector<uint64_t> blocks = task->get_full_blocks();
    if (last_task == task->get_md5()) {
        for (auto each : blocks) {
            if (marked_blocks.contains(each)) {
                continue;
            }
            QSize size(20, 20);
            QPixmap icon = QPixmap(":/icons/downloaded_block.svg");
            auto new_icon = icon.scaled(20, 20, Qt::KeepAspectRatio);
            QLabel* label = new QLabel();
            label->setPixmap(new_icon);
            label->setMaximumSize(size);
            grid->addWidget(label, each / row_len, each % row_len);
            marked_blocks.insert(each);
        }
    } else {
        marked_blocks.clear();
        int block = -1;
        clearLayout(grid);
        if (!blocks.isEmpty()) {
            block = blocks.takeFirst();
        }
        for (uint64_t i = 0; i < task->get_block_number(); i++) {
            QString path;
            if (block == i) {
                path = ":/icons/downloaded_block.svg";
                if (!blocks.isEmpty()) {
                    block = blocks.takeFirst();
                }
            } else {
                path = ":/icons/undownloaded_block.svg";
            }
            QSize size(20, 20);
            QPixmap icon = QPixmap(path);
            auto new_icon = icon.scaled(20, 20, Qt::KeepAspectRatio);
            QLabel* label = new QLabel();
            label->setPixmap(new_icon);
            label->setMaximumSize(size);
            grid->addWidget(label, i / row_len, i % row_len);
        }
        last_task = task->get_md5();
    }
}

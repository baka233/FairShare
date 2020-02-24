#include "functionbar.h"
#include "downloadwindow.h"
#include <QHBoxLayout>
#include <QObject>
#include <QToolButton>
#include <QFileDialog>
#include <QStyle>
#include <updatewindow.h>

FunctionBar::FunctionBar(std::shared_ptr<TaskList> _list,QWidget *parent)
    : QWidget(parent), list(_list)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    downloadButton = new QPushButton(QIcon(":/icons/download.png"), "下载");
    uploadButton = new QPushButton(QIcon(":/icons/upload.png"), "上传");
    startButton = new QPushButton(QIcon(":/icons/start.png"), "开始");
    pauseButton = new QPushButton(QIcon(":/icons/stop.png"), "暂停");
//    stopButton = new QPushButton(QIcon(":/icons/pause.png"), "终止");
    deleteButton = new QPushButton(QIcon(":/icons/delete.png"), "删除");

    downloadButton->setFlat(true);
    uploadButton->setFlat(true);
    startButton->setFlat(true);
    pauseButton->setFlat(true);
    //stopButton->setFlat(true);
    deleteButton->setFlat(true);


    connect(uploadButton, &QToolButton::clicked , [&] {
        if (updateWindow != nullptr) {
            delete updateWindow;
        }
        updateWindow = new UpdateWindow(this->list);
        updateWindow->show();
    });

    connect(downloadButton, &QToolButton::clicked, [&] {
        if (downloadWindow != nullptr) {
            delete downloadWindow;
        }
        downloadWindow = new DownloadWindow(this->list);
        downloadWindow->show();
    });

    connect(startButton, &QToolButton::clicked, this, &FunctionBar::start_click_slots);
    connect(pauseButton, &QToolButton::clicked, this, &FunctionBar::pause_click_slots);
    connect(deleteButton, &QToolButton::clicked, this, &FunctionBar::delete_click_slots);


    downloadButton->setMaximumWidth(100);
    uploadButton->setMaximumWidth(100);
    pauseButton->setMaximumWidth(100);
    // stopButton->setMaximumWidth(100);
    startButton->setMaximumWidth(100);
    deleteButton->setMaximumWidth(100);

    layout->setAlignment(Qt::AlignLeft);
    layout->addWidget(downloadButton);
    layout->addWidget(uploadButton);
    layout->addWidget(startButton);
    layout->addWidget(pauseButton);
    // layout->addWidget(stopButton);
    layout->addWidget(deleteButton);
}

FunctionBar::~FunctionBar()
{
    if (updateWindow != nullptr)
    {
        delete updateWindow;
    }
    if (downloadWindow != nullptr)
    {
        delete downloadWindow;
    }
}

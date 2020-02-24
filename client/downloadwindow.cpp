#include "downloadwindow.h"
#include <tasklist.h>
#include <QGridLayout>
#include <QLabel>
#include <QFileDialog>
#include <QPushButton>
#include <QToolButton>
#include <QMessageBox>

DownloadWindow::DownloadWindow(std::shared_ptr<TaskList> _list, QWidget *parent)
    : QMainWindow(parent), list(_list)
{
    this->setAttribute(Qt::WA_QuitOnClose, false);
    QWidget* central_widget = new QWidget;
    QGridLayout* layout = new QGridLayout(central_widget);
    QLabel* file_label = new QLabel(tr("保存路径"));
    QLabel* hash_label = new QLabel(tr("文件hash"));
    QPushButton* add_button = new QPushButton(tr("下载"));
    QPushButton* select_button = new QPushButton(tr("选择文件"));

    this->file_line = new QLineEdit();
    this->hash_line = new QLineEdit();
    this->file_line->setText(QDir::currentPath());

    connect(select_button, &QToolButton::clicked, [=] {
        QString file_path = QFileDialog::getExistingDirectory();
        this->file_line->setText(file_path);
    });

    connect(add_button,&QToolButton::clicked, [=] {
        auto task = std::make_shared<Task>(
            this->file_line->text(),
            this->hash_line->text(),
            Task::DOWNLOAD
        );
        auto hash = task->get_md5().toUtf8();
        if (this->list->addTask(hash, task) == false) {
            QMessageBox::information(this, "错误", "任务已存在");
        }
        this->hide();
    });

    layout->addWidget(file_label, 1, 1, 3, 2);
    layout->addWidget(hash_label, 5, 1, 3, 2);
    layout->addWidget(this->file_line, 1, 3, 3, 5);
    layout->addWidget(this->hash_line, 5, 3, 3, 8);
    layout->addWidget(add_button, 10, 5, 2, 2);
    layout->addWidget(select_button, 1, 8, 3, 3);

    this->setCentralWidget(central_widget);

}

DownloadWindow::DownloadWindow(QString hash, std::shared_ptr<TaskList> _list, QWidget *parent)
    : DownloadWindow(_list, parent)
{
    this->hash_line->setText(hash);
}

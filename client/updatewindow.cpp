#include "updatewindow.h"
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QPushButton>
#include <QTextEdit>
#include <QGridLayout>
#include <QToolButton>
#include <QMessageBox>
#include "tasklist.h"

UpdateWindow::UpdateWindow(std::shared_ptr<TaskList> _list, QWidget *parent)
    : QMainWindow(parent), list(_list)
{
    this->setAttribute(Qt::WA_QuitOnClose, false);
    QWidget* central_widget = new QWidget;
    QGridLayout* layout = new QGridLayout(central_widget);
    QLabel* file_label = new QLabel(tr("路径"));
    QLabel* description_label = new QLabel(tr("文件注释"));
    QPushButton* add_button = new QPushButton(tr("上传"));
    QPushButton* select_button = new QPushButton(tr("选择文件"));

    this->file_line = new QLineEdit();
    this->description_edit = new QTextEdit();

    connect(select_button, &QToolButton::clicked, [&] {
        QString file_path = QFileDialog::getOpenFileName();
        this->file_line->setText(file_path);
    });


    connect(add_button,&QToolButton::clicked, [&] {
        auto task = std::make_shared<Task>(this->file_line->text(), this->description_edit->toPlainText(), Task::UPLOAD);

        auto hash = task->get_md5().toUtf8();
        if (hash == "") {
            connect(task.get(), &Task::upload_prepared,  [=] {
                auto hash = task->get_md5().toUtf8();
                this->list->removeTmpTask(task);
                this->list->addTask(hash, task);
            });
            this->list->addTmpTask(task);
        }
        else if (this->list->addTask(hash, task) == false) {
            QMessageBox::information(this, "错误", "任务已存在");
        }
        this->hide();
    });

    layout->addWidget(file_label, 1, 1, 3, 2);
    layout->addWidget(description_label, 5, 1, 3, 2);
    layout->addWidget(this->file_line, 1, 3, 3, 5);
    layout->addWidget(this->description_edit, 5, 3, 4, 8);
    layout->addWidget(add_button, 10, 5, 2, 2);
    layout->addWidget(select_button, 1, 8, 3, 3);

    this->setCentralWidget(central_widget);
}



#ifndef FUNCTIONBAR_H
#define FUNCTIONBAR_H

#include <QWidget>
#include <QPushButton>
#include "updatewindow.h"
#include "downloadwindow.h"

class FunctionBar : public QWidget
{
    Q_OBJECT
public:
    explicit FunctionBar(std::shared_ptr<TaskList> _list, QWidget *parent = nullptr);
    ~FunctionBar();
private:
    QPushButton* downloadButton;

    QPushButton* uploadButton;
    QPushButton* pauseButton;
    QPushButton* startButton;
    // QPushButton* stopButton;
    QPushButton* deleteButton;
    UpdateWindow* updateWindow = nullptr;
    DownloadWindow* downloadWindow = nullptr;
    std::shared_ptr<TaskList> list;
private slots:
    void start_click_slots() {
        emit start_clicked();
    }
    void pause_click_slots() {
        emit pause_clicked();
    }
    void delete_click_slots() {
        emit delete_clicked(false);
    }

public slots:
    void download_hash_slots(QString hash) {
        if (downloadWindow != nullptr) {
            delete downloadWindow;
        }
        downloadWindow = new DownloadWindow(hash, this->list);
        downloadWindow->show();

    }
signals:
    void start_clicked();
    void pause_clicked();
    void delete_clicked(bool delete_file);
};

#endif // FUNCTIONBAR_H

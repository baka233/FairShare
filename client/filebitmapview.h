#ifndef FILEBITMAPVIEW_H
#define FILEBITMAPVIEW_H

#include <QWidget>
#include <QTextBrowser>
#include <QScrollArea>
#include <QGridLayout>
#include "task.h"


class FileBitMapView : public QWidget
{
    Q_OBJECT
public:
    explicit FileBitMapView(QWidget *parent = nullptr);
    void clearLayout(QLayout *layout);


public slots:
    void show_blocks(std::shared_ptr<Task> task);
private:
    int count = 0;
    QString last_task = "";
    QSet<uint64_t> marked_blocks;
    QGridLayout *grid = nullptr;

signals:

};

#endif // FILEBITMAPVIEW_H

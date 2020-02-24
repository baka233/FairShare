#ifndef NAVIGATORBAR_H
#define NAVIGATORBAR_H

#include <QWidget>
#include <QPushButton>

class NavigatorBar : public QWidget
{
    Q_OBJECT
public:
    explicit NavigatorBar(QWidget *parent = nullptr);
    ~NavigatorBar();
private:
    QPushButton* searchViewButton;
    QPushButton* downloadViewButton;

private slots:
    void file_list_slots() {
        emit file_list_clicked();
    }

    void search_slots() {
        emit search_clicked();
    }

signals:
    void file_list_clicked();
    void search_clicked();
};

#endif // NAVIGATORBAR_H

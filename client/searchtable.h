#ifndef SEARCHTABLE_H
#define SEARCHTABLE_H

#include <QWidget>
#include <QTableView>
#include <QTextEdit>
#include <QStandardItemModel>
#include <QPushButton>
#include <QNetworkAccessManager>
#include <QLineEdit>


class SearchTable : public QWidget
{
    Q_OBJECT
public:
    explicit SearchTable(QWidget *parent = nullptr);

private:
    QString get_size_beautify(uint64_t size) const;

    QTableView* table;
    QLineEdit* search_bar;
    QStandardItemModel* model;
    QPushButton* search_button;
    QNetworkAccessManager* net_manager;
    QString base_url = "http://127.0.0.1:8000";

signals:
    void download_event(QString hash);

private slots:
    void search();

signals:

};

#endif // SEARCHTABLE_H

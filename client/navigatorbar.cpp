#include "navigatorbar.h"
#include <QVBoxLayout>
#include <QPushButton>

NavigatorBar::NavigatorBar(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    downloadViewButton = new QPushButton("下载列表");
    searchViewButton = new QPushButton("搜索");

    connect(downloadViewButton, &QPushButton::clicked, this, &NavigatorBar::file_list_slots);
    connect(searchViewButton, &QPushButton::clicked, this, &NavigatorBar::search_slots);

    layout->addWidget(downloadViewButton);
    layout->addWidget(searchViewButton);
    layout->setAlignment(Qt::AlignTop);
}


NavigatorBar::~NavigatorBar() {
    if (downloadViewButton != nullptr) {
        delete downloadViewButton;
    }
    if (searchViewButton != nullptr) {
        delete searchViewButton;
    }
}

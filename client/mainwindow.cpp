#include "mainwindow.h"
#include "functionbar.h"
#include "navigatorbar.h"
#include "filelistview.h"
#include "taskinfopanel.h"
#include "filebitmapview.h"
#include <QVBoxLayout>
#include <QListWidget>
#include <QAbstractAnimation>
#include "searchtable.h"


MainWindow::MainWindow(std::shared_ptr<TaskList> list, QWidget *parent)
    : QMainWindow(parent)
{
    setAttribute(Qt::WA_QuitOnClose, true);
    QWidget* central_widget = new QWidget;
    QGridLayout* main_layout = new QGridLayout(central_widget);
    QHBoxLayout* container_layout = new QHBoxLayout();
//    QHBoxLayout* file_info_layout = new QHBoxLayout();
    FunctionBar* function_bar = new FunctionBar(list);
    FileListView* file_list_view = new FileListView(list);
    NavigatorBar* navigator_bar = new NavigatorBar();
    TaskInfoPanel* task_info_panel = new TaskInfoPanel();
    FileBitMapView* browser = new FileBitMapView();
    QTabWidget* tabWidget = new QTabWidget;
    SearchTable* table = new SearchTable;

    file_list_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    file_list_view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

    connect(function_bar, &FunctionBar::start_clicked, file_list_view, &FileListView::start_task);
    connect(function_bar, &FunctionBar::pause_clicked, file_list_view, &FileListView::pause_task);
    connect(function_bar, &FunctionBar::delete_clicked, file_list_view, &FileListView::delete_task);
    connect(navigator_bar, &NavigatorBar::file_list_clicked, [=] {
        table->hide();
        file_list_view->show();
        tabWidget->show();
    });

    connect(navigator_bar, &NavigatorBar::search_clicked, [=] {
        file_list_view->hide();
        tabWidget->hide();
        table->show();
    });

    connect(file_list_view, &FileListView::select_item, task_info_panel, &TaskInfoPanel::show_item_info);
    connect(file_list_view, &FileListView::select_item, browser, &FileBitMapView::show_blocks);


    connect(table, &SearchTable::download_event, function_bar, &FunctionBar::download_hash_slots);

    table->hide();

    tabWidget->addTab(task_info_panel, "文件信息");
    tabWidget->addTab(browser, "文件位图");

    container_layout->addWidget(navigator_bar);

//    file_info_layout->addWidget(list);

    this->resize(QSize(1024, 600));
    main_layout->addWidget(function_bar, 0, 0, 1, 10);
    main_layout->addWidget(file_list_view, 1, 2, 6, 8);
    main_layout->addWidget(tabWidget, 7, 2, 3, 8);
    main_layout->addLayout(container_layout, 1, 0, 9, 2);
    main_layout->addWidget(table, 1, 2, 9, 8);

    this->setCentralWidget(central_widget);
}

MainWindow::~MainWindow()
{
}


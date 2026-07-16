//MainWindow只负责页面切换
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "widget/dishwidget.h"
#include "widget/tablewidget.h"
#include "widget/coverwidget.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // resize(1200,800);
    setWindowTitle(QStringLiteral("大丰收 · 平板点餐系统"));
    ui->menubar->hide();
    ui->statusbar->hide();
    setMinimumSize(1200,800);

    //进入程序默认显示桌台
    showCoverPage();

}

void MainWindow::showCoverPage()
{
    auto *cover = new CoverWidget(this);
    setCentralWidget(cover);
    connect(cover, &CoverWidget::startOrdering, this, &MainWindow::showTablePage);
}

void MainWindow::showTablePage(){
    TableWidget *table =
        new TableWidget(this);
    setCentralWidget(table);
    connect(table,&TableWidget::tableSelected,this,[this](int tableId){
       qDebug()<<"选择桌台:"<<tableId;
        showDishPage(tableId);
    });
}


    // if(tableWidget==nullptr)
    // {
    //     tableWidget=new TableWidget(this);
    // }
    // setCentralWidget(tableWidget);
    // connect(tableWidget,&TableWidget::tableSelected,this,[this](int tableId){
    //     qDebug()<<"选择桌台:"<<tableId;
    //     showDishPage(tableId);
    // });


void MainWindow::showDishPage(int tableId)
{
    DishWidget *dish =new DishWidget(tableId,this);
    setCentralWidget(dish);
    connect(dish,&DishWidget::backToTable,this,[this]()
            {
                showTablePage();
            });
}

// dishWidget = new DishWidget(tableId,this);
// setCentralWidget(dishWidget);
// connect(dishWidget,&DishWidget::backToTable,this,[this](){
//     showTablePage();
// });
MainWindow::~MainWindow()
{
    delete ui;
}


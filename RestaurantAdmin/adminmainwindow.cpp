#include "adminmainwindow.h"
#include "ui_adminmainwindow.h"
#include "login/adminloginwidget.h"
#include "table/admintablewidget.h"
#include "dish/admindishwidget.h"
#include "order/adminorderwidget.h"
#include <QTabWidget>

AdminMainWindow::AdminMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AdminMainWindow)
{
    ui->setupUi(this);
    setWindowTitle("餐厅管理系统");
    setMinimumSize(1050, 700);
    showLoginPage();
}

void AdminMainWindow::showLoginPage()
{
    auto *login = new AdminLoginWidget(this);
    setCentralWidget(login);
    connect(login, &AdminLoginWidget::loginSucceeded, this, &AdminMainWindow::showTablePage);
}

void AdminMainWindow::showTablePage(const QString &username)
{
    auto *tabs = new QTabWidget(this);
    auto *tables = new AdminTableWidget(username, this);
    auto *dishes = new AdminDishWidget(this);
    auto *orders = new AdminOrderWidget(this);
    tabs->addTab(tables, "桌台管理");
    tabs->addTab(dishes, "菜品管理");
    tabs->addTab(orders, "出餐管理");
    setCentralWidget(tabs);
    connect(tables, &AdminTableWidget::logoutRequested, this, &AdminMainWindow::showLoginPage);
}

AdminMainWindow::~AdminMainWindow()
{
    delete ui;
}

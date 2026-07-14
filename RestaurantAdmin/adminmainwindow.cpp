#include "adminmainwindow.h"
#include "ui_adminmainwindow.h"
#include "login/adminloginwidget.h"
#include "table/admintablewidget.h"
#include "dish/admindishwidget.h"
#include "order/adminorderwidget.h"
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

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
    auto *page = new QWidget(this);
    page->setObjectName("adminWorkspace");
    page->setAttribute(Qt::WA_StyledBackground, true);
    page->setStyleSheet("QWidget#adminWorkspace{background:#f7f3eb;}");

    auto *navigation = new QWidget(page);
    navigation->setObjectName("capsuleNavigation");
    navigation->setStyleSheet("QWidget#capsuleNavigation{background:#fffaf2;border:1px solid #eadcc9;border-radius:29px;}");
    navigation->setFixedHeight(66);

    auto *navigationLayout = new QHBoxLayout(navigation);
    navigationLayout->setContentsMargins(7, 7, 7, 7);
    navigationLayout->setSpacing(9);

    auto *buttonGroup = new QButtonGroup(page);
    buttonGroup->setExclusive(true);
    const QString navigationStyle =
        "QPushButton{outline:none;background:#f4eadb;color:#48423d;border:0;border-radius:25px;"
        "font-family:'Microsoft YaHei UI';font-size:16px;font-weight:700;padding:0 24px;}"
        "QPushButton:hover{background:#f1dfca;color:#cf7426;}"
        "QPushButton:checked{background:#ed9338;color:white;}"
        "QPushButton:checked:hover{background:#e4862c;color:white;}"
        "QPushButton:pressed{padding-top:2px;}";

    auto *tableButton = new QPushButton("▦  桌台管理", navigation);
    auto *dishButton = new QPushButton("♨  菜品管理", navigation);
    auto *orderButton = new QPushButton("✓  出餐管理", navigation);
    const QList<QPushButton*> navigationButtons{tableButton, dishButton, orderButton};
    for (int index = 0; index < navigationButtons.size(); ++index) {
        auto *button = navigationButtons[index];
        button->setCheckable(true);
        button->setFocusPolicy(Qt::NoFocus);
        button->setCursor(Qt::PointingHandCursor);
        button->setMinimumSize(178, 52);
        button->setStyleSheet(navigationStyle);
        buttonGroup->addButton(button, index);
        navigationLayout->addWidget(button);
    }
    tableButton->setChecked(true);

    auto *stack = new QStackedWidget(page);
    stack->setObjectName("adminPageStack");
    stack->setStyleSheet("QStackedWidget#adminPageStack{background:transparent;border:0;}");
    auto *tables = new AdminTableWidget(username, stack);
    auto *dishes = new AdminDishWidget(stack);
    auto *orders = new AdminOrderWidget(stack);
    stack->addWidget(tables);
    stack->addWidget(dishes);
    stack->addWidget(orders);

    connect(buttonGroup, &QButtonGroup::idClicked, stack, &QStackedWidget::setCurrentIndex);

    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 18, 28, 0);
    layout->setSpacing(0);
    layout->addWidget(navigation, 0, Qt::AlignHCenter);
    layout->addWidget(stack, 1);

    setCentralWidget(page);
    connect(tables, &AdminTableWidget::logoutRequested, this, &AdminMainWindow::showLoginPage);
}

AdminMainWindow::~AdminMainWindow()
{
    delete ui;
}

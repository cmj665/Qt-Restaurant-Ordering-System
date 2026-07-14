#include "adminloginwidget.h"
#include "ui_adminloginwidget.h"
#include "../network/networkmanager.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

AdminLoginWidget::AdminLoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AdminLoginWidget)
    , usernameEdit(new QLineEdit(this))
    , passwordEdit(new QLineEdit(this))
    , loginButton(new QPushButton("登录", this))
    , messageLabel(new QLabel(this))
    , network(new NetworkManager(this))
{
    ui->setupUi(this);
    auto *panel = new QWidget(this);
    panel->setMaximumWidth(420);
    auto *form = new QVBoxLayout(panel);
    auto *title = new QLabel("餐厅管理系统", panel);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size:30px;font-weight:700;color:#263238;margin-bottom:20px;");
    usernameEdit->setPlaceholderText("管理员账号");
    passwordEdit->setPlaceholderText("密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    for (QLineEdit *edit : {usernameEdit, passwordEdit})
        edit->setStyleSheet("padding:12px;border:1px solid #cfd8dc;border-radius:7px;font-size:16px;");
    loginButton->setMinimumHeight(46);
    loginButton->setStyleSheet("QPushButton{background:#1976d2;color:white;border:0;border-radius:7px;font-size:17px;} QPushButton:hover{background:#1565c0;}");
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setStyleSheet("color:#d32f2f;min-height:24px;");
    form->addWidget(title); form->addWidget(usernameEdit); form->addWidget(passwordEdit);
    form->addWidget(messageLabel); form->addWidget(loginButton);
    auto *layout = new QVBoxLayout(this);
    layout->addStretch(); layout->addWidget(panel, 0, Qt::AlignHCenter); layout->addStretch();
    setStyleSheet("background:#f4f7fa;");

    connect(loginButton, &QPushButton::clicked, this, &AdminLoginWidget::submitLogin);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &AdminLoginWidget::submitLogin);
    connect(network, &NetworkManager::loginFinished, this,
            [this](bool success, const QString &message, const QString &username) {
        loginButton->setEnabled(true);
        if (success) emit loginSucceeded(username);
        else messageLabel->setText(message);
    });
}

void AdminLoginWidget::submitLogin()
{
    const QString username = usernameEdit->text().trimmed();
    if (username.isEmpty() || passwordEdit->text().isEmpty()) {
        messageLabel->setText("请输入账号和密码");
        return;
    }
    loginButton->setEnabled(false);
    messageLabel->setText("正在登录...");
    network->login(username, passwordEdit->text());
}

AdminLoginWidget::~AdminLoginWidget()
{
    delete ui;
}

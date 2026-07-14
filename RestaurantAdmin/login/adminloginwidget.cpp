#include "adminloginwidget.h"
#include "ui_adminloginwidget.h"
#include "passwordrecoverydialog.h"
#include "../network/networkmanager.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>

AdminLoginWidget::AdminLoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AdminLoginWidget)
    , usernameEdit(new QLineEdit(this))
    , passwordEdit(new QLineEdit(this))
    , loginButton(new QPushButton("登 录", this))
    , messageLabel(new QLabel(this))
    , network(new NetworkManager(this))
{
    ui->setupUi(this);

    setWindowTitle("餐厅管理系统 - 管理端登录");
    setObjectName("adminLoginPage");
    setAttribute(Qt::WA_StyledBackground, true);
    setMinimumSize(820, 560);
    setStyleSheet(
        "QWidget#adminLoginPage{"
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,"
        "stop:0 #fffaf0,stop:0.48 #f8ecd7,stop:1 #f7cfa9);"
        "}"
    );

    auto *panel = new QWidget(this);
    panel->setObjectName("loginCard");
    panel->setFixedWidth(500);
    panel->setStyleSheet("QWidget#loginCard{background:#ffffff;border-radius:28px;}");

    auto *shadow = new QGraphicsDropShadowEffect(panel);
    shadow->setBlurRadius(48);
    shadow->setOffset(0, 14);
    shadow->setColor(QColor(122, 76, 35, 55));
    panel->setGraphicsEffect(shadow);

    auto *form = new QVBoxLayout(panel);
    form->setContentsMargins(48, 38, 48, 42);
    form->setSpacing(16);

    auto *brandMark = new QLabel("♨", panel);
    brandMark->setAlignment(Qt::AlignCenter);
    brandMark->setStyleSheet("color:#e99a4b;font-size:38px;font-weight:800;background:transparent;");

    auto *title = new QLabel("餐厅管理系统", panel);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-family:'Microsoft YaHei UI';font-size:30px;font-weight:800;color:#3f3b38;background:transparent;");

    auto *subtitle = new QLabel("管理员登录", panel);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("font-size:15px;color:#8a817b;background:transparent;margin-bottom:10px;");

    usernameEdit->setPlaceholderText("管理员账号");
    passwordEdit->setPlaceholderText("密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    for (QLineEdit *edit : {usernameEdit, passwordEdit}) {
        edit->setFixedHeight(54);
        edit->setStyleSheet(
            "QLineEdit{background:#ffffff;color:#403b37;border:1px solid #ddd5ce;"
            "border-radius:25px;padding:0 20px;font-size:16px;selection-background-color:#f2a14a;}"
            "QLineEdit:hover{border-color:#e7b278;}"
            "QLineEdit:focus{border:2px solid #ed9b45;padding:0 19px;}"
        );
    }

    loginButton->setFixedHeight(54);
    loginButton->setCursor(Qt::PointingHandCursor);
    loginButton->setStyleSheet(
        "QPushButton{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #f39a36,stop:1 #ed862c);"
        "color:white;border:0;border-radius:25px;font-size:17px;font-weight:700;}"
        "QPushButton:hover{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #f6a74b,stop:1 #f09139);}"
        "QPushButton:pressed{background:#dc7623;padding-top:2px;}"
        "QPushButton:disabled{background:#e9c3a1;color:#fff8f1;}"
    );

    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setFixedHeight(24);
    messageLabel->setStyleSheet("color:#d65c4a;font-size:14px;background:transparent;");

    auto *forgotButton = new QPushButton("忘记密码？", panel);
    forgotButton->setCursor(Qt::PointingHandCursor);
    forgotButton->setFocusPolicy(Qt::NoFocus);
    forgotButton->setStyleSheet(
        "QPushButton{outline:none;background:transparent;color:#d98231;border:0;font-size:14px;padding:3px 5px;}"
        "QPushButton:hover{color:#bd681f;text-decoration:underline;}"
    );
    auto *forgotRow = new QHBoxLayout;
    forgotRow->setContentsMargins(0, -5, 0, -3);
    forgotRow->addStretch();
    forgotRow->addWidget(forgotButton);

    form->addWidget(brandMark);
    form->addWidget(title);
    form->addWidget(subtitle);
    form->addSpacing(4);
    form->addWidget(usernameEdit);
    form->addWidget(passwordEdit);
    form->addLayout(forgotRow);
    form->addWidget(messageLabel);
    form->addWidget(loginButton);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(36, 36, 36, 36);
    layout->addStretch();
    layout->addWidget(panel, 0, Qt::AlignHCenter);
    layout->addStretch();

    usernameEdit->setFocus();

    connect(loginButton, &QPushButton::clicked, this, &AdminLoginWidget::submitLogin);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &AdminLoginWidget::submitLogin);
    connect(forgotButton, &QPushButton::clicked, this, [this]() {
        PasswordRecoveryDialog dialog(usernameEdit->text(), this);
        if (dialog.exec() == QDialog::Accepted) {
            messageLabel->setStyleSheet("color:#2e8b57;font-size:14px;background:transparent;");
            messageLabel->setText("密码已重置，请输入新密码登录");
            passwordEdit->clear();
            passwordEdit->setFocus();
        }
    });
    connect(network, &NetworkManager::loginFinished, this,
            [this](bool success, const QString &message, const QString &username) {
        loginButton->setEnabled(true);
        if (success) emit loginSucceeded(username);
        else {
            messageLabel->setStyleSheet("color:#d65c4a;font-size:14px;background:transparent;");
            messageLabel->setText(message);
        }
    });
}

void AdminLoginWidget::submitLogin()
{
    messageLabel->setStyleSheet("color:#d65c4a;font-size:14px;background:transparent;");
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

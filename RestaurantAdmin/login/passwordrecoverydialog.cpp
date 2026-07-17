#include "passwordrecoverydialog.h"
#include "../network/networkmanager.h"

#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

PasswordRecoveryDialog::PasswordRecoveryDialog(const QString &initialUsername, QWidget *parent)
    : QDialog(parent), network(new NetworkManager(this)), usernameEdit(new QLineEdit(this)),
      questionLabel(new QLabel(this)), answerEdit(new QLineEdit(this)), newPasswordEdit(new QLineEdit(this)),
      confirmPasswordEdit(new QLineEdit(this)), messageLabel(new QLabel(this)),
      queryButton(new QPushButton("查询密保问题", this)), resetButton(new QPushButton("重置密码", this)),
      resetArea(new QWidget(this))
{
    // 对话框外观：创建账号、密保答案和新密码输入区域。
    setWindowTitle("找回管理员密码");
    setObjectName("passwordRecoveryDialog");
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(540, 610);
    setStyleSheet("QDialog#passwordRecoveryDialog{background:#f7f3eb;}");

    auto *title = new QLabel("找回密码", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size:28px;font-weight:800;color:#333333;background:transparent;");
    auto *subtitle = new QLabel("验证管理员密保问题后设置新密码", this);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("font-size:14px;color:#8a817b;background:transparent;");

    auto *card = new QWidget(this);
    card->setObjectName("recoveryCard");
    card->setStyleSheet("QWidget#recoveryCard{background:white;border-radius:22px;}");
    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(30); shadow->setOffset(0, 9); shadow->setColor(QColor(90, 60, 35, 38));
    card->setGraphicsEffect(shadow);

    const QString inputStyle =
        "QLineEdit{background:#fffdfa;color:#403b37;border:1px solid #ddd3c8;border-radius:12px;padding:0 15px;font-size:15px;}"
        "QLineEdit:focus{border:2px solid #ed9b45;padding:0 14px;}";
    usernameEdit->setPlaceholderText("管理员账号");
    usernameEdit->setText(initialUsername.trimmed());
    answerEdit->setPlaceholderText("请输入密保答案");
    newPasswordEdit->setPlaceholderText("新密码（至少6位）");
    confirmPasswordEdit->setPlaceholderText("再次输入新密码");
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    for (auto *edit : {usernameEdit, answerEdit, newPasswordEdit, confirmPasswordEdit}) {
        edit->setFixedHeight(48);
        edit->setStyleSheet(inputStyle);
    }

    questionLabel->setWordWrap(true);
    questionLabel->setAlignment(Qt::AlignCenter);
    questionLabel->setStyleSheet("background:#fff3e5;color:#9b5e29;border:1px solid #f2cfaa;border-radius:12px;padding:12px;font-size:15px;font-weight:700;");
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setWordWrap(true);
    messageLabel->setFixedHeight(38);
    messageLabel->setStyleSheet("color:#d65c4a;font-size:13px;background:transparent;");

    const QString primaryStyle =
        "QPushButton{outline:none;background:#ed9338;color:white;border:0;border-radius:20px;font-size:15px;font-weight:700;}"
        "QPushButton:hover{background:#e4862c;} QPushButton:pressed{background:#d67722;} QPushButton:disabled{background:#e6c3a2;color:#fff8f0;}";
    queryButton->setFixedHeight(44); resetButton->setFixedHeight(44);
    queryButton->setStyleSheet(primaryStyle); resetButton->setStyleSheet(primaryStyle);
    queryButton->setCursor(Qt::PointingHandCursor); resetButton->setCursor(Qt::PointingHandCursor);
    queryButton->setFocusPolicy(Qt::NoFocus); resetButton->setFocusPolicy(Qt::NoFocus);

    auto *resetLayout = new QVBoxLayout(resetArea);
    resetLayout->setContentsMargins(0, 0, 0, 0); resetLayout->setSpacing(11);
    resetLayout->addWidget(questionLabel);
    resetLayout->addWidget(answerEdit);
    resetLayout->addWidget(newPasswordEdit);
    resetLayout->addWidget(confirmPasswordEdit);
    resetLayout->addWidget(resetButton);
    resetArea->hide();

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(34, 30, 34, 28); cardLayout->setSpacing(12);
    cardLayout->addWidget(usernameEdit);
    cardLayout->addWidget(queryButton);
    cardLayout->addWidget(resetArea);
    cardLayout->addWidget(messageLabel);
    cardLayout->addStretch();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(38, 26, 38, 34); layout->setSpacing(10);
    layout->addWidget(title); layout->addWidget(subtitle); layout->addSpacing(8); layout->addWidget(card, 1);

    // 第一步：按管理员账号向后端查询密保问题。
    connect(queryButton, &QPushButton::clicked, this, [this]() {
        const QString username = usernameEdit->text().trimmed();
        if (username.isEmpty())
        { messageLabel->setText("请输入管理员账号");
            return; }
        // 防止重复点击，按钮置灰禁用
        queryButton->setEnabled(false);
        // 提示加载状态
        messageLabel->setText("正在查询密保问题...");
        // 调用网络层接口，向后端发送账号，查询对应密保问题
        network->getSecurityQuestion(username);
    });

    // 查询结果：显示密保问题，并开放答案和新密码输入区域。
    connect(network, &NetworkManager::securityQuestionFinished, this,
            [this](bool success, const QString &message, const QString &question, const QString &username) {
        // 请求结束，恢复查询按钮可用
        queryButton->setEnabled(true);
        // 分支1：查询失败（账号不存在/接口报错等）
        if (!success)
        {
            resetArea->hide();        // 隐藏密码重置输入区
            verifiedUsername.clear();   // 清空已验证账号缓存
            messageLabel->setText(message);  // 展示后端返回的错误文字
            return; }
        // 分支2：查询成功，拿到密保问题
        verifiedUsername = username; // 缓存当前验证通过的账号，后续改密码要用
        usernameEdit->setEnabled(false);
        queryButton->hide();
        questionLabel->setText("密保问题：" + question);
        resetArea->show();
        messageLabel->clear();
        answerEdit->setFocus();
    });
    // 第二步：校验输入格式并提交密保答案及新密码。
    connect(resetButton, &QPushButton::clicked, this, [this]() {
        // 校验1：密保答案不能为空
        if (answerEdit->text().trimmed().isEmpty())
        {
            messageLabel->setText("请输入密保答案");
            return;
        }
        // 校验2：新密码长度至少6位
        if (newPasswordEdit->text().length() < 6)
        {
            messageLabel->setText("新密码至少需要6位");
            return;
        }
        // 校验3：两次输入密码必须一致
        if (newPasswordEdit->text() != confirmPasswordEdit->text())
        {
            messageLabel->setText("两次输入的新密码不一致");
            return;
        }
        // 全部校验通过，进入提交流程
        // 禁用按钮防重复提交
        resetButton->setEnabled(false); messageLabel->setText("正在重置密码...");
        // 调用重置密码接口：验证账号、密保答案、新密码
        network->resetPassword(verifiedUsername, answerEdit->text(), newPasswordEdit->text());
    });

   // 密码重置后端结果回调
    connect(network, &NetworkManager::passwordResetFinished, this, [this](bool success, const QString &message) {
        resetButton->setEnabled(true);
        if (!success)
        {
            messageLabel->setText(message);
            return;
        }
         // 重置成功分支
        messageLabel->setStyleSheet("color:#2e8b57;font-size:13px;background:transparent;");
        messageLabel->setText(message);
        resetButton->setText("完成");
        disconnect(resetButton, nullptr, this, nullptr);
        connect(resetButton, &QPushButton::clicked, this, &QDialog::accept);
    });
}

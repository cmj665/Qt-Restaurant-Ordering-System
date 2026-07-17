#ifndef ADMINLOGINWIDGET_H
#define ADMINLOGINWIDGET_H

#include <QWidget>

namespace Ui {
class AdminLoginWidget;
}

/**
 * @brief 管理端登录页，负责输入校验并转交 NetworkManager 认证。
 */
class AdminLoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdminLoginWidget(QWidget *parent = nullptr);
    ~AdminLoginWidget();

signals:
    void loginSucceeded(const QString &username);

private slots:
    void submitLogin();

private:
    Ui::AdminLoginWidget *ui;
    class QLineEdit *usernameEdit;
    class QLineEdit *passwordEdit;
    class QPushButton *loginButton;
    class QLabel *messageLabel;
    class NetworkManager *network;
};

#endif // ADMINLOGINWIDGET_H

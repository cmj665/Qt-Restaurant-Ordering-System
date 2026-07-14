#ifndef ADMINLOGINWIDGET_H
#define ADMINLOGINWIDGET_H

#include <QWidget>

namespace Ui {
class AdminLoginWidget;
}

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

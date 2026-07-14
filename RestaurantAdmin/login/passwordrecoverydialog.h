#ifndef PASSWORDRECOVERYDIALOG_H
#define PASSWORDRECOVERYDIALOG_H

#include <QDialog>

class QLabel;
class QLineEdit;
class QPushButton;
class QWidget;
class NetworkManager;

class PasswordRecoveryDialog final : public QDialog
{
    Q_OBJECT
public:
    explicit PasswordRecoveryDialog(const QString &initialUsername = {}, QWidget *parent = nullptr);

private:
    NetworkManager *network;
    QLineEdit *usernameEdit;
    QLabel *questionLabel;
    QLineEdit *answerEdit;
    QLineEdit *newPasswordEdit;
    QLineEdit *confirmPasswordEdit;
    QLabel *messageLabel;
    QPushButton *queryButton;
    QPushButton *resetButton;
    QWidget *resetArea;
    QString verifiedUsername;
};

#endif

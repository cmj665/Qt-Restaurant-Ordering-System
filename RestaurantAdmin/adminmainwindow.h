#ifndef ADMINMAINWINDOW_H
#define ADMINMAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class AdminMainWindow;
}
QT_END_NAMESPACE

class AdminMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AdminMainWindow(QWidget *parent = nullptr);
    ~AdminMainWindow() override;

private:
    void showLoginPage();
    void showTablePage(const QString &username);
    Ui::AdminMainWindow *ui;
};
#endif // ADMINMAINWINDOW_H

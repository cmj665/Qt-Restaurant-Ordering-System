#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "network/networkmanager.h"
#include "widget/tablewidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class TavkeWidget;
class DishWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void showTablePage();
    void showDishPage(int tableId);

private:
    Ui::MainWindow *ui;
    NetworkManager *network;
    // TableWidget *tableWidget = nullptr;
    // DishWidget *dishWidget = nullptr;



};
#endif // MAINWINDOW_H

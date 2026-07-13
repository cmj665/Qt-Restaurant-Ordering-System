#ifndef DISHWIDGET_H
#define DISHWIDGET_H

#include <QWidget>
#include "../model/Dish.h"
#include "../network/networkmanager.h"
#include <QLabel>
#include <QScrollArea>
#include <QWidget>
#include <QScrollArea>
#include <QGridLayout>
#include "cartwidget.h"
#include <QPushButton>

namespace Ui {
class DishWidget;
}

class DishWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DishWidget(int tableId,QWidget *parent = nullptr);
    ~DishWidget();

public slots:
    void changeTable(int tableId);

signals:
    void backToTable();

private:
    Ui::DishWidget *ui;

    NetworkManager *network;

    QScrollArea *scrollArea;
    QWidget *container;
    QGridLayout *layout;

    QPushButton *cartButton;
    CartWidget *cartWidget;

    QList<Dish> dishes;

    //当前桌号
    int currentTableId;
    QLabel *titleLabel;
    QPushButton *changeTableButton;
    QPushButton *backTableButton;

    int pendingOrderId;
    double pendingMoney;


};

#endif // DISHWIDGET_H

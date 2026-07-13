#ifndef ORDERDETAILWIDGET_H
#define ORDERDETAILWIDGET_H

#include <QWidget>
#include "../network/networkmanager.h"

namespace Ui {
class OrderDetailWidget;
}

class OrderDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OrderDetailWidget(int tableId,QWidget *parent = nullptr);
    ~OrderDetailWidget();

signals:
    void payRequested(int orderId,double money);

private:
    Ui::OrderDetailWidget *ui;

    int currentTableId;
    NetworkManager *network;

    int currentOrderId;
    double currentMoney;
    int currentPayStatus;

private slots:
    void showDetail(bool success,QJsonObject data);

};

#endif // ORDERDETAILWIDGET_H

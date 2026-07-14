#ifndef ORDERDETAILWIDGET_H
#define ORDERDETAILWIDGET_H

#include <QJsonObject>
#include <QWidget>
#include "../network/networkmanager.h"

namespace Ui { class OrderDetailWidget; }

class OrderDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OrderDetailWidget(int tableId, QWidget *parent = nullptr);
    ~OrderDetailWidget();
    void refresh();

signals:
    void payRequested(int orderId, double money);

private slots:
    void showDetail(bool success, QJsonObject data);

private:
    Ui::OrderDetailWidget *ui;
    int currentTableId;
    NetworkManager *network;
    int currentOrderId = 0;
    double currentMoney = 0;
    int currentPayStatus = 0;
};

#endif // ORDERDETAILWIDGET_H

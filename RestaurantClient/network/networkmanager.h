#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QList>
#include <QJsonObject>

#include "../model/Dish.h"
#include "../model/CartItem.h"
#include "../model/DiningTable.h"

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    void getDishList();

    void submitOrder(int tableId,const QList<CartItem> &items);

    void getTableList();

    void updateTableStatus(int id,int status);

    void getUnpaidOrder(int tableId);

    void pay(int orderId,int payType);

    void getOrderDetail(int tableId);

    void checkOrderCanPay(int orderId);

signals:
    void dishListReceived(QList<Dish> &dishes);
    void orderSubmitted(bool success,const QString &message);
    void tableListReceived(QList<DiningTable> tables);
    void unpaidOrderReceived(bool hasOrder,int orderId,double money);
    void payFinished(bool success,QString message);
    void orderDetailReceived(bool success,QJsonObject data);

    //桌台状态修改完成
    void tableStatusUpdated(bool success);

    void orderCanPay(bool canPay,QString msg);

private:
    QNetworkAccessManager *manager;

private slots:
    void onFinished(QNetworkReply *reply);

};

#endif // NETWORKMANAGER_H

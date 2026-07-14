#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QList>
#include <QJsonObject>
#include <QMap>

#include "../model/Dish.h"
#include "../model/CartItem.h"
#include "../model/DiningTable.h"

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    void getDishList();
    void getDishCategories();

    void submitOrder(int tableId,const QList<CartItem> &items);

    void getTableList();

    void updateTableStatus(int id,int status);

    void cleanTable(int id);

    void getUnpaidOrder(int tableId);

    void checkoutTable(int tableId);

    void pay(int orderId,int payType);

    void getOrderDetail(int tableId);

    void checkOrderCanPay(int orderId);

signals:
    void dishListReceived(QList<Dish> &dishes);
    void dishCategoriesReceived(QMap<int, QString> categories);
    void orderSubmitted(bool success,const QString &message);
    void tableListReceived(QList<DiningTable> tables);
    void unpaidOrderReceived(bool hasOrder,int orderId,double money);
    void checkoutReady(bool success, int orderId, double money, QString message);
    void payFinished(bool success,QString message);
    void orderDetailReceived(bool success,QJsonObject data);

    //桌台状态修改完成
    void tableStatusUpdated(bool success);
    void tableCleaned(bool success, QString message);

    void orderCanPay(bool canPay,QString msg);

private:
    QNetworkAccessManager *manager;
    bool orderSubmitting = false;
    void pollOrderTask(const QString &taskId, int attempt = 0);

private slots:
    void onFinished(QNetworkReply *reply);

};

#endif // NETWORKMANAGER_H

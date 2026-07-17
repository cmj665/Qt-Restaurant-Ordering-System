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

/**
 * @brief 客户端 HTTP 接口的统一适配层。
 *
 * 负责把界面层的点餐、结账和抽奖请求序列化为 REST 请求，并将响应解析为
 * Qt 模型后通过信号返回。界面组件不应直接依赖具体 URL 或 JSON 字段。
 */
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
    void getOrderReceipt(int orderId);
    void getRewardChances(int tableId);
    void getRewardChancesByOrder(int orderId);
    void drawReward(int orderId);

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
    void orderReceiptReceived(bool success,QJsonObject data,QString message);
    void rewardChancesReceived(bool success,int orderId,int chances,QString message);
    void rewardDrawFinished(bool success,QString rewardName,int remainingChances,QString message);

    //桌台状态修改完成
    void tableStatusUpdated(bool success);
    void tableCleaned(bool success, QString message);

    void orderCanPay(bool canPay,QString msg);

private:
    QNetworkAccessManager *manager;
    // 防止用户连续点击造成同一购物车重复提交。
    bool orderSubmitting = false;
    // 异步下单返回任务编号后，短轮询任务状态直至成功、失败或超时。
    void pollOrderTask(const QString &taskId, int attempt = 0);

private slots:
    void onFinished(QNetworkReply *reply);

};

#endif // NETWORKMANAGER_H

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QList>
#include <QJsonArray>
#include "../model/diningtable.h"
#include "../model/dish.h"

class QNetworkAccessManager;

/**
 * @brief 管理端访问后端 REST API 的统一入口。
 *
 * 封装登录、桌台、菜品和出餐管理请求，并以 Qt 信号反馈结果，使各管理页面
 * 只处理展示和交互逻辑，不直接拼接 URL 或解析网络响应。
 */
class NetworkManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(NetworkManager)
public:
    explicit NetworkManager(QObject *parent = nullptr);
    void login(const QString &username, const QString &password);
    void getSecurityQuestion(const QString &username);
    void resetPassword(const QString &username, const QString &securityAnswer, const QString &newPassword);
    void getTableList();
    void cleanTable(int tableId);
    void getDishList();
    void getDishCategories();
    void addDish(const Dish &dish);
    void updateDish(const Dish &dish);
    void deleteDish(int dishId);
    void restoreDish(int dishId);
    void adjustStock(int dishId, int stock);
    void getAdminOrderItems();
    void updateOrderItemStatus(int itemId,int status);

signals:
    void loginFinished(bool success, const QString &message, const QString &username);
    void securityQuestionFinished(bool success, const QString &message, const QString &question, const QString &username);
    void passwordResetFinished(bool success, const QString &message);
    void tableListReceived(const QList<DiningTable> &tables);
    void tableListFailed(const QString &message);
    void tableCleaned(bool success, const QString &message);
    void dishListReceived(const QList<Dish> &dishes);
    void dishCategoriesReceived(const QList<DishCategory> &categories);
    void dishOperationFinished(bool success, const QString &message);
    void adminOrderItemsReceived(const QJsonArray &items);
    void orderItemOperationFinished(bool success,const QString &message);

private:
    QNetworkAccessManager *manager;
    /** add/update 共用的菜品 JSON 序列化与请求发送逻辑。 */
    void sendDish(const QString &path, const Dish &dish);
};

#endif // NETWORKMANAGER_H

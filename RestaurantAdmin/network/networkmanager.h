#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QList>
#include <QJsonArray>
#include "../model/diningtable.h"
#include "../model/dish.h"

class QNetworkAccessManager;

class NetworkManager : public QObject
{
    Q_OBJECT
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
    const QString baseUrl = "http://localhost:8080";
    void sendDish(const QString &path, const Dish &dish);
};

#endif // NETWORKMANAGER_H

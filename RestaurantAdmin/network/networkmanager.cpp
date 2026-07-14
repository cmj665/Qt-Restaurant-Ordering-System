#include "networkmanager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

namespace {
QString errorMessage(QNetworkReply *reply, const QByteArray &body)
{
    const QJsonObject object = QJsonDocument::fromJson(body).object();
    return object.value("message").toString(reply->errorString());
}
}

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent), manager(new QNetworkAccessManager(this)) {}

void NetworkManager::login(const QString &username, const QString &password)
{
    QNetworkRequest request{QUrl(baseUrl + "/admin/login")};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject body{{"username", username}, {"password", password}};
    QNetworkReply *reply = manager->post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply, username]() {
        const QByteArray data = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            emit loginFinished(false, errorMessage(reply, data), {});
        } else {
            const QJsonObject object = QJsonDocument::fromJson(data).object();
            emit loginFinished(true, "登录成功", object.value("username").toString(username));
        }
        reply->deleteLater();
    });
}

void NetworkManager::getTableList()
{
    QNetworkReply *reply = manager->get(QNetworkRequest(QUrl(baseUrl + "/table/list")));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const QByteArray data = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            emit tableListFailed(errorMessage(reply, data));
        } else {
            QList<DiningTable> tables;
            for (const QJsonValue &value : QJsonDocument::fromJson(data).array()) {
                const QJsonObject object = value.toObject();
                tables.append({object.value("id").toInt(), object.value("tableName").toString(),
                               object.value("capacity").toInt(), object.value("status").toInt()});
            }
            emit tableListReceived(tables);
        }
        reply->deleteLater();
    });
}

void NetworkManager::cleanTable(int tableId)
{
    QNetworkRequest request{QUrl(baseUrl + QString("/table/clean/%1").arg(tableId))};
    QNetworkReply *reply = manager->post(request, QByteArray());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const QByteArray data = reply->readAll();
        const bool success = reply->error() == QNetworkReply::NoError;
        emit tableCleaned(success, success ? QString::fromUtf8(data) : errorMessage(reply, data));
        reply->deleteLater();
    });
}

void NetworkManager::getDishList()
{
    QNetworkReply *reply=manager->get(QNetworkRequest(QUrl(baseUrl+"/dish/admin/list")));
    connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();if(reply->error()!=QNetworkReply::NoError){emit dishOperationFinished(false,errorMessage(reply,data));}else{QList<Dish>dishes;for(const QJsonValue &value:QJsonDocument::fromJson(data).array()){const QJsonObject o=value.toObject();dishes.append({o["id"].toInt(),o["catId"].toInt(),o["name"].toString(),o["price"].toDouble(),o["stock"].toInt(),o["picture"].toString(),o["description"].toString(),o["isDeleted"].toInt(),o["soldCount"].toInt()});}emit dishListReceived(dishes);}reply->deleteLater();});
}
void NetworkManager::getDishCategories()
{
    QNetworkReply *reply=manager->get(QNetworkRequest(QUrl(baseUrl+"/dish/categories")));
    connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();QList<DishCategory>items;if(reply->error()==QNetworkReply::NoError){for(const QJsonValue&value:QJsonDocument::fromJson(data).array()){const QJsonObject o=value.toObject();items.append({o["id"].toInt(),o["name"].toString()});}}emit dishCategoriesReceived(items);reply->deleteLater();});
}
void NetworkManager::sendDish(const QString &path,const Dish &dish)
{
    QJsonObject object{{"id",dish.id},{"catId",dish.catId},{"name",dish.name},{"price",dish.price},{"stock",dish.stock},{"picture",dish.picture},{"description",dish.description}};QNetworkRequest request{QUrl(baseUrl+path)};request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");QNetworkReply *reply=manager->post(request,QJsonDocument(object).toJson(QJsonDocument::Compact));connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();const bool ok=reply->error()==QNetworkReply::NoError;emit dishOperationFinished(ok,ok?QString::fromUtf8(data):errorMessage(reply,data));reply->deleteLater();});
}
void NetworkManager::addDish(const Dish&dish){sendDish("/dish/add",dish);}
void NetworkManager::updateDish(const Dish&dish){sendDish("/dish/update",dish);}
void NetworkManager::deleteDish(int id){QNetworkRequest request{QUrl(baseUrl+QString("/dish/delete/%1").arg(id))};QNetworkReply*reply=manager->post(request,QByteArray());connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();const bool ok=reply->error()==QNetworkReply::NoError;emit dishOperationFinished(ok,ok?QString::fromUtf8(data):errorMessage(reply,data));reply->deleteLater();});}
void NetworkManager::restoreDish(int id){QNetworkRequest request{QUrl(baseUrl+QString("/dish/restore/%1").arg(id))};QNetworkReply*reply=manager->post(request,QByteArray());connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();const bool ok=reply->error()==QNetworkReply::NoError;emit dishOperationFinished(ok,ok?QString::fromUtf8(data):errorMessage(reply,data));reply->deleteLater();});}
void NetworkManager::adjustStock(int id,int stock){QNetworkRequest request{QUrl(baseUrl+"/dish/stock")};request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");QJsonObject object{{"dishId",id},{"stock",stock}};QNetworkReply*reply=manager->post(request,QJsonDocument(object).toJson(QJsonDocument::Compact));connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();const bool ok=reply->error()==QNetworkReply::NoError;emit dishOperationFinished(ok,ok?QString::fromUtf8(data):errorMessage(reply,data));reply->deleteLater();});}

void NetworkManager::getAdminOrderItems(){QNetworkReply*reply=manager->get(QNetworkRequest(QUrl(baseUrl+"/order/admin/items")));connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();if(reply->error()==QNetworkReply::NoError)emit adminOrderItemsReceived(QJsonDocument::fromJson(data).array());else emit orderItemOperationFinished(false,errorMessage(reply,data));reply->deleteLater();});}
void NetworkManager::updateOrderItemStatus(int itemId,int status){QNetworkRequest request{QUrl(baseUrl+QString("/order/item/%1/status/%2").arg(itemId).arg(status))};QNetworkReply*reply=manager->post(request,QByteArray());connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();const bool ok=reply->error()==QNetworkReply::NoError;emit orderItemOperationFinished(ok,ok?QString::fromUtf8(data):errorMessage(reply,data));reply->deleteLater();});}

#include "networkmanager.h"
#include "serverconfig.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

namespace {
const QString &baseUrl = ServerConfig::BaseUrl;

// 网络错误解析：优先读取后端 JSON 中的 message，否则使用 Qt 的网络错误文本。
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
    // 管理员登录：向后端提交账号密码，并通过信号返回登录结果。
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

void NetworkManager::getSecurityQuestion(const QString &username)
{
    // 密保问题查询：按管理员账号获取其已设置的密保问题。
    QUrl url(baseUrl + "/admin/security-question");
    QUrlQuery query;
    query.addQueryItem("username", username);
    url.setQuery(query);
    QNetworkReply *reply = manager->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, username]() {
        const QByteArray data = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            emit securityQuestionFinished(false, errorMessage(reply, data), {}, username);
        } else {
            const QJsonObject object = QJsonDocument::fromJson(data).object();
            emit securityQuestionFinished(true, {}, object.value("question").toString(),
                                          object.value("username").toString(username));
        }
        reply->deleteLater();
    });
}

void NetworkManager::resetPassword(const QString &username, const QString &securityAnswer, const QString &newPassword)
{
    // 密码重置：提交密保答案和新密码，由后端完成校验与更新。
    QNetworkRequest request{QUrl(baseUrl + "/admin/reset-password")};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject body{{"username", username}, {"securityAnswer", securityAnswer}, {"newPassword", newPassword}};
    QNetworkReply *reply = manager->post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const QByteArray data = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            emit passwordResetFinished(false, errorMessage(reply, data));
        } else {
            const QJsonObject object = QJsonDocument::fromJson(data).object();
            emit passwordResetFinished(true, object.value("message").toString("密码重置成功"));
        }
        reply->deleteLater();
    });
}

void NetworkManager::getTableList()
{
    // 桌台数据：读取后端桌台列表并把 JSON 转换为 DiningTable 对象。
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
    // 清台操作：通知后端结束当前桌台服务并恢复空闲状态。
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
    // 菜品列表：读取包含在售、下架、库存和销量信息的管理端菜品数据。
    QNetworkReply *reply=manager->get(QNetworkRequest(QUrl(baseUrl+"/dish/admin/list")));
    connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();if(reply->error()!=QNetworkReply::NoError){emit dishOperationFinished(false,errorMessage(reply,data));}else{QList<Dish>dishes;for(const QJsonValue &value:QJsonDocument::fromJson(data).array()){const QJsonObject o=value.toObject();dishes.append({o["id"].toInt(),o["catId"].toInt(),o["name"].toString(),o["price"].toDouble(),o["stock"].toInt(),o["picture"].toString(),o["description"].toString(),o["isDeleted"].toInt(),o["soldCount"].toInt()});}emit dishListReceived(dishes);}reply->deleteLater();});
}
void NetworkManager::getDishCategories()
{
    // 菜品分类：读取分类编号和名称，供新增、修改菜品时选择。
    QNetworkReply *reply=manager->get(QNetworkRequest(QUrl(baseUrl+"/dish/categories")));
    connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();QList<DishCategory>items;if(reply->error()==QNetworkReply::NoError){for(const QJsonValue&value:QJsonDocument::fromJson(data).array()){const QJsonObject o=value.toObject();items.append({o["id"].toInt(),o["name"].toString()});}}emit dishCategoriesReceived(items);reply->deleteLater();});
}
void NetworkManager::sendDish(const QString &path,const Dish &dish)
{
    // 菜品保存公共逻辑：把菜品转换为 JSON，供新增和修改接口复用。
    QJsonObject object{{"id",dish.id},{"catId",dish.catId},{"name",dish.name},{"price",dish.price},{"stock",dish.stock},{"picture",dish.picture},{"description",dish.description}};QNetworkRequest request{QUrl(baseUrl+path)};request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");QNetworkReply *reply=manager->post(request,QJsonDocument(object).toJson(QJsonDocument::Compact));connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();const bool ok=reply->error()==QNetworkReply::NoError;emit dishOperationFinished(ok,ok?QString::fromUtf8(data):errorMessage(reply,data));reply->deleteLater();});
}
// 菜品维护接口：新增、修改、上下架以及库存调整。
void NetworkManager::addDish(const Dish&dish){sendDish("/dish/add",dish);}
void NetworkManager::updateDish(const Dish&dish){sendDish("/dish/update",dish);}
void NetworkManager::deleteDish(int id){QNetworkRequest request{QUrl(baseUrl+QString("/dish/delete/%1").arg(id))};QNetworkReply*reply=manager->post(request,QByteArray());connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();const bool ok=reply->error()==QNetworkReply::NoError;emit dishOperationFinished(ok,ok?QString::fromUtf8(data):errorMessage(reply,data));reply->deleteLater();});}
void NetworkManager::restoreDish(int id){QNetworkRequest request{QUrl(baseUrl+QString("/dish/restore/%1").arg(id))};QNetworkReply*reply=manager->post(request,QByteArray());connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();const bool ok=reply->error()==QNetworkReply::NoError;emit dishOperationFinished(ok,ok?QString::fromUtf8(data):errorMessage(reply,data));reply->deleteLater();});}
void NetworkManager::adjustStock(int id,int stock){QNetworkRequest request{QUrl(baseUrl+"/dish/stock")};request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");QJsonObject object{{"dishId",id},{"stock",stock}};QNetworkReply*reply=manager->post(request,QJsonDocument(object).toJson(QJsonDocument::Compact));connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();const bool ok=reply->error()==QNetworkReply::NoError;emit dishOperationFinished(ok,ok?QString::fromUtf8(data):errorMessage(reply,data));reply->deleteLater();});}

// 出餐管理接口：读取所有待处理订单项，并更新制作、出餐或取消状态。
void NetworkManager::getAdminOrderItems(){QNetworkReply*reply=manager->get(QNetworkRequest(QUrl(baseUrl+"/order/admin/items")));connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();if(reply->error()==QNetworkReply::NoError)emit adminOrderItemsReceived(QJsonDocument::fromJson(data).array());else emit orderItemOperationFinished(false,errorMessage(reply,data));reply->deleteLater();});}
void NetworkManager::updateOrderItemStatus(int itemId,int status){QNetworkRequest request{QUrl(baseUrl+QString("/order/item/%1/status/%2").arg(itemId).arg(status))};QNetworkReply*reply=manager->post(request,QByteArray());connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();const bool ok=reply->error()==QNetworkReply::NoError;emit orderItemOperationFinished(ok,ok?QString::fromUtf8(data):errorMessage(reply,data));reply->deleteLater();});}

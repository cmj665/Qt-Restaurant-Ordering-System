#include "networkmanager.h"

#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>

#include "../model/Dish.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QUrlQuery>

#include "../model/CartItem.h"

NetworkManager::NetworkManager(QObject *parent)
    : QObject{parent}
{
    manager = new QNetworkAccessManager(this);
    //不再使用这个全局连接
    // connect(manager,&QNetworkAccessManager::finished,this,&NetworkManager::onFinished);

}


void NetworkManager::getDishList(){
    QUrl url("http://localhost:8080/dish/list");
    QNetworkRequest request(url);
    QNetworkReply *reply = manager->get(request);

    connect(reply,&QNetworkReply::finished,this,[this,reply](){
        if(reply->error()!=QNetworkReply::NoError)
        {
            qDebug()<<"获取菜品失败："<<reply->errorString();
            reply->deleteLater();
            return;
        }

        const QByteArray data = reply->readAll();
        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(data,&parseError);

        if(parseError.error!=QJsonParseError::NoError)
        {
            qDebug()<<"菜品接口返回的不是JSON数组";
            reply->deleteLater();
            return;

        }

        const QJsonArray array = document.array();
        QList<Dish> dishes;
        for(const QJsonValue &value : array)
        {
            const QJsonObject object = value.toObject();
            Dish dish;
            dish.id = object["id"].toInt();
            dish.catId =object["catId"].toInt();
            dish.name = object["name"].toString();
            dish.price = object["price"].toDouble();
            dish.stock =object["stock"].toInt();
            dish.picture = object["picture"].toString();
            dish.description =object["description"].toString();
            dishes.append(dish);
        }
        emit dishListReceived(dishes);
        reply->deleteLater();

    });


}

void NetworkManager::onFinished(QNetworkReply *reply){

    if(reply->error()!=QNetworkReply::NoError)
    {
        qDebug()<<"请求失败："<<reply->errorString();
        return;
    }

    QByteArray data = reply->readAll();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray array = doc.array();
    QList<Dish> dishes;
    for(auto value : array)
    {
        QJsonObject obj =value.toObject();
        Dish dish;
        dish.id = obj["id"].toInt();
        dish.catId = obj["catId"].toInt();
        dish.name = obj["name"].toString();
        dish.price = obj["price"].toDouble();
        dish.stock = obj["stock"].toInt();
        dish.picture = obj["picture"].toString();
        dish.description = obj["description"].toString();
        dishes.append(dish);
        ;
    }
    emit dishListReceived(dishes);
    reply->deleteLater();
}

void NetworkManager::submitOrder(int tableId,const QList<CartItem> &items){
    QJsonObject rootObject;
    rootObject["tableId"] = tableId;
    QJsonArray itemArray;
    for(const CartItem &cartItem : items)
    {
        QJsonObject itemObject;
        itemObject["dishId"] = cartItem.dish.id;
        itemObject["count"] = cartItem.count;
        itemArray.append(itemObject);
    }
    rootObject["items"] = itemArray;

    QJsonDocument document(rootObject);
    QByteArray jsonData = document.toJson(QJsonDocument::Compact);

    qDebug()<<"即将提交的订单JSON："<<QString::fromUtf8(jsonData);
    QNetworkRequest request{QUrl("http://localhost:8080/order/submit")};
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    QNetworkReply *reply = manager->post(request,jsonData);
    connect(reply,&QNetworkReply::finished,this,[this,reply](){
        const QByteArray responseData = reply->readAll();
        if(reply->error()!=QNetworkReply::NoError)
        {
            const QString message = QString::fromUtf8(responseData).isEmpty()
                                        ?reply->errorString():QString::fromUtf8(responseData);
            qDebug()<<"提交订单失败"<<message;
            emit orderSubmitted(false,message);

            reply->deleteLater();
            return;
        }
        const QString message =QString::fromUtf8(responseData);
        qDebug()<<"提交订单成功，服务器返回："<<message;
        emit orderSubmitted(true,message);
        reply->deleteLater();
    });


}

void NetworkManager::getTableList()
{
    QUrl url("http://localhost:8080/table/list");
    QNetworkRequest request(url);
    QNetworkReply *reply=manager->get(request);

    connect(reply,&QNetworkReply::finished,this,[this,reply](){
        if(reply->error()!=QNetworkReply::NoError)
        {
            qDebug()<<"桌台请求失败："<<reply->errorString();
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray array=doc.array();

        QList<DiningTable> tables;
        for(QJsonValue value:array)
        {
            QJsonObject obj = value.toObject();
            DiningTable table;
            table.id = obj["id"].toInt();
            table.tableName = obj["tableName"].toString();
            table.capacity = obj["capacity"].toInt();
            table.status = obj["status"].toInt();
            tables.append(table);

        }
        emit tableListReceived(tables);
        reply->deleteLater();

    });
}

void NetworkManager::updateTableStatus(int id,int status){
    QUrl url("http://localhost:8080/table/status");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
        "application/json");

    // QUrlQuery params;
    // params.addQueryItem("id",QString::number(id));
    // params.addQueryItem("status",QString::number(status));
    // QByteArray data = params.toString(QUrl::FullyEncoded).toUtf8();

    QJsonObject obj;
    obj["tableId"] = id;
    obj["status"] = status;
    QJsonDocument doc(obj);



    qDebug()<<"发送状态:"<<id<<status;
    QNetworkReply *reply = manager->post(request,doc.toJson());

    connect(reply,&QNetworkReply::finished,this,[this,reply](){
        if(reply->error()!=QNetworkReply::NoError)
        {
            emit tableStatusUpdated(false);
            reply->deleteLater();
            return;
        }
        emit tableStatusUpdated(true);
        reply->deleteLater();


    });

}

void NetworkManager::getUnpaidOrder(int tableId)
{
    QUrl url = (QString("http://localhost:8080/order/unpaid/%1").arg(tableId));
    QNetworkRequest request(url);
    QNetworkReply *reply = manager->get(request);
    connect(reply,&QNetworkReply::finished,this,[this,reply](){
        QByteArray data = reply->readAll();
        if(reply->error()!=QNetworkReply::NoError)
        {
            emit unpaidOrderReceived(false,0,0);
            reply->deleteLater();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isNull()||doc.isEmpty())
        {
            emit unpaidOrderReceived(false,0,0);
            reply->deleteLater();
            return;
        }

        QJsonObject obj =doc.object();
        int orderId =obj["id"].toInt();
        double money = obj["totalPrice"].toDouble();
        qDebug()
            <<"订单ID:"
            <<orderId
            <<"金额:"
            <<money;

        emit unpaidOrderReceived(true,orderId,money);
        reply->deleteLater();

    });

}

void NetworkManager::pay(int orderId,int payType){
    QUrl url("http://localhost:8080/payment/pay");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    QJsonObject obj;
    obj["orderId"] = orderId;
    obj["payType"] = payType;
    qDebug()
        <<"支付请求:"
        <<orderId
        <<payType;
    QJsonDocument doc(obj);
    QByteArray data =doc.toJson();
    QNetworkReply *reply = manager->post(request,data);
    connect(reply,&QNetworkReply::finished,this,[this,reply](){
        QByteArray data = reply->readAll();
        if(reply->error()!=QNetworkReply::NoError)
        {
            QJsonParseError error;
            QJsonDocument doc =QJsonDocument::fromJson(data,&error);
            QString msg="支付失败";

            if(error.error==QJsonParseError::NoError)
            {
                QJsonObject obj=doc.object();
                if(obj.contains("message"))
                {
                    msg=obj["message"].toString();
                }
            }

            emit payFinished(false,msg);
            reply->deleteLater();
            return;
        }
        else
        {
            emit payFinished(true,"支付成功"/*QString::fromUtf8(result)*/);
        }

        reply->deleteLater();
    });

}


void NetworkManager::getOrderDetail(int tableId){
    QUrl url(QString("http://localhost:8080/order/detail/%1").arg(tableId));
    QNetworkRequest request(url);
    QNetworkReply *reply = manager->get(request);
    connect(reply,&QNetworkReply::finished,this,[this,reply](){
        QByteArray data = reply->readAll();
        if(reply->error()!=QNetworkReply::NoError)
        {
            emit orderDetailReceived(false,QJsonObject());
            reply->deleteLater();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(!doc.isObject())
        {
            emit orderDetailReceived(false,QJsonObject());
            reply->deleteLater();
            return;
        }
        emit orderDetailReceived(true,doc.object());
        reply->deleteLater();

    });

}


void NetworkManager::checkOrderCanPay(int orderId)
{

    QUrl url(
        QString(
            "http://localhost:8080/order/canPay/%1"
            )
            .arg(orderId)
        );

    QNetworkRequest request(url);
    QNetworkReply *reply =
        manager->get(request);
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this,reply](){

                QByteArray data =
                    reply->readAll();

                QJsonDocument doc =
                    QJsonDocument::fromJson(data);

                QJsonObject obj =
                    doc.object();

                bool canPay =
                    obj["canPay"].toBool();

                QString msg =
                    obj["message"].toString();

                emit orderCanPay(
                    canPay,
                    msg
                    );

                reply->deleteLater();

            });

}

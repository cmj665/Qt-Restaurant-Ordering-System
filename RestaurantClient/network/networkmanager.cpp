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
#include <QTimer>

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
            dish.soldCount = object["soldCount"].toInt();
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
    if(orderSubmitting)
    {
        //orderSubmitting用于防止用户连续点击：
        emit orderSubmitted(false, "订单正在后台处理中，请勿重复提交");
        return;
    }
    orderSubmitting = true;

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
            orderSubmitting = false;
            emit orderSubmitted(false,message);

            reply->deleteLater();
            return;
        }
        QJsonParseError parseError;
        const QJsonDocument response = QJsonDocument::fromJson(responseData, &parseError);
        const QString taskId = response.object()["taskId"].toString();
        if(parseError.error != QJsonParseError::NoError || taskId.isEmpty())
        {
            orderSubmitting = false;
            emit orderSubmitted(false, "服务器未返回订单任务编号");
            reply->deleteLater();
            return;
        }

        qDebug()<<"订单已进入后台队列，任务编号："<<taskId;
        pollOrderTask(taskId);
        reply->deleteLater();
    });


}

void NetworkManager::getDishCategories()
{
    QNetworkReply *reply = manager->get(QNetworkRequest(QUrl("http://localhost:8080/dish/categories")));
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        QMap<int, QString> categories;
        if(reply->error() == QNetworkReply::NoError)
        {
            const QJsonArray array = QJsonDocument::fromJson(reply->readAll()).array();
            for(const QJsonValue &value : array)
            {
                const QJsonObject object = value.toObject();
                categories.insert(object["id"].toInt(), object["name"].toString());
            }
        }
        emit dishCategoriesReceived(categories);
        reply->deleteLater();
    });
}


//客户端轮询代码
void NetworkManager::pollOrderTask(const QString &taskId, int attempt)
{
    //最多轮询 60 次间隔 500ms，成功 / 失败则结束轮询，超时 / 网络错误 / 处理中都延迟重试。
    if(attempt >= 60)
    {
        orderSubmitting = false;
        emit orderSubmitted(false, "订单处理超时，请稍后查看订单状态");
        return;
    }

    QNetworkRequest request(QUrl(QString("http://localhost:8080/order/task/%1").arg(taskId)));
    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, taskId, attempt](){
        const QByteArray data = reply->readAll();
        if(reply->error() != QNetworkReply::NoError)
        {
            reply->deleteLater();
            QTimer::singleShot(500, this, [this, taskId, attempt](){
                pollOrderTask(taskId, attempt + 1);
            });
            return;
        }

        const QJsonObject result = QJsonDocument::fromJson(data).object();
        const QString status = result["status"].toString();
        const QString message = result["message"].toString();
        reply->deleteLater();

        //状态判断：
        if(status == "SUCCESS")
        {
            orderSubmitting = false;
            emit orderSubmitted(true, message);
        }
        else if(status == "FAILED")
        {
            orderSubmitting = false;
            emit orderSubmitted(false, message);
        }
        else
        {
            QTimer::singleShot(500, this, [this, taskId, attempt](){
                pollOrderTask(taskId, attempt + 1);
            });
        }
    });
}

void NetworkManager::getTableList()
{
    QUrl url("http://localhost:8080/table/list");
    QNetworkRequest request(url);
    //manager 是类内 QNetworkAccessManager 网络管理器
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
            table.id = obj["id"].toInt();      //读取 json 里id字段，转整数赋值给桌台 id。
            table.tableName = obj["tableName"].toString();
            table.capacity = obj["capacity"].toInt();
            table.status = obj["status"].toInt();
            tables.append(table);

        }
        //发射信号，把完整桌台列表传给绑定的 TableWidget::showTables 函数，渲染界面桌台卡片
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

void NetworkManager::cleanTable(int id)
{
    QNetworkRequest request(QUrl(QString("http://localhost:8080/table/clean/%1").arg(id)));
    QNetworkReply *reply = manager->post(request, QByteArray());
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        const QByteArray data = reply->readAll();
        if(reply->error() != QNetworkReply::NoError)
        {
            QString message = reply->errorString();
            const QJsonObject error = QJsonDocument::fromJson(data).object();
            if(error.contains("message"))
                message = error["message"].toString();
            emit tableCleaned(false, message);
        }
        else
        {
            emit tableCleaned(true, QString::fromUtf8(data));
        }
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

void NetworkManager::checkoutTable(int tableId)
{
    QNetworkRequest request(QUrl(QString("http://localhost:8080/order/checkout/%1").arg(tableId)));
    QNetworkReply *reply = manager->post(request, QByteArray());
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        const QByteArray data = reply->readAll();
        const QJsonObject object = QJsonDocument::fromJson(data).object();
        if(reply->error() != QNetworkReply::NoError)
        {
            const QString message = object["message"].toString(reply->errorString());
            emit checkoutReady(false, 0, 0, message);
        }
        else
        {
            emit checkoutReady(true, object["id"].toInt(), object["totalPrice"].toDouble(), "可以结账");
        }
        reply->deleteLater();
    });
}


void NetworkManager::pay(int orderId,int payType){
    QUrl url("http://localhost:8080/payment/pay");
    //创建网络请求对象，绑定接口地址
    QNetworkRequest request(url);
    //设置请求头 Content-Type: application/json，告诉后端本次请求传递 JSON 数据
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    //组装 JSON 请求体
    QJsonObject obj;
    obj["orderId"] = orderId;
    obj["payType"] = payType;
    qDebug()
        <<"支付请求:"
        <<orderId
        <<payType;

    //QJsonDocument 把 JSON 对象转为二进制字节数组，网络 POST 只能发字节流
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

            //如果后端有返回合法JSON，读取后端自定义错误提示message
            if(error.error==QJsonParseError::NoError)
            {
                QJsonObject obj=doc.object();
                if(obj.contains("message"))
                {
                    msg=obj["message"].toString();
                }
            }

            //向外发射失败信号，传给 PayWidget 界面
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

void NetworkManager::getOrderReceipt(int orderId){
    QNetworkReply *reply=manager->get(QNetworkRequest(QUrl(QString("http://localhost:8080/order/receipt/%1").arg(orderId))));
    connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();if(reply->error()!=QNetworkReply::NoError){QString message=reply->errorString();QJsonObject error=QJsonDocument::fromJson(data).object();if(error.contains("message"))message=error["message"].toString();emit orderReceiptReceived(false,QJsonObject(),message);}else emit orderReceiptReceived(true,QJsonDocument::fromJson(data).object(),QString());reply->deleteLater();});
}

void NetworkManager::getRewardChances(int tableId){
    QNetworkReply*reply=manager->get(QNetworkRequest(QUrl(QString("http://localhost:8080/reward/chances/table/%1").arg(tableId))));
    connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();if(reply->error()!=QNetworkReply::NoError){QJsonObject e=QJsonDocument::fromJson(data).object();emit rewardChancesReceived(false,0,0,e["message"].toString(reply->errorString()));}else{QJsonObject o=QJsonDocument::fromJson(data).object();emit rewardChancesReceived(true,o["orderId"].toInt(),o["chances"].toInt(),QString());}reply->deleteLater();});
}
void NetworkManager::getRewardChancesByOrder(int orderId){
    QNetworkReply*reply=manager->get(QNetworkRequest(QUrl(QString("http://localhost:8080/reward/chances/order/%1").arg(orderId))));
    connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();if(reply->error()!=QNetworkReply::NoError){QJsonObject e=QJsonDocument::fromJson(data).object();emit rewardChancesReceived(false,0,0,e["message"].toString(reply->errorString()));}else{QJsonObject o=QJsonDocument::fromJson(data).object();emit rewardChancesReceived(true,o["orderId"].toInt(),o["chances"].toInt(),QString());}reply->deleteLater();});
}
void NetworkManager::drawReward(int orderId){
    QNetworkRequest request{QUrl(QString("http://localhost:8080/reward/draw/%1").arg(orderId))};QNetworkReply*reply=manager->post(request,QByteArray());
    connect(reply,&QNetworkReply::finished,this,[this,reply](){const QByteArray data=reply->readAll();QJsonObject o=QJsonDocument::fromJson(data).object();if(reply->error()!=QNetworkReply::NoError)emit rewardDrawFinished(false,QString(),0,o["message"].toString(reply->errorString()));else emit rewardDrawFinished(true,o["rewardName"].toString(),o["remainingChances"].toInt(),QString());reply->deleteLater();});
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

#include "orderdetailwidget.h"
#include "ui_orderdetailwidget.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>

OrderDetailWidget::OrderDetailWidget(int tableId,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OrderDetailWidget)
    ,currentTableId(tableId)
    ,currentOrderId(0)
    ,currentMoney(0)
    ,currentPayStatus(0)
{
    ui->setupUi(this);

    network = new NetworkManager(this);
    connect(network,&NetworkManager::orderDetailReceived,
            this,&OrderDetailWidget::showDetail);

    connect(ui->payButton,&QPushButton::clicked,this,[this](){
        if(currentOrderId==0)
        {
            QMessageBox::warning(this,"提示","没有可支付订单");
            return;
        }

        if(currentPayStatus==1)
        {
            QMessageBox::information(this,"提示","该订单已经支付");
            return;
        }

        emit payRequested(currentOrderId,currentMoney);
    });


    network->getOrderDetail(tableId);

    connect(ui->closeButton,&QPushButton::clicked,this,[this](){
        this->close();
    });
}

void OrderDetailWidget::showDetail(bool success,QJsonObject data){
    if(!success)
    {
        ui->nameLabel->setText("没有订单");
        return;
    }

    //先保存订单信息
    currentOrderId =data["orderId"].toInt();
    currentMoney =data["totalPrice"].toDouble();
    currentPayStatus = data["payStatus"].toInt();

    ui->nameLabel->setText(QString("%1号桌订单").arg(currentTableId));

    QJsonArray items = data["items"].toArray();

    ui->orderList->clear();

    for(auto item:items)
    {

        QJsonObject obj =item.toObject();
        QString text =QString("%1   x%2   ￥%3")
                .arg(
                    obj["dishName"]
                        .toString()
                    )

                .arg(
                    obj["count"]
                        .toInt()
                    )

                .arg(
                    obj["price"]
                        .toDouble()
                    );

        ui->orderList->addItem(text);

    }


    //显示金额
    ui->totalLabel->setText(QString("总价:%1元").arg(data["totalPrice"].toDouble()));

    if(currentPayStatus==1)
    {
        ui->payButton->setText("已支付");
        ui->payButton->setEnabled(false);
    }
    else
    {
        ui->payButton->setText("立即支付");
        ui->payButton->setEnabled(true);
    }


}







OrderDetailWidget::~OrderDetailWidget()
{
    delete ui;
}

#include "orderdetailwidget.h"
#include "ui_orderdetailwidget.h"

#include <QJsonArray>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

OrderDetailWidget::OrderDetailWidget(int tableId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OrderDetailWidget)
    , currentTableId(tableId)
    , network(new NetworkManager(this))
{
    ui->setupUi(this);
    setWindowTitle("订单详情");
    resize(600, 680);
    setMinimumSize(520, 560);
    QWidget *content = ui->verticalLayout->parentWidget();
    QVBoxLayout *windowLayout = new QVBoxLayout(this);
    windowLayout->setContentsMargins(18, 18, 18, 18);
    windowLayout->addWidget(content);
    setLayout(windowLayout);
    ui->verticalLayout->setSpacing(14);

    ui->nameLabel->setAlignment(Qt::AlignCenter);
    ui->nameLabel->setStyleSheet("font-size:27px;font-weight:bold;color:#2c3e50;padding:10px;");
    ui->orderList->setStyleSheet(
        "QListWidget{font-size:17px;border:1px solid #dcdcdc;border-radius:8px;padding:8px;background:#fafafa;}"
        "QListWidget::item{border-bottom:1px solid #eee;padding:8px;}"
    );
    ui->totalLabel->setAlignment(Qt::AlignRight);
    ui->totalLabel->setStyleSheet("font-size:23px;font-weight:bold;color:#e67e22;padding:10px;");
    ui->payButton->setMinimumHeight(52);
    ui->payButton->setStyleSheet(
        "QPushButton{font-size:18px;background:#27ae60;color:white;border-radius:8px;}"
        "QPushButton:disabled{background:#aaa;}"
    );
    ui->closeButton->setText("关闭");
    ui->closeButton->setMinimumHeight(42);

    connect(network, &NetworkManager::orderDetailReceived, this, &OrderDetailWidget::showDetail);
    connect(ui->payButton, &QPushButton::clicked, this, [this](){
        if(currentOrderId <= 0 || currentPayStatus == 1)
            return;
        emit payRequested(currentOrderId, currentMoney);
    });
    connect(ui->closeButton, &QPushButton::clicked, this, &QWidget::close);
    refresh();
}

void OrderDetailWidget::refresh()
{
    ui->nameLabel->setText(QString("%1号桌 · 订单详情").arg(currentTableId));
    ui->orderList->clear();
    ui->orderList->addItem("正在加载订单...");
    ui->payButton->setEnabled(false);
    network->getOrderDetail(currentTableId);
}

void OrderDetailWidget::showDetail(bool success, QJsonObject data)
{
    ui->orderList->clear();
    if(!success || data.isEmpty())
    {
        currentOrderId = 0;
        currentMoney = 0;
        currentPayStatus = 1;
        ui->nameLabel->setText(QString("%1号桌 · 暂无待支付订单").arg(currentTableId));
        ui->orderList->addItem("当前桌台没有可显示的未支付订单");
        ui->totalLabel->setText("合计：￥0.00");
        ui->payButton->setText("暂无可支付订单");
        ui->payButton->setEnabled(false);
        return;
    }

    currentOrderId = data["orderId"].toInt();
    currentMoney = data["totalPrice"].toDouble();
    currentPayStatus = data["payStatus"].toInt();
    ui->nameLabel->setText(QString("%1号桌 · 订单 #%2").arg(currentTableId).arg(currentOrderId));

    const QJsonArray items = data["items"].toArray();
    int index = 1;
    for(const QJsonValue &value : items)
    {
        const QJsonObject item = value.toObject();
        const int count = item["count"].toInt();
        const double price = item["price"].toDouble();
        QListWidgetItem *row = new QListWidgetItem(
            QString("%1. %2\n    ￥%3 × %4    小计 ￥%5")
                .arg(index++).arg(item["dishName"].toString())
                .arg(price, 0, 'f', 2).arg(count).arg(price * count, 0, 'f', 2)
        );
        row->setSizeHint(QSize(0, 64));
        ui->orderList->addItem(row);
    }

    ui->totalLabel->setText(QString("订单合计：￥%1").arg(currentMoney, 0, 'f', 2));
    ui->payButton->setText(currentPayStatus == 1 ? "订单已支付" : "进入支付");
    ui->payButton->setEnabled(currentPayStatus != 1);
}

OrderDetailWidget::~OrderDetailWidget()
{
    delete ui;
}

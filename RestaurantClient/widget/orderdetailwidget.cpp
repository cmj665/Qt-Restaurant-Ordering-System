#include "orderdetailwidget.h"
#include "ui_orderdetailwidget.h"

#include <QJsonArray>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTimer>
#include <QColor>

OrderDetailWidget::OrderDetailWidget(int tableId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OrderDetailWidget)
    , currentTableId(tableId)
    , network(new NetworkManager(this))
    , refreshTimer(new QTimer(this))
{
    ui->setupUi(this);
    //--------------UI---------------------
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

    //------------绑定事件-------------------------
    connect(network, &NetworkManager::orderDetailReceived, this, &OrderDetailWidget::showDetail);
    connect(ui->payButton, &QPushButton::clicked, this, [this](){
        if(currentOrderId <= 0 || currentPayStatus == 1)
            return;
        emit payRequested(currentOrderId, currentMoney);
    });
    connect(ui->closeButton, &QPushButton::clicked, this, &QWidget::close);
    // // 2秒定时自动刷新订单
    refreshTimer->setInterval(2000);
    connect(refreshTimer,&QTimer::timeout,this,[this](){network->getOrderDetail(currentTableId);});
    refreshTimer->start();
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

//后端返回 JSON 后渲染页面
void OrderDetailWidget::showDetail(bool success, QJsonObject data)
{
    ui->orderList->clear();
    //请求失败 / 无订单分支
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

    //读取订单基础信息
    currentOrderId = data["orderId"].toInt();
    currentMoney = data["totalPrice"].toDouble();
    currentPayStatus = data["payStatus"].toInt();
    ui->nameLabel->setText(QString("%1号桌 · 订单 #%2").arg(currentTableId).arg(currentOrderId));

    //循环渲染每一条菜品明细
    const QJsonArray items = data["items"].toArray();
    int index = 1;
    int pendingCount = 0;  //循环渲染每一条菜品明细
    for(const QJsonValue &value : items)
    {
        const QJsonObject item = value.toObject();
        const int count = item["count"].toInt();
        const double price = item["price"].toDouble();
        const int itemStatus = item["itemStatus"].toInt();
         // itemStatus：0未出餐、1已出餐、2已取消
        if(itemStatus == 0) ++pendingCount;
        const QString state = itemStatus == 0 ? "未出餐" : itemStatus == 1 ? "已出餐" : "已取消";
        //拼接富文本行：序号、菜名、状态、单价×份数、小计
        QListWidgetItem *row = new QListWidgetItem(
            QString("%1. %2  【%3】\n    ￥%4 × %5    小计 ￥%6")
                .arg(index++).arg(item["dishName"].toString()).arg(state)
                .arg(price, 0, 'f', 2).arg(count)
                // 已取消小计显示0，正常显示单价×份数
                .arg(itemStatus==2?0:price*count, 0, 'f', 2)
        );
        row->setForeground(QColor(
            itemStatus==0?"#e67e22":     // 橙色：未出餐
                itemStatus==1?"#27ae60": // 绿色：已出餐
                 "#95a5a6"));           // 灰色：已取消
        row->setSizeHint(QSize(0, 64));  // 灰色：已取消
        ui->orderList->addItem(row);
    }

    //更新底部总价与支付按钮文字、可用性
    ui->totalLabel->setText(QString("订单合计：￥%1").arg(currentMoney, 0, 'f', 2));
    ui->payButton->setText(currentPayStatus == 1 ? "订单已支付" : pendingCount>0 ? QString("等待出餐（%1项）").arg(pendingCount) : "进入支付");
    ui->payButton->setEnabled(currentPayStatus != 1 && pendingCount == 0);
}

OrderDetailWidget::~OrderDetailWidget()
{
    delete ui;
}

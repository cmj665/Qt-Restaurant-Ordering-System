#include "adminorderwidget.h"
#include "../network/networkmanager.h"
#include <QHeaderView>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>
AdminOrderWidget::AdminOrderWidget(QWidget*parent):QWidget(parent),network(new NetworkManager(this)),table(new QTableWidget(this)),summary(new QLabel(this)),timer(new QTimer(this)){
    auto*title=new QLabel("出餐管理",this);title->setStyleSheet("font-size:30px;font-weight:700;");auto*refreshButton=new QPushButton("刷新",this);auto*top=new QHBoxLayout;top->addWidget(title);top->addStretch();top->addWidget(refreshButton);
    table->setColumnCount(8);table->setHorizontalHeaderLabels({"桌台","订单号","菜品","数量","单价","状态","订单项ID","操作"});table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);table->setEditTriggers(QAbstractItemView::NoEditTriggers);table->verticalHeader()->setVisible(false);
    auto*layout=new QVBoxLayout(this);layout->setContentsMargins(28,24,28,24);layout->addLayout(top);layout->addWidget(summary);layout->addWidget(table,1);
    connect(refreshButton,&QPushButton::clicked,this,[this](){refresh();});
    connect(network,&NetworkManager::adminOrderItemsReceived,this,[this](const QJsonArray&items){table->setRowCount(items.size());int pending=0;for(int row=0;row<items.size();++row){QJsonObject item=items[row].toObject();int status=item["itemStatus"].toInt();if(status==0)++pending;QStringList values{item["tableName"].toString(),QString::number(item["orderId"].toInt()),item["dishName"].toString(),QString::number(item["count"].toInt()),QString::number(item["price"].toDouble(),'f',2),status==0?"未出餐":status==1?"已出餐":"已取消",QString::number(item["id"].toInt())};for(int col=0;col<values.size();++col)table->setItem(row,col,new QTableWidgetItem(values[col]));auto*actions=new QWidget(table);auto*box=new QHBoxLayout(actions);box->setContentsMargins(2,2,2,2);auto*serve=new QPushButton("出餐",actions);auto*cancel=new QPushButton("取消菜品",actions);serve->setEnabled(status==0);cancel->setEnabled(status==0);box->addWidget(serve);box->addWidget(cancel);int id=item["id"].toInt();connect(serve,&QPushButton::clicked,this,[this,id](){network->updateOrderItemStatus(id,1);});connect(cancel,&QPushButton::clicked,this,[this,id](){if(QMessageBox::question(this,"确认取消","取消后将退回库存并从订单金额中扣除，确定继续？")==QMessageBox::Yes)network->updateOrderItemStatus(id,2);});table->setCellWidget(row,7,actions);}summary->setText(QString("当前未支付订单项 %1 个 · 等待出餐 %2 个 · 每2秒自动同步").arg(items.size()).arg(pending));});
    connect(network,&NetworkManager::orderItemOperationFinished,this,[this](bool ok,const QString&message){if(!ok)QMessageBox::warning(this,"操作失败",message);refresh();});timer->setInterval(2000);connect(timer,&QTimer::timeout,this,[this](){refresh();});timer->start();refresh();
}
void AdminOrderWidget::refresh(){network->getAdminOrderItems();}

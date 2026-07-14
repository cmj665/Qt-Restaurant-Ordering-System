#include "admindishwidget.h"
#include "disheditdialog.h"
#include "../network/networkmanager.h"
#include <QColor>
#include <QDateTime>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

AdminDishWidget::AdminDishWidget(QWidget *parent)
    : QWidget(parent), network(new NetworkManager(this)),
      table(new QTableWidget(this)), statusLabel(new QLabel(this))
{
    auto *title=new QLabel("菜品管理",this);
    title->setStyleSheet("font-size:30px;font-weight:700;color:#263238;");
    auto *addButton=new QPushButton("新增菜品",this);
    auto *refreshButton=new QPushButton("刷新",this);
    auto *top=new QHBoxLayout;
    top->addWidget(title); top->addStretch(); top->addWidget(addButton); top->addWidget(refreshButton);

    table->setColumnCount(9);
    table->setHorizontalHeaderLabels({"ID","菜品名称","分类","价格","库存","销量","状态","描述","操作"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(7,QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);

    auto *layout=new QVBoxLayout(this);
    layout->setContentsMargins(28,24,28,24);
    layout->addLayout(top); layout->addWidget(table,1); layout->addWidget(statusLabel);
    connect(addButton,&QPushButton::clicked,this,&AdminDishWidget::addDish);
    connect(refreshButton,&QPushButton::clicked,this,&AdminDishWidget::refresh);
    connect(network,&NetworkManager::dishCategoriesReceived,this,[this](const QList<DishCategory>&items){categories=items;refresh();});

    connect(network,&NetworkManager::dishListReceived,this,[this](const QList<Dish>&dishes){
        table->setRowCount(dishes.size());
        int active=0, removed=0, low=0;
        QStringList lowNames;
        for(int row=0;row<dishes.size();++row){
            const Dish dish=dishes[row];
            const bool deleted=dish.isDeleted==1;
            if(deleted) ++removed;
            else { ++active; if(dish.stock<10){++low;lowNames<<QString("%1(%2)").arg(dish.name).arg(dish.stock);} }
            QString categoryName=QString::number(dish.catId);
            for(const auto &item:categories) if(item.id==dish.catId){categoryName=item.name;break;}
            QStringList values{QString::number(dish.id),dish.name,categoryName,
                QString::number(dish.price,'f',2),QString::number(dish.stock),
                QString::number(dish.soldCount),deleted?"已下架":"在售",dish.description};
            for(int col=0;col<values.size();++col) table->setItem(row,col,new QTableWidgetItem(values[col]));
            table->item(row,6)->setForeground(QColor(deleted?"#757575":"#2e7d32"));
            if(!deleted && dish.stock<10){
                table->item(row,4)->setForeground(QColor("#d32f2f"));
                table->item(row,4)->setText(dish.stock==0?"0（缺货）":QString::number(dish.stock)+"（预警）");
            }
            if(deleted) for(int col=0;col<8;++col) table->item(row,col)->setBackground(QColor("#eeeeee"));

            auto *actions=new QWidget(table); auto *box=new QHBoxLayout(actions); box->setContentsMargins(2,2,2,2);
            auto *edit=new QPushButton("修改",actions); auto *stockButton=new QPushButton("调库存",actions);
            auto *stateButton=new QPushButton(deleted?"上架":"下架",actions);
            edit->setEnabled(!deleted); stockButton->setEnabled(!deleted);
            box->addWidget(edit); box->addWidget(stockButton); box->addWidget(stateButton);
            connect(edit,&QPushButton::clicked,this,[this,dish](){editDish(dish);});
            connect(stockButton,&QPushButton::clicked,this,[this,dish](){adjustStock(dish);});
            connect(stateButton,&QPushButton::clicked,this,[this,dish,deleted](){
                if(deleted){
                    if(QMessageBox::question(this,"确认上架","确认重新上架“"+dish.name+"”？")==QMessageBox::Yes) network->restoreDish(dish.id);
                }else if(QMessageBox::question(this,"确认下架","确认下架“"+dish.name+"”？\n历史订单不会被删除。")==QMessageBox::Yes) network->deleteDish(dish.id);
            });
            table->setCellWidget(row,8,actions);
        }
        const QString warning=low>0?QString("库存预警 %1 项：%2").arg(low).arg(lowNames.join("、")):"库存正常";
        statusLabel->setStyleSheet(low>0?"color:#d32f2f;font-weight:bold;":"color:#2e7d32;");
        statusLabel->setText(QString("在售 %1 · 已下架 %2 · %3 · 最近刷新 %4")
            .arg(active).arg(removed).arg(warning).arg(QDateTime::currentDateTime().toString("HH:mm:ss")));
    });
    connect(network,&NetworkManager::dishOperationFinished,this,[this](bool success,const QString&message){
        if(!success) QMessageBox::warning(this,"操作失败",message);
        refresh();
    });
    network->getDishCategories();
}

void AdminDishWidget::refresh(){statusLabel->setText("正在读取菜品列表...");network->getDishList();}
void AdminDishWidget::addDish(){DishEditDialog dialog(categories,nullptr,this);if(dialog.exec()==QDialog::Accepted)network->addDish(dialog.value());}
void AdminDishWidget::editDish(const Dish&dish){DishEditDialog dialog(categories,&dish,this);if(dialog.exec()==QDialog::Accepted)network->updateDish(dialog.value());}
void AdminDishWidget::adjustStock(const Dish&dish){bool ok=false;int value=QInputDialog::getInt(this,"库存调整",dish.name+"的新库存数量：",dish.stock,0,999999,1,&ok);if(ok)network->adjustStock(dish.id,value);}

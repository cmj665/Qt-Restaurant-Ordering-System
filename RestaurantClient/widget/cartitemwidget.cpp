#include "cartitemwidget.h"
#include "ui_cartitemwidget.h"
#include "widget/ui_cartitemwidget.h"

//购物车中的单个商品控件（CartItemWidget）
//负责显示一条购物车商品，dish+count并把"+"、"-"按钮点击事件通知给外部
CartItemWidget::CartItemWidget(const Dish &dish,int count,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CartItemWidget)
    ,m_dish(dish)
    ,m_count(count)
{
    ui->setupUi(this);
    updateView();
    //加减按钮
    connect(ui->plusButton,&QPushButton::clicked,this,&CartItemWidget::onPlus);
    connect(ui->minusButton,&QPushButton::clicked,this,&CartItemWidget::onMinus);
}

CartItemWidget::~CartItemWidget()
{
    delete ui;
}

//刷新界面，数量更新
void CartItemWidget::updateView()
{
    ui->nameLabel->setText(m_dish.name);
    ui->countLabel->setText(QString::number(m_count));
}

void CartItemWidget::onPlus(){
    emit addOne(m_dish.id);
}

void CartItemWidget::onMinus()
{
    emit removeOne(m_dish.id);
}
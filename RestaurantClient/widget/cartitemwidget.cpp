#include "cartitemwidget.h"
#include "ui_cartitemwidget.h"
#include "widget/ui_cartitemwidget.h"

CartItemWidget::CartItemWidget(const Dish &dish,int count,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CartItemWidget)
    ,m_dish(dish)
    ,m_count(count)
{
    ui->setupUi(this);
    updateView();
    connect(ui->plusButton,&QPushButton::clicked,this,&CartItemWidget::onPlus);
    connect(ui->minusButton,&QPushButton::clicked,this,&CartItemWidget::onMinus);
}

CartItemWidget::~CartItemWidget()
{
    delete ui;
}

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
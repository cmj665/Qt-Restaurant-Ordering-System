#include "cartwidget.h"
#include "ui_cartwidget.h"

#include <QListWidgetItem>
#include <QMessageBox>
#include <QVariant>
#include <QDebug>

CartWidget::CartWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CartWidget)
{
    ui->setupUi(this);

    setWindowTitle("购物车");
        resize(420,600);

    // //购物车为空禁止查看订单
    // ui->detailButton->setEnabled(false);

    ui->titleLabel->setAlignment(Qt::AlignCenter);
    ui->totalLabel->setAlignment(Qt::AlignRight);

    ui->titleLabel->setStyleSheet(
        "font-size:22px;"
        "font-weight:bold;"
        "padding:8px;"
        );

    ui->submitButton->setMinimumHeight(45);

    //删除选中的菜品
    connect(ui->removeButton,&QPushButton::clicked,this,[this](){removeCurrentDish();});

    //清空购物车
    connect(ui->clearButton,&QPushButton::clicked,this,[this](){
        clearCart();
    });

    //提交订单
    connect(ui->submitButton,&QPushButton::clicked,this,[this](){
        if(m_cart.isEmpty())
        {
            QMessageBox::warning(this,"提示","购物车不能为空");
                return;
        }
        emit submitOrderRequested(cartItems());

    });

    //要去支付订单
    connect(ui->checkoutButton,&QPushButton::clicked,this,[this](){
        qDebug()<<"点击去结账";
        emit checkoutRequested();
    });

    //查看订单
    connect(ui->detailButton,&QPushButton::clicked,this,[this](){
        emit detailRequested();
    });



}

CartWidget::~CartWidget()
{
    delete ui;
}

void CartWidget::addDish(const Dish &dish)
{
    //已经有这道菜,数量加1
    if(m_cart.contains(dish.id))
    {
        m_cart[dish.id].count++;
    }
    else{
        CartItem item;
        item.dish = dish;
        item.count =1;
        m_cart.insert(dish.id,item);
    }
    refreshCart();
    // //已有订单，可以查看订单详情
    // ui->detailButton->setEnabled(true);
}

void CartWidget::refreshCart()
{
    ui->cartListWidget->clear();

    for(const CartItem &item :m_cart)
    {
        const double subtotal = item.dish.price * item.count;

        const QString text = QString("%1 x %2\n单价：￥%3  小计：￥%4")
                                 .arg(item.dish.name)
                                 .arg(item.count)
                                 .arg(item.dish.price,0,'f',2)
                                 .arg(subtotal,0,'f',2);

        QListWidgetItem *listItem = new QListWidgetItem(text);

        //把dishId存进列表项中，删除时shiyong
        listItem->setData(Qt::UserRole,item.dish.id);
        listItem->setSizeHint(QSize(0,65));
        ui->cartListWidget->addItem(listItem);

    }

    ui->totalLabel->setText(QString("合计：￥%1").arg(totalPrice(),0,'f',2));

}

QList<CartItem> CartWidget::cartItems() const
{
    QList<CartItem> result;
    for(const CartItem &item : m_cart)
    {
        result.append(item);
    }
    return result;
}

double CartWidget::totalPrice() const
{
    double total=0.0;
    for(const CartItem &item : m_cart)
    {
        total+=item.dish.price*item.count;
    }
    return total;
}

void CartWidget::removeCurrentDish()
{
    QListWidgetItem *currentItem = ui->cartListWidget->currentItem();
    if(currentItem == nullptr)
    {
        QMessageBox::information(this,"提示","请先选择要删除的菜品");
        return ;
    }

    const int dishId = currentItem->data(Qt::UserRole).toInt();

    m_cart.remove(dishId);

    refreshCart();
}

void CartWidget::clearCart()
{
    m_cart.clear();
    refreshCart();
    // //清空后没有订单，禁止查看订单
    // ui->detailButton->setEnabled(false);
}





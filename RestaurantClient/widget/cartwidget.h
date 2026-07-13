#ifndef CARTWIDGET_H
#define CARTWIDGET_H

#include <QWidget>
#include <QMap>
#include <QList>

#include "../model/cartitem.h"
#include "../model/dish.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class CartWidget;
}
QT_END_NAMESPACE

class CartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CartWidget(QWidget *parent = nullptr);
    ~CartWidget();


    //添加一道菜
    void addDish(const Dish &dish);

    //获取全部购物车内容
    QList<CartItem> cartItems() const;

    //计算总价
    double totalPrice() const;

    //清空购物车
    void clearCart();

    // //删除指定菜品
    // void removeDish(int dishId);

signals:
    //点击提交订单后，把购物车内容发出去
    void submitOrderRequested(const QList<CartItem> &items);

    //支付的信号
    void checkoutRequested();

    //订单详情的信号
    void detailRequested();


private:
    Ui::CartWidget *ui;

    //key:dish.id;
    //value:购物车项
    QMap<int,CartItem> m_cart;

    //刷新界面
    void refreshCart();

    //删除当前选中的菜品
    void removeCurrentDish();


};

#endif // CARTWIDGET_H

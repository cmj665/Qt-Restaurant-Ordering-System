#ifndef CARTITEMWIDGET_H
#define CARTITEMWIDGET_H

#include <QWidget>

#include "../model/dish.h"

namespace Ui {
class CartItemWidget;
}

class CartItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CartItemWidget(
        const Dish &dish,
        int count,
        QWidget *parent = nullptr);
    ~CartItemWidget();

signals:
    //增加数量
    void  addOne(int dishId);

    //减少数量
    void removeOne(int dishId);

private slots:
    void onPlus();
    void onMinus();

private:
    Ui::CartItemWidget *ui;
    Dish m_dish;
    int m_count;
    void updateView();
};

#endif // CARTITEMWIDGET_H

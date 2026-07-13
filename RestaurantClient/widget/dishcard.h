#ifndef DISHCARD_H
#define DISHCARD_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPixmap>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "../model/Dish.h"

namespace Ui {
class DishCard;
}

class DishCard : public QWidget
{
    Q_OBJECT

public:
    explicit DishCard(
        const Dish &dish,
        QWidget *parent = nullptr);
    ~DishCard();

signals:
    //点击加入购物车发送
    void addDish(Dish &dish);

private:
    Ui::DishCard *ui;
    Dish m_dish;

    QLabel *imageLabel;

    QLabel *nameLabel;

    QLabel *priceLabel;

    QLabel *stockLabel;

    QPushButton *addButton;

    //请求图片
    QNetworkAccessManager *manager;

};

#endif // DISHCARD_H

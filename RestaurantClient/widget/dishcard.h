#ifndef DISHCARD_H
#define DISHCARD_H

#include <QLabel>
#include <QNetworkAccessManager>
#include <QPushButton>
#include <QWidget>
#include "../model/Dish.h"

namespace Ui { class DishCard; }
class QResizeEvent;

class DishCard : public QWidget
{
    Q_OBJECT

public:
    explicit DishCard(const Dish &dish, QWidget *parent = nullptr, bool recommended = false);
    ~DishCard();
    void setQuantity(int quantity);
    void setDarkMode(bool enabled);

signals:
    void increaseDish(const Dish &dish);
    void decreaseDish(int dishId);

private:
    Ui::DishCard *ui;
    Dish m_dish;
    int currentQuantity = 0;
    QLabel *imageLabel;
    QLabel *nameLabel;
    QLabel *priceLabel;
    QLabel *stockLabel;
    QLabel *soldLabel;
    QLabel *quantityLabel;
    QPushButton *minusButton;
    QPushButton *plusButton;
    QNetworkAccessManager *manager;
};

#endif // DISHCARD_H

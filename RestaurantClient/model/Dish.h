#ifndef DISH_H
#define DISH_H

#include <QString>

class Dish{

public:
    int id;
    int catId;
    QString name;
    double price;
    int stock;
    QString picture;
    QString description;

};

#endif // DISH_H

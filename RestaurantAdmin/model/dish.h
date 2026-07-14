#ifndef ADMIN_DISH_H
#define ADMIN_DISH_H
#include <QString>
struct Dish { int id=0; int catId=0; QString name; double price=0; int stock=0; QString picture; QString description; int isDeleted=0; int soldCount=0; };
struct DishCategory { int id=0; QString name; };
#endif

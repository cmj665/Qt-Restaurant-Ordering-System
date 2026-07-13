#ifndef CARTITEM_H
#define CARTITEM_H

#include "Dish.h"

struct CartItem
{
    Dish dish;
    int count=1;
};

#endif // CARTITEM_H

#ifndef ADMIN_DISH_H
#define ADMIN_DISH_H
#include <QString>
// 菜品数据模型：保存菜品基础信息、上下架状态和累计销量。
struct Dish {
    int id=0;
    int catId=0;
    QString name;
    double price=0;
    int stock=0;
    QString picture;
    QString description;
    int isDeleted=0; int soldCount=0;
};
// 菜品分类模型：用于分类下拉框和分类名称展示。
struct DishCategory {
    int id=0;
    QString name;
};
#endif

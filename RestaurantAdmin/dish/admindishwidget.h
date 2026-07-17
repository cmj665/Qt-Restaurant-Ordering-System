#ifndef ADMINDISHWIDGET_H
#define ADMINDISHWIDGET_H
#include <QWidget>
#include "../model/dish.h"
class QLabel; class QTableWidget; class NetworkManager; class QNetworkAccessManager;
/**
 * @brief 菜品档案及库存维护页面。
 *
 * 集中处理新增、编辑、上下架与库存调整；分类和菜品数据均以后端返回结果为准。
 */
class AdminDishWidget:public QWidget {
public: explicit AdminDishWidget(QWidget *parent=nullptr);
private: void refresh(); void addDish(); void editDish(const Dish &dish); void adjustStock(const Dish &dish); NetworkManager *network; QNetworkAccessManager *imageManager; QTableWidget *table; QLabel *statusLabel; QList<DishCategory> categories; };
#endif

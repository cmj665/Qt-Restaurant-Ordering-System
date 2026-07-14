#ifndef ADMINDISHWIDGET_H
#define ADMINDISHWIDGET_H
#include <QWidget>
#include "../model/dish.h"
class QLabel; class QTableWidget; class NetworkManager; class QNetworkAccessManager;
class AdminDishWidget:public QWidget {
public: explicit AdminDishWidget(QWidget *parent=nullptr);
private: void refresh(); void addDish(); void editDish(const Dish &dish); void adjustStock(const Dish &dish); NetworkManager *network; QNetworkAccessManager *imageManager; QTableWidget *table; QLabel *statusLabel; QList<DishCategory> categories; };
#endif

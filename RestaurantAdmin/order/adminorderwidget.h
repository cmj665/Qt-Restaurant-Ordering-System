#ifndef ADMINORDERWIDGET_H
#define ADMINORDERWIDGET_H
#include <QWidget>
class NetworkManager; class QTableWidget; class QLabel; class QTimer;
class AdminOrderWidget:public QWidget {
public: explicit AdminOrderWidget(QWidget *parent=nullptr);
private: void refresh(); NetworkManager *network; QTableWidget *table; QLabel *summary; QTimer *timer;
};
#endif

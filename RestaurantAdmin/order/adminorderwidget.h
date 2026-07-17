#ifndef ADMINORDERWIDGET_H
#define ADMINORDERWIDGET_H
#include <QWidget>
class NetworkManager; class QTableWidget; class QLabel; class QTimer;
/**
 * @brief 后厨待处理菜品管理页面。
 *
 * 定时展示尚未完成的订单项，并把出餐或取消操作同步到服务端。
 */
class AdminOrderWidget:public QWidget {
public: explicit AdminOrderWidget(QWidget *parent=nullptr);
private: void refresh(); NetworkManager *network; QTableWidget *table; QLabel *summary; QTimer *timer;
};
#endif

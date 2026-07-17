#ifndef ADMINTABLEWIDGET_H
#define ADMINTABLEWIDGET_H

#include <QWidget>
#include "../model/diningtable.h"

class QLabel;
class QTableWidget;
class QTimer;
class NetworkManager;

/**
 * @brief 管理端桌台监控页面。
 *
 * 周期性拉取桌台状态、汇总使用情况，并提供清台操作。刷新请求失败时保留现有
 * 表格内容，以免短暂网络故障造成界面数据闪烁。
 */
class AdminTableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AdminTableWidget(const QString &username, QWidget *parent = nullptr);

signals:
    void logoutRequested();

private:
    void refresh();
    void showTables(const QList<DiningTable> &tables);
    NetworkManager *network;
    QTableWidget *table;
    QLabel *summaryLabel;
    QLabel *statusLabel;
    QTimer *timer;
};

#endif

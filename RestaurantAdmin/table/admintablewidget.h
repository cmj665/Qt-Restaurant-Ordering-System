#ifndef ADMINTABLEWIDGET_H
#define ADMINTABLEWIDGET_H

#include <QWidget>
#include "../model/diningtable.h"

class QLabel;
class QTableWidget;
class QTimer;
class NetworkManager;

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

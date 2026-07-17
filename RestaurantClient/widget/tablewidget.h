#ifndef TABLEWIDGET_H
#define TABLEWIDGET_H

#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHash>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QWidget>
#include "../network/networkmanager.h"

namespace Ui { class TableWidget; }

/**
 * @brief 桌台选择页。
 *
 * 定时刷新桌台占用状态，并按服务端返回的桌台编号更新对应卡片；只有可用桌台
 * 才会发出 tableSelected 信号进入点餐流程。
 */
class TableWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(TableWidget)

public:
    explicit TableWidget(QWidget *parent = nullptr);
    ~TableWidget();

signals:
    void tableSelected(int tableId);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void showTables(QList<DiningTable> tables);
    void refreshTables();

private:
    Ui::TableWidget *ui;
    NetworkManager *network;
    QGraphicsView *graphicsView;
    QGraphicsScene *scene;
    QHash<int, QGraphicsProxyWidget *> tableItems;
    QHash<int, QPushButton *> tableButtons;
    QWidget *legendWidget;
    QPushButton *refreshButton;
    QLabel *refreshStatusLabel;
    QTimer *refreshTimer;

    QLabel *createColorLabel(const QString &color, const QString &text);
    void createLegend();
    void updateTableItem(const DiningTable &table);
    void arrangeTableItems();
};

#endif // TABLEWIDGET_H

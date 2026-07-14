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

class TableWidget : public QWidget
{
    Q_OBJECT

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

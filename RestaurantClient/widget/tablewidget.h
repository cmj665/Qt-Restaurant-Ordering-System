#ifndef TABLEWIDGET_H
#define TABLEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include "../network/networkmanager.h"
#include <QGridLayout>
#include <QLabel>
#include <QHBoxLayout>

namespace Ui {
class TableWidget;
}

class TableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableWidget(QWidget *parent = nullptr);
    ~TableWidget();



signals:
    //发送选择的桌号
    void tableSelected(int tableId);

private:
    Ui::TableWidget *ui;
    // void createTables();
    NetworkManager *network;
    QGridLayout *layout;
    QWidget *legendWidget;

private:
    QLabel *createColorLabel(QString color, QString text);
    //什么颜色表示什么桌台状态
    void createLegend();

private slots:
    void showTables(QList<DiningTable> tables);

};

#endif // TABLEWIDGET_H

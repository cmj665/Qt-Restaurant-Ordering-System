#ifndef DISHWIDGET_H
#define DISHWIDGET_H

#include <QWidget>
#include "../model/Dish.h"
#include "../network/networkmanager.h"
#include <QLabel>
#include <QScrollArea>
#include <QWidget>
#include <QScrollArea>
#include <QGridLayout>
#include "cartwidget.h"
#include <QPushButton>
#include <QPointer>
#include <QHBoxLayout>
#include <QPropertyAnimation>

class QResizeEvent;

class PayWidget;
class OrderDetailWidget;

namespace Ui {
class DishWidget;
}

class DishWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DishWidget(int tableId,QWidget *parent = nullptr);
    ~DishWidget();

public slots:
    void changeTable(int tableId);

signals:
    void backToTable();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::DishWidget *ui;

    NetworkManager *network;

    QScrollArea *scrollArea;
    QWidget *container;
    QGridLayout *layout;
    QVBoxLayout *categoryLayout;
    QWidget *categoryWidget;

    CartWidget *cartWidget;
    QPushButton *cartBackdrop = nullptr;
    QPropertyAnimation *cartAnimation = nullptr;
    bool cartDrawerOpen = false;
    QWidget *cartSummaryBar = nullptr;
    QLabel *cartTotalLabel = nullptr;
    QLabel *cartCountLabel = nullptr;
    QPushButton *cartConfirmButton = nullptr;

    QList<Dish> dishes;
    int selectedCategory = 0;

    //当前桌号
    int currentTableId;
    QLabel *titleLabel;
    QPushButton *changeTableButton;
    QPushButton *backTableButton;

    int pendingOrderId;
    double pendingMoney;
    bool paymentOpening = false;
    bool checkoutInProgress = false;
    QPointer<PayWidget> activePayWidget;
    QPointer<OrderDetailWidget> activeDetailWidget;

    void openPaymentWindow();
    void printReceipt(int orderId, double money, int payType);
    void renderDishes();
    void openCartDrawer();
    void closeCartDrawer();
    QRect cartDrawerGeometry(bool opened) const;


};

#endif // DISHWIDGET_H

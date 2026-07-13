#ifndef PAYWIDGET_H
#define PAYWIDGET_H

#include <QWidget>
#include "../network/networkmanager.h"

namespace Ui {
class PayWidget;
}

class PayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PayWidget(int orderId,double money,QWidget *parent = nullptr);
    ~PayWidget();

signals:
    void paySuccess();


private:
    Ui::PayWidget *ui;

    NetworkManager *network;
    int currentOrderId;
    bool paying;
};

#endif // PAYWIDGET_H

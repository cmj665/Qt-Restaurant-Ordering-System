#ifndef PAYWIDGET_H
#define PAYWIDGET_H

#include <QPixmap>
#include <QWidget>
#include "../network/networkmanager.h"

class QLabel;
class QPushButton;

namespace Ui {
class PayWidget;
}

class PayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PayWidget(int orderId, double money, QWidget *parent = nullptr);
    ~PayWidget();

signals:
    void paySuccess(int payType);

private:
    Ui::PayWidget *ui;
    NetworkManager *network;
    int currentOrderId;
    double currentMoney;
    int selectedPayType;
    bool paying;
    QLabel *channelLabel;
    QLabel *qrLabel;
    QLabel *statusLabel;
    QPushButton *confirmButton;
    QPushButton *backButton;

    void selectPaymentChannel(int payType);
    QPixmap createMockQrCode(const QString &content) const;
};

#endif // PAYWIDGET_H

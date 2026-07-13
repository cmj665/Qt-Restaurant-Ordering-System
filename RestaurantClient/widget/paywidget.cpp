#include "paywidget.h"
#include "ui_paywidget.h"

#include <QMessageBox>
#include <QLayout>

PayWidget::PayWidget(int orderId,double money,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PayWidget)
    ,currentOrderId(orderId)
    ,paying(false)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window);

    resize(500,300);

    network = new NetworkManager(this);

    ui->titleLabel->setText("订单支付");

    ui->titleLabel->setAlignment(Qt::AlignCenter);

    ui->titleLabel->setStyleSheet(
        "font-size:32px;"
        "font-weight:bold;"
        );

    ui->moneyLabel->setText(
        QString("应付金额：￥%1")
            .arg(money,0,'f',2)
        );


    ui->moneyLabel->setAlignment(
        Qt::AlignCenter
        );


    ui->moneyLabel->setStyleSheet(
        "font-size:24px;"
        "color:#e67e22;"
        "font-weight:bold;"
        );

    //按钮美化
    ui->wechatButton->setMinimumHeight(100);
    ui->alipayButton->setMinimumHeight(100);


    ui->wechatButton->setStyleSheet(
        "QPushButton{"
        "font-size:20px;"
        "background:#07c160;"
        "color:white;"
        "border-radius:10px;"
        "}"
        );


    ui->alipayButton->setStyleSheet(
        "QPushButton{"
        "font-size:20px;"
        "background:#1677ff;"
        "color:white;"
        "border-radius:10px;"
        "}"
        );

    //增加布局
    QVBoxLayout *layout =
        qobject_cast<QVBoxLayout*>(this->layout());

    if(layout)
    {
        layout->setSpacing(25);
        layout->setContentsMargins(
            50,30,50,30
            );
    }

    //微信支付
    connect(ui->wechatButton,&QPushButton::clicked,this,[this](){
        if(paying)
        {
            return;
        }

        paying=true;
        ui->wechatButton->setEnabled(false);
        ui->alipayButton->setEnabled(false);
        network->pay(currentOrderId,1);

        // network->pay(currentOrderId,1);

        // emit paySuccess();

    });

    //支付宝支付
    connect(ui->alipayButton,&QPushButton::clicked,this,[this](){
        if(paying)
        {
            return;
        }

        paying=true;
        ui->wechatButton->setEnabled(false);
        ui->alipayButton->setEnabled(false);
        network->pay(currentOrderId,2);
    });

    connect(network,&NetworkManager::payFinished,
            this,
            [this](bool success,QString msg){

        paying=false;

        if(success)
            {
            QMessageBox::information(this,"成功","支付成功");
            emit paySuccess();

            close();
        }
        else
            {
            ui->wechatButton->setEnabled(true);
            ui->alipayButton->setEnabled(true);

            QMessageBox::warning(this,"失败",msg);
        }
    });

}

PayWidget::~PayWidget()
{
    delete ui;
}

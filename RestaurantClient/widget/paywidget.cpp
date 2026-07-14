#include "paywidget.h"
#include "ui_paywidget.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>

PayWidget::PayWidget(int orderId, double money, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PayWidget)
    , network(new NetworkManager(this))
    , currentOrderId(orderId)
    , currentMoney(money)
    , selectedPayType(0)
    , paying(false)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window);
    setWindowTitle("订单支付");
    resize(520, 680);
    ui->verticalLayout->parentWidget()->setGeometry(20, 10, 480, 650);

    ui->titleLabel->setText("订单支付");
    ui->titleLabel->setAlignment(Qt::AlignCenter);
    ui->titleLabel->setStyleSheet("font-size:30px;font-weight:bold;");

    ui->moneyLabel->setText(QString("应付金额：￥%1").arg(money, 0, 'f', 2));
    ui->moneyLabel->setAlignment(Qt::AlignCenter);
    ui->moneyLabel->setStyleSheet("font-size:24px;color:#e67e22;font-weight:bold;");

    ui->wechatButton->setText("微信支付");
    ui->alipayButton->setText("支付宝支付");
    ui->wechatButton->setMinimumHeight(72);
    ui->alipayButton->setMinimumHeight(72);
    ui->wechatButton->setStyleSheet(
        "QPushButton{font-size:20px;background:#07c160;color:white;border-radius:10px;}"
    );
    ui->alipayButton->setStyleSheet(
        "QPushButton{font-size:20px;background:#1677ff;color:white;border-radius:10px;}"
    );

    channelLabel = new QLabel(this);
    channelLabel->setAlignment(Qt::AlignCenter);
    channelLabel->hide();

    qrLabel = new QLabel(this);
    qrLabel->setAlignment(Qt::AlignCenter);
    qrLabel->setMinimumSize(300, 300);
    qrLabel->hide();

    statusLabel = new QLabel("请选择支付方式", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("font-size:16px;color:#666;");

    confirmButton = new QPushButton("模拟扫码并完成支付", this);
    confirmButton->setMinimumHeight(52);
    confirmButton->setStyleSheet(
        "QPushButton{font-size:18px;background:#ff8a00;color:white;border-radius:8px;}"
        "QPushButton:disabled{background:#aaa;}"
    );
    confirmButton->hide();

    backButton = new QPushButton("返回选择支付方式", this);
    backButton->setMinimumHeight(42);
    backButton->hide();

    ui->verticalLayout->insertWidget(3, channelLabel);
    ui->verticalLayout->insertWidget(4, qrLabel);
    ui->verticalLayout->insertWidget(5, statusLabel);
    ui->verticalLayout->insertWidget(6, confirmButton);
    ui->verticalLayout->insertWidget(7, backButton);
    ui->verticalLayout->setSpacing(14);
    ui->verticalLayout->setContentsMargins(35, 20, 35, 20);

    connect(ui->wechatButton, &QPushButton::clicked, this, [this](){
        selectPaymentChannel(1);
    });
    connect(ui->alipayButton, &QPushButton::clicked, this, [this](){
        selectPaymentChannel(2);
    });

    connect(confirmButton, &QPushButton::clicked, this, [this](){
        if(paying || selectedPayType == 0)
            return;

        paying = true;
        confirmButton->setEnabled(false);
        backButton->setEnabled(false);
        statusLabel->setText("正在向模拟支付平台确认支付结果...");
        network->pay(currentOrderId, selectedPayType);
    });

    connect(backButton, &QPushButton::clicked, this, [this](){
        if(paying)
            return;

        selectedPayType = 0;
        channelLabel->hide();
        qrLabel->hide();
        confirmButton->hide();
        backButton->hide();
        ui->wechatButton->show();
        ui->alipayButton->show();
        statusLabel->setText("请选择支付方式");
    });

    connect(network, &NetworkManager::payFinished, this,
            [this](bool success, const QString &message){
        paying = false;
        if(success)
        {
            statusLabel->setText("支付成功");
            emit paySuccess(selectedPayType);
            close();
            return;
        }

        confirmButton->setEnabled(true);
        backButton->setEnabled(true);
        statusLabel->setText("支付失败，可重新模拟扫码");
        QMessageBox::warning(this, "失败", message);
    });
}

void PayWidget::selectPaymentChannel(int payType)
{
    if(paying)
        return;

    selectedPayType = payType;
    const bool wechat = payType == 1;
    const QString channel = wechat ? "微信支付" : "支付宝";
    const QString scheme = wechat ? "weixin" : "alipay";
    const QString content = QString("mockpay://%1/order/%2?amount=%3&nonce=%4")
                                .arg(scheme)
                                .arg(currentOrderId)
                                .arg(currentMoney, 0, 'f', 2)
                                .arg(QDateTime::currentMSecsSinceEpoch());

    channelLabel->setText(channel + " · 模拟收银台");
    channelLabel->setStyleSheet(QString("font-size:22px;font-weight:bold;color:%1;")
                                    .arg(wechat ? "#07c160" : "#1677ff"));
    qrLabel->setPixmap(createMockQrCode(content));
    statusLabel->setText(QString("请使用%1扫描二维码").arg(channel));

    ui->wechatButton->hide();
    ui->alipayButton->hide();
    channelLabel->show();
    qrLabel->show();
    confirmButton->show();
    confirmButton->setEnabled(true);
    backButton->show();
    backButton->setEnabled(true);
}

QPixmap PayWidget::createMockQrCode(const QString &content) const
{
    constexpr int modules = 29;
    constexpr int scale = 9;
    constexpr int quiet = 4;
    const int imageSize = (modules + quiet * 2) * scale;
    QPixmap pixmap(imageSize, imageSize);
    pixmap.fill(Qt::white);

    QPainter painter(&pixmap);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);

    auto drawModule = [&](int x, int y) {
        painter.drawRect((x + quiet) * scale, (y + quiet) * scale, scale, scale);
    };
    auto inFinder = [](int x, int y, int ox, int oy) {
        return x >= ox && x < ox + 7 && y >= oy && y < oy + 7;
    };
    auto drawFinder = [&](int ox, int oy) {
        for(int y = 0; y < 7; ++y)
            for(int x = 0; x < 7; ++x)
                if(x == 0 || x == 6 || y == 0 || y == 6 ||
                   (x >= 2 && x <= 4 && y >= 2 && y <= 4))
                    drawModule(ox + x, oy + y);
    };

    const QByteArray hash = QCryptographicHash::hash(content.toUtf8(), QCryptographicHash::Sha256);
    int bit = 0;
    for(int y = 0; y < modules; ++y)
    {
        for(int x = 0; x < modules; ++x)
        {
            if(inFinder(x, y, 0, 0) || inFinder(x, y, modules - 7, 0) ||
               inFinder(x, y, 0, modules - 7))
                continue;

            const unsigned char byte = static_cast<unsigned char>(hash.at((bit / 8) % hash.size()));
            if(((byte >> (bit % 8)) & 1) ^ ((x + y) % 3 == 0))
                drawModule(x, y);
            ++bit;
        }
    }

    drawFinder(0, 0);
    drawFinder(modules - 7, 0);
    drawFinder(0, modules - 7);
    return pixmap;
}

PayWidget::~PayWidget()
{
    delete ui;
}

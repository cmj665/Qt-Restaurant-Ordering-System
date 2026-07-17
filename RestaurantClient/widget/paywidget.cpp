#include "paywidget.h"
#include "ui_paywidget.h"

#include <QCryptographicHash>  //哈希加密，用来生成二维码黑白点阵随机数据
#include <QDateTime>    //获取时间戳，生成支付链接唯一随机串
#include <QLabel>
#include <QMessageBox>
#include <QPainter>   //绘图工具，手动绘制模拟二维码
#include <QPushButton>

PayWidget::PayWidget(int orderId, double money, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PayWidget)
    , network(new NetworkManager(this))   //负责发起支付接口请求
    , currentOrderId(orderId)
    , currentMoney(money)
    , selectedPayType(0)   //0 未选支付方式，1 微信，2 支付宝
    , paying(false)        //支付中标记，防止重复点击提交
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window);   //独立弹窗，不是内嵌子部件
    setWindowTitle("订单支付");
    resize(520, 680);
    ui->verticalLayout->parentWidget()->setGeometry(20, 10, 480, 650);

    //静态文字控件样式（UI 拖拽生成的控件）

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
    // 微信绿色、支付宝蓝色样式
    ui->wechatButton->setStyleSheet(
        "QPushButton{font-size:20px;background:#07c160;color:white;border-radius:10px;}"
    );
    ui->alipayButton->setStyleSheet(
        "QPushButton{font-size:20px;background:#1677ff;color:white;border-radius:10px;}"
    );

    //5 个运行时动态创建的控件，刚打开窗口全部隐藏，选完支付方式才显示。
    channelLabel = new QLabel(this);  // 支付渠道标题（微信/支付宝）
    channelLabel->setAlignment(Qt::AlignCenter);
    channelLabel->hide();  // 默认隐藏

    qrLabel = new QLabel(this);  //二维码图片显示框
    qrLabel->setAlignment(Qt::AlignCenter);
    qrLabel->setMinimumSize(300, 300);
    qrLabel->hide();  // 默认隐藏

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

    //把控件插入垂直布局
    ui->verticalLayout->insertWidget(3, channelLabel);
    ui->verticalLayout->insertWidget(4, qrLabel);
    ui->verticalLayout->insertWidget(5, statusLabel);
    ui->verticalLayout->insertWidget(6, confirmButton);
    ui->verticalLayout->insertWidget(7, backButton);
    ui->verticalLayout->setSpacing(14);
    ui->verticalLayout->setContentsMargins(35, 20, 35, 20);

    //点击微信传入 1，支付宝传入 2，调用selectPaymentChannel切换支付界面
    connect(ui->wechatButton, &QPushButton::clicked, this, [this](){
        selectPaymentChannel(1);
    });
    connect(ui->alipayButton, &QPushButton::clicked, this, [this](){
        selectPaymentChannel(2);
    });

    connect(confirmButton, &QPushButton::clicked, this, [this](){
        //正在请求支付中(防止重复多点提交)或者没有选择支付方式
        if(paying || selectedPayType == 0)
            return;

        paying = true;
        confirmButton->setEnabled(false);
        backButton->setEnabled(false);
        statusLabel->setText("正在向模拟支付平台确认支付结果...");
        //调用网络类发起支付请求
        network->pay(currentOrderId, selectedPayType);
    });

    connect(backButton, &QPushButton::clicked, this, [this](){
        if(paying)
            return;

        //未支付时，切回初始选择支付方式页面，清空选中类型。
        selectedPayType = 0;
        // 隐藏二维码、确认按钮
        channelLabel->hide();
        qrLabel->hide();
        confirmButton->hide();
        backButton->hide();
        // 重新显示微信支付宝选择按钮
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

        //支付失败：解锁按钮，弹窗提示错误信息
        confirmButton->setEnabled(true);
        backButton->setEnabled(true);
        statusLabel->setText("支付失败，可重新模拟扫码");
        QMessageBox::warning(this, "失败", message);
    });
}


//点击微信 / 支付宝按钮后执行，切换到二维码收银台页面
void PayWidget::selectPaymentChannel(int payType)
{
    //正在支付则直接返回，禁止切换
    if(paying)
        return;

    selectedPayType = payType;
    const bool wechat = payType == 1;
    const QString channel = wechat ? "微信支付" : "支付宝";
    const QString scheme = wechat ? "weixin" : "alipay";
    //拼接模拟支付链接：携带支付渠道、订单 ID、金额、毫秒时间戳（保证链接唯一）
    //scheme 用来拼接二维码链接里的支付渠道标识，生成 mockpay://weixin/order/xxx 这类支付地址，给模拟二维码使用。
    const QString content = QString("mockpay://%1/order/%2?amount=%3&nonce=%4")
                                .arg(scheme)
                                .arg(currentOrderId)
                                .arg(currentMoney, 0, 'f', 2)
                                .arg(QDateTime::currentMSecsSinceEpoch());

    channelLabel->setText(channel + " · 模拟收银台");
    channelLabel->setStyleSheet(QString("font-size:22px;font-weight:bold;color:%1;")
                                    .arg(wechat ? "#07c160" : "#1677ff"));

    //调用createMockQrCode生成二维码图片赋值给 qrLabel
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


//手动绘制模拟二维码
QPixmap PayWidget::createMockQrCode(const QString &content) const
{
    //常量定义：29 模块二维码、缩放倍数、留白边距，计算画布总尺寸
    constexpr int modules = 29;  // 二维码网格：29行29列小方块
    constexpr int scale = 9;     // 每个小方块占9像素，放大变清晰
    constexpr int quiet = 4;     // 二维码四周留白（安静区，标准二维码必须留白）
    const int imageSize = (modules + quiet * 2) * scale;
    QPixmap pixmap(imageSize, imageSize);
    pixmap.fill(Qt::white);      // 整张画布底色白色

    QPainter painter(&pixmap);
    painter.setPen(Qt::NoPen);      // 不需要边框线
    painter.setBrush(Qt::black);    // 填充色黑色，用来画黑色方块

    //drawModule：绘制单个黑白方块
    auto drawModule = [&](int x, int y) {
        painter.drawRect((x + quiet) * scale, (y + quiet) * scale, scale, scale);
    };
    //inFinder：判断坐标是否是二维码三个角落定位框
    auto inFinder = [](int x, int y, int ox, int oy) {
        return x >= ox && x < ox + 7 && y >= oy && y < oy + 7;
    };      //定位框固定是 7×7 大小方块，ox/oy 是定位框左上角网格坐标；

    //drawFinder：绘制二维码四角标志性大方块
    auto drawFinder = [&](int ox, int oy) {
        for(int y = 0; y < 7; ++y)
            for(int x = 0; x < 7; ++x)
                if(x == 0 || x == 6 || y == 0 || y == 6 ||
                   (x >= 2 && x <= 4 && y >= 2 && y <= 4))
                    drawModule(ox + x, oy + y);
    };

    //对传入支付链接做 SHA256 哈希，用哈希值的二进制比特随机填充二维码点阵
    const QByteArray hash = QCryptographicHash::hash(content.toUtf8(), QCryptographicHash::Sha256);
    int bit = 0;
    for(int y = 0; y < modules; ++y)
    {
        for(int x = 0; x < modules; ++x)
        {
            // 跳过三个角落定位框区域，这里先不填充随机格子
            if(inFinder(x, y, 0, 0) || inFinder(x, y, modules - 7, 0) ||
               inFinder(x, y, 0, modules - 7))
                continue;

            // 从sha256哈希字节中取出1个字节
            const unsigned char byte = static_cast<unsigned char>(hash.at((bit / 8) % hash.size()));
             // 取出当前bit位的值（0或1）（前一个括号）    +  异或运算：随机微调，避免图案太规整
            if(((byte >> (bit % 8)) & 1) ^ ((x + y) % 3 == 0))
                drawModule(x, y);
            ++bit;
        }
    }

    //最后绘制三个角落定位块
    drawFinder(0, 0);               //左上角定位框
    drawFinder(modules - 7, 0);     //右上角定位框
    drawFinder(0, modules - 7);     //左下角定位框
    return pixmap;
}

PayWidget::~PayWidget()
{
    delete ui;
}

#include "dishcard.h"
#include "ui_dishcard.h"

#include <QHBoxLayout>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVBoxLayout>
#include <QPainterPath>
#include <QPainter>
#include <QPixmapCache>
#include <QEnterEvent>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>


DishCard::DishCard(const Dish &dish, QWidget *parent, bool recommended)
    : QWidget(parent)
    , ui(new Ui::DishCard)
    , m_dish(dish)
    , manager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);

    //-------------UI---------------------------
    setAttribute(Qt::WA_StyledBackground,false);
    setAttribute(Qt::WA_TranslucentBackground,true);
    setFixedSize(300, 388);
    setMouseTracking(true); //开启鼠标追踪，捕获移入移出事件

    //-----卡片阴影 + 悬浮动画配置--------
    cardShadow = new QGraphicsDropShadowEffect(this);
    cardShadow->setBlurRadius(14);  //默认阴影模糊14
    cardShadow->setOffset(0, 4);    // 默认阴影向下偏移4px
    cardShadow->setColor(QColor(0, 0, 0, 45));// 半透明黑色阴影
    setGraphicsEffect(cardShadow);

    //两套动画：控制模糊半径、阴影偏移
    shadowBlurAnimation = new QPropertyAnimation(cardShadow, "blurRadius", this);
    shadowOffsetAnimation = new QPropertyAnimation(cardShadow, "offset", this);
    for(QPropertyAnimation *animation : {shadowBlurAnimation, shadowOffsetAnimation})
    {
        animation->setDuration(160);
        animation->setEasingCurve(QEasingCurve::OutCubic); //平滑减速回弹曲线
    }

    // 创建顶部图片框
    imageLabel = new QLabel(this);
    imageLabel->setFixedSize(298, 190);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("background:#f0ebe4;border-radius:16px;color:#999999;font-size:13px;");
    //文本标签赋值（菜品信息渲染）
    nameLabel = ui->nameLabel;
    priceLabel = ui->priceLabel;
    stockLabel = ui->stockLabel;
    ui->addButton->hide();

    nameLabel->setText(dish.name);
    nameLabel->setWordWrap(true);// 菜名过长自动换行
    nameLabel->setMinimumHeight(42);
    nameLabel->setStyleSheet("font-size:19px;font-weight:800;color:#1f2937;border:none;background:transparent;");

    priceLabel->setText(QString("￥%1").arg(dish.price, 0, 'f', 2));
    priceLabel->setStyleSheet("font-size:24px;font-weight:900;color:#f97316;border:none;background:transparent;");

    stockLabel->setText(QString("库存 %1").arg(dish.stock));
    stockLabel->setStyleSheet("font-size:12px;color:#9ca3af;border:none;background:transparent;");

    soldLabel = new QLabel(QString("已售 %1 份").arg(dish.soldCount), this);
    soldLabel->setStyleSheet("font-size:12px;color:#fb923c;border:none;background:transparent;");

    //加减按钮、数量文字控件创建
    minusButton = new QPushButton("−", this);
    plusButton = new QPushButton("+", this);
    quantityLabel = new QLabel("0", this);
    quantityLabel->setAlignment(Qt::AlignCenter);
    quantityLabel->setMinimumWidth(45);
    quantityLabel->setStyleSheet("font-size:18px;font-weight:bold;color:#374151;border:none;background:transparent;");
    // 统一按钮尺寸、橙色圆角样式
    for(QPushButton *button : {minusButton, plusButton})
    {
        button->setFixedSize(42, 38);
        button->setStyleSheet(
            "QPushButton{background:#f97316;color:white;border:none;border-radius:10px;font-size:24px;font-weight:bold;}"
            "QPushButton:hover{background:#ea580c;} QPushButton:disabled{background:#e5e7eb;color:#9ca3af;}"
        );
    }

    // 整体垂直布局组装（卡片内部排版）
    QVBoxLayout *cardLayout = new QVBoxLayout(this);
    cardLayout->setContentsMargins(0, 0, 0, 14);  // 整体底部留白14
    cardLayout->setSpacing(7);
    cardLayout->addWidget(imageLabel, 0, Qt::AlignCenter);

    nameLabel->setContentsMargins(16,0,16,0);
    cardLayout->addWidget(nameLabel);
    // 第一行水平布局：价格 左，库存 右
    QHBoxLayout *priceRow = new QHBoxLayout;
    priceRow->addWidget(priceLabel);
    priceRow->addStretch();
    priceRow->addWidget(stockLabel);
    cardLayout->addLayout(priceRow);
    // 第二行水平布局：销量文字 + 减号 + 数量 + 加号
    QHBoxLayout *salesRow = new QHBoxLayout;
    salesRow->addWidget(soldLabel);
    salesRow->addStretch();
    salesRow->addWidget(minusButton);
    salesRow->addWidget(quantityLabel);
    salesRow->addWidget(plusButton);
    cardLayout->addLayout(salesRow);

    setLayout(cardLayout);
    priceRow->setContentsMargins(16,0,16,0);
    salesRow->setContentsMargins(16,0,16,0);
    setStyleSheet("QLabel{background:transparent;border:none;}");


    //-----------热销角标------------------------
    //热销角标（recommended=true 才创建）
    if(recommended)
    {
        QLabel *badge = new QLabel("🔥 热销", this);
        badge->setAlignment(Qt::AlignCenter);
        badge->setFixedSize(78, 30);
        badge->move(20, 20);
        badge->setStyleSheet(
            "background:#f97316;color:white;border:none;border-radius:15px;"
            "font-size:14px;font-weight:bold;padding:2px;"
        );
        badge->raise();
    }

    //=============初始化购物车数量 0，绑定按钮点击信号===========
    setQuantity(0);
    connect(plusButton, &QPushButton::clicked, this, [this](){
        if(currentQuantity < m_dish.stock)
            emit increaseDish(m_dish);
    });
    connect(minusButton, &QPushButton::clicked, this, [this](){
        if(currentQuantity > 0)
            emit decreaseDish(m_dish.id);
    });

    //========图片缓存配置、拼接后端图片地址==========
    static const bool cacheConfigured=[](){QPixmapCache::setCacheLimit(65536);return true;}();
    Q_UNUSED(cacheConfigured);
    QString picturePath=dish.picture.trimmed();
    // 自动拼接完整图片URL：本地后端8080端口
    const QUrl imageUrl=picturePath.startsWith("http://")||picturePath.startsWith("https://")
        ? QUrl(picturePath)
        : picturePath.startsWith("/images/")
            ? QUrl("http://localhost:8080"+picturePath)
            : QUrl("http://localhost:8080/images/"+picturePath);
    //统一图片请求地址，用 URL 作为缓存 key，避免重复下载同一张图。
    const QString cacheKey="dish-card:"+imageUrl.toString();

    // 缓存命中直接显示图片
    QPixmap cachedPixmap;
    if(!picturePath.isEmpty() && QPixmapCache::find(cacheKey,&cachedPixmap)){
        imageLoaded=true;
        imageLabel->setStyleSheet("background:transparent;border-radius:16px;");
        imageLabel->setPixmap(cachedPixmap);
        return;
    }
    if(picturePath.isEmpty()){
        imageLabel->setText("暂无图片");
        return;
    }

    // 异步网络下载图片 + 手动裁剪圆角存入缓存
    QNetworkReply *reply = manager->get(QNetworkRequest(imageUrl));
    connect(reply, &QNetworkReply::finished, this, [this, reply, cacheKey](){
        QPixmap pixmap;
        if(reply->error() == QNetworkReply::NoError && pixmap.loadFromData(reply->readAll()))
        {
            const QSize size=imageLabel->size();
            // 图片等比例放大铺满框
            const QPixmap scaled=pixmap.scaled(size,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
            QPixmap rounded(size);
            rounded.fill(Qt::transparent);
            QPainter painter(&rounded);
            painter.setRenderHint(QPainter::Antialiasing,true);
            QPainterPath clip;
            clip.addRoundedRect(QRectF(0,0,size.width(),size.height()),23,23);
            painter.setClipPath(clip);
            const int x=(size.width()-scaled.width())/2;
            const int y=(size.height()-scaled.height())/2;
            painter.drawPixmap(x,y,scaled);
            imageLoaded = true;
            //存入全局缓存
            QPixmapCache::insert(cacheKey,rounded);
            imageLabel->setStyleSheet("background:transparent;border-radius:16px;");
            imageLabel->setPixmap(rounded);
        }
        else {
            imageLabel->setText("暂无图片");
            imageLabel->setStyleSheet(darkMode
                ? "background:#333333;color:#808080;border-radius:16px;font-size:13px;"
                : "background:#f0ebe4;color:#999999;border-radius:16px;font-size:13px;");
        }
        reply->deleteLater();
    });
}

//更新卡片显示选购份数
void DishCard::setQuantity(int quantity)
{
    // 限制范围：0 ~ 菜品库存，防止负数、超库存
    currentQuantity = qBound(0, quantity, m_dish.stock);
    quantityLabel->setText(QString::number(currentQuantity));
    // 0份时减号禁用
    minusButton->setEnabled(currentQuantity > 0);
    // 库存满加号禁用
    plusButton->setEnabled(currentQuantity < m_dish.stock);
}

//切换深浅主题
void DishCard::setDarkMode(bool enabled)
{
    darkMode = enabled;
    if(enabled){
        //深色模式：文字白色、深色背景、数量框深色底色
        setStyleSheet("QLabel{background:transparent;border:none;}");
        nameLabel->setStyleSheet("font-size:19px;font-weight:800;color:#f5f5f5;background:transparent;");
        priceLabel->setStyleSheet("font-size:24px;font-weight:900;color:#f97316;background:transparent;");
        stockLabel->setStyleSheet("font-size:12px;color:#b0b0b0;background:transparent;");
        soldLabel->setStyleSheet("font-size:12px;color:#fb923c;background:transparent;");
        quantityLabel->setStyleSheet("font-size:18px;font-weight:bold;color:white;background:#3a3a3a;border-radius:9px;padding:3px 8px;");
        //按钮禁用灰色适配深色
        const QString darkButtonStyle="QPushButton{background:#f97316;color:white;border:none;border-radius:10px;font-size:24px;font-weight:bold;} QPushButton:hover{background:#ea580c;} QPushButton:disabled{background:#3a3a3a;color:#808080;}";
        minusButton->setStyleSheet(darkButtonStyle);
        plusButton->setStyleSheet(darkButtonStyle);
    }else{
        //浅色餐厅暖橙风格
        setStyleSheet("QLabel{background:transparent;border:none;}");
        nameLabel->setStyleSheet("font-size:19px;font-weight:800;color:#1f2937;background:transparent;");
        priceLabel->setStyleSheet("font-size:24px;font-weight:900;color:#f97316;background:transparent;");
        stockLabel->setStyleSheet("font-size:12px;color:#9ca3af;background:transparent;");
        soldLabel->setStyleSheet("font-size:12px;color:#fb923c;background:transparent;");
        quantityLabel->setStyleSheet("font-size:18px;font-weight:bold;color:#374151;background:transparent;");
        const QString style="QPushButton{background:#f97316;color:white;border:none;border-radius:10px;font-size:24px;font-weight:bold;} QPushButton:hover{background:#ea580c;} QPushButton:disabled{background:#e5e7eb;color:#9ca3af;}";
        minusButton->setStyleSheet(style);plusButton->setStyleSheet(style);
    }
    // 图片未加载时同步修改占位背景色
    if(!imageLoaded)
        imageLabel->setStyleSheet(enabled
            ? "background:#333333;color:#808080;border-radius:16px;font-size:13px;"
            : "background:#f0ebe4;color:#999999;border-radius:16px;font-size:13px;");
    update();// 触发paintEvent重绘卡片底色
}

//绘制卡片整体圆角底板
void DishCard::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);// 抗锯齿，圆角顺滑
    // 根据明暗切换边框、卡片底色
    painter.setPen(QPen(QColor(darkMode ? "#3a3a3a" : "#ffffff"), 1));
    painter.setBrush(QColor(darkMode ? "#2a2a2a" : "#ffffff"));
    //绘制卡片外框圆角矩形
    painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), darkMode?16:24, darkMode?16:24);
}


//enterEvent /leaveEvent 鼠标移入、移出
void DishCard::enterEvent(QEnterEvent *event)
{
    QWidget::enterEvent(event);
    if(hovered)
        return;
    hovered = true;
    animateHover(true); // 启动上浮动画
}

void DishCard::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    if(!hovered)
        return;
    hovered = false;
    animateHover(false); // 收回动画
}

//悬浮动画核心逻辑
void DishCard::animateHover(bool raised)
{
    shadowBlurAnimation->stop();
    shadowOffsetAnimation->stop();
    // 动画起点=当前阴影状态
    shadowBlurAnimation->setStartValue(cardShadow->blurRadius());
    shadowBlurAnimation->setEndValue(raised ? 28.0 : 14.0);
    //悬浮状态终点：模糊28，向下偏移11，阴影更深
    shadowOffsetAnimation->setStartValue(cardShadow->offset());
    shadowOffsetAnimation->setEndValue(QPointF(0, raised ? 11 : 4));
    cardShadow->setColor(darkMode
        ? QColor(0, 0, 0, raised ? 150 : 95)
        : QColor(73, 45, 20, raised ? 85 : 45));
    shadowBlurAnimation->start();
    shadowOffsetAnimation->start();
}


DishCard::~DishCard()
{
    delete ui;
}

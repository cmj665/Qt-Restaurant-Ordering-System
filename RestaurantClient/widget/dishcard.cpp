#include "dishcard.h"
#include "ui_dishcard.h"

#include <QHBoxLayout>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVBoxLayout>
#include <QPainterPath>
#include <QPainter>
#include <QPixmapCache>

DishCard::DishCard(const Dish &dish, QWidget *parent, bool recommended)
    : QWidget(parent)
    , ui(new Ui::DishCard)
    , m_dish(dish)
    , manager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground,false);
    setAttribute(Qt::WA_TranslucentBackground,true);
    setFixedSize(300, 388);

    imageLabel = new QLabel(this);
    imageLabel->setFixedSize(298, 190);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("background:#f0ebe4;border-radius:16px;color:#999999;font-size:13px;");
    nameLabel = ui->nameLabel;
    priceLabel = ui->priceLabel;
    stockLabel = ui->stockLabel;
    ui->addButton->hide();

    nameLabel->setText(dish.name);
    nameLabel->setWordWrap(true);
    nameLabel->setMinimumHeight(42);
    nameLabel->setStyleSheet("font-size:19px;font-weight:800;color:#1f2937;border:none;background:transparent;");
    priceLabel->setText(QString("￥%1").arg(dish.price, 0, 'f', 2));
    priceLabel->setStyleSheet("font-size:24px;font-weight:900;color:#f97316;border:none;background:transparent;");
    stockLabel->setText(QString("库存 %1").arg(dish.stock));
    stockLabel->setStyleSheet("font-size:12px;color:#9ca3af;border:none;background:transparent;");
    soldLabel = new QLabel(QString("已售 %1 份").arg(dish.soldCount), this);
    soldLabel->setStyleSheet("font-size:12px;color:#fb923c;border:none;background:transparent;");

    minusButton = new QPushButton("−", this);
    plusButton = new QPushButton("+", this);
    quantityLabel = new QLabel("0", this);
    quantityLabel->setAlignment(Qt::AlignCenter);
    quantityLabel->setMinimumWidth(45);
    quantityLabel->setStyleSheet("font-size:18px;font-weight:bold;color:#374151;border:none;background:transparent;");
    for(QPushButton *button : {minusButton, plusButton})
    {
        button->setFixedSize(42, 38);
        button->setStyleSheet(
            "QPushButton{background:#f97316;color:white;border:none;border-radius:10px;font-size:24px;font-weight:bold;}"
            "QPushButton:hover{background:#ea580c;} QPushButton:disabled{background:#e5e7eb;color:#9ca3af;}"
        );
    }

    QVBoxLayout *cardLayout = new QVBoxLayout(this);
    cardLayout->setContentsMargins(0, 0, 0, 14);
    cardLayout->setSpacing(7);
    cardLayout->addWidget(imageLabel, 0, Qt::AlignCenter);
    nameLabel->setContentsMargins(16,0,16,0);
    cardLayout->addWidget(nameLabel);
    QHBoxLayout *priceRow = new QHBoxLayout;
    priceRow->addWidget(priceLabel);
    priceRow->addStretch();
    priceRow->addWidget(stockLabel);
    cardLayout->addLayout(priceRow);
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

    setQuantity(0);
    connect(plusButton, &QPushButton::clicked, this, [this](){
        if(currentQuantity < m_dish.stock)
            emit increaseDish(m_dish);
    });
    connect(minusButton, &QPushButton::clicked, this, [this](){
        if(currentQuantity > 0)
            emit decreaseDish(m_dish.id);
    });

    static const bool cacheConfigured=[](){QPixmapCache::setCacheLimit(65536);return true;}();
    Q_UNUSED(cacheConfigured);
    QString picturePath=dish.picture.trimmed();
    const QUrl imageUrl=picturePath.startsWith("http://")||picturePath.startsWith("https://")
        ? QUrl(picturePath)
        : picturePath.startsWith("/images/")
            ? QUrl("http://localhost:8080"+picturePath)
            : QUrl("http://localhost:8080/images/"+picturePath);
    const QString cacheKey="dish-card:"+imageUrl.toString();
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

    QNetworkReply *reply = manager->get(QNetworkRequest(imageUrl));
    connect(reply, &QNetworkReply::finished, this, [this, reply, cacheKey](){
        QPixmap pixmap;
        if(reply->error() == QNetworkReply::NoError && pixmap.loadFromData(reply->readAll()))
        {
            const QSize size=imageLabel->size();
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

void DishCard::setQuantity(int quantity)
{
    currentQuantity = qBound(0, quantity, m_dish.stock);
    quantityLabel->setText(QString::number(currentQuantity));
    minusButton->setEnabled(currentQuantity > 0);
    plusButton->setEnabled(currentQuantity < m_dish.stock);
}

void DishCard::setDarkMode(bool enabled)
{
    darkMode = enabled;
    if(enabled){
        setStyleSheet("QLabel{background:transparent;border:none;}");
        nameLabel->setStyleSheet("font-size:19px;font-weight:800;color:#f5f5f5;background:transparent;");
        priceLabel->setStyleSheet("font-size:24px;font-weight:900;color:#f97316;background:transparent;");
        stockLabel->setStyleSheet("font-size:12px;color:#b0b0b0;background:transparent;");
        soldLabel->setStyleSheet("font-size:12px;color:#fb923c;background:transparent;");
        quantityLabel->setStyleSheet("font-size:18px;font-weight:bold;color:white;background:#3a3a3a;border-radius:9px;padding:3px 8px;");
        const QString darkButtonStyle="QPushButton{background:#f97316;color:white;border:none;border-radius:10px;font-size:24px;font-weight:bold;} QPushButton:hover{background:#ea580c;} QPushButton:disabled{background:#3a3a3a;color:#808080;}";
        minusButton->setStyleSheet(darkButtonStyle);
        plusButton->setStyleSheet(darkButtonStyle);
    }else{
        setStyleSheet("QLabel{background:transparent;border:none;}");
        nameLabel->setStyleSheet("font-size:19px;font-weight:800;color:#1f2937;background:transparent;");
        priceLabel->setStyleSheet("font-size:24px;font-weight:900;color:#f97316;background:transparent;");
        stockLabel->setStyleSheet("font-size:12px;color:#9ca3af;background:transparent;");
        soldLabel->setStyleSheet("font-size:12px;color:#fb923c;background:transparent;");
        quantityLabel->setStyleSheet("font-size:18px;font-weight:bold;color:#374151;background:transparent;");
        const QString style="QPushButton{background:#f97316;color:white;border:none;border-radius:10px;font-size:24px;font-weight:bold;} QPushButton:hover{background:#ea580c;} QPushButton:disabled{background:#e5e7eb;color:#9ca3af;}";
        minusButton->setStyleSheet(style);plusButton->setStyleSheet(style);
    }
    if(!imageLoaded)
        imageLabel->setStyleSheet(enabled
            ? "background:#333333;color:#808080;border-radius:16px;font-size:13px;"
            : "background:#f0ebe4;color:#999999;border-radius:16px;font-size:13px;");
    update();
}

void DishCard::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(darkMode ? "#3a3a3a" : "#ffffff"), 1));
    painter.setBrush(QColor(darkMode ? "#2a2a2a" : "#ffffff"));
    painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), darkMode?16:24, darkMode?16:24);
}


DishCard::~DishCard()
{
    delete ui;
}

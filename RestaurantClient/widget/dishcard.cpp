#include "dishcard.h"
#include "ui_dishcard.h"

#include <QHBoxLayout>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QPainterPath>
#include <QPainter>

DishCard::DishCard(const Dish &dish, QWidget *parent, bool recommended)
    : QWidget(parent)
    , ui(new Ui::DishCard)
    , m_dish(dish)
    , manager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground,true);
    setFixedSize(300, 388);

    imageLabel = new QLabel("图片加载中...", this);
    imageLabel->setFixedSize(298, 190);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #ff8c42,stop:1 #ffd166);border-radius:22px 22px 0 0;color:white;font-size:18px;");
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
    setStyleSheet(
        "DishCard{background:#ffffff;border:1px solid #ffffff;border-radius:24px;}"
        "DishCard QLabel{background:transparent;border:none;}"
    );
    auto *shadow=new QGraphicsDropShadowEffect(this);shadow->setBlurRadius(30);shadow->setOffset(0,10);shadow->setColor(QColor(145,90,45,65));setGraphicsEffect(shadow);

    if(recommended)
    {
        QLabel *badge = new QLabel("🔥 热销", this);
        badge->setAlignment(Qt::AlignCenter);
        badge->setFixedSize(78, 30);
        badge->move(20, 20);
        badge->setStyleSheet(
            "background:#ef4444;color:white;border:none;border-radius:15px;"
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

    QNetworkReply *reply = manager->get(QNetworkRequest(
        QUrl("http://localhost:8080/images/" + dish.picture)));
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
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
            imageLabel->setPixmap(rounded);
        }
        else
            imageLabel->setText("暂无图片");
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
    if(enabled){
        setStyleSheet("DishCard{background:#505762;border:1px solid #69717d;border-radius:24px;} DishCard QLabel{background:transparent;border:none;}");
        nameLabel->setStyleSheet("font-size:19px;font-weight:800;color:#f8fafc;background:transparent;");
        priceLabel->setStyleSheet("font-size:24px;font-weight:900;color:#ff6b35;background:transparent;");
        stockLabel->setStyleSheet("font-size:12px;color:#d1d5db;background:transparent;");
        soldLabel->setStyleSheet("font-size:12px;color:#fb923c;background:transparent;");
        quantityLabel->setStyleSheet("font-size:18px;font-weight:bold;color:white;background:transparent;");
        minusButton->setStyleSheet("QPushButton{background:#69717d;color:white;border:none;border-radius:10px;font-size:24px;} QPushButton:disabled{color:#9ca3af;}");
        plusButton->setStyleSheet("QPushButton{background:#e64b35;color:white;border:none;border-radius:10px;font-size:24px;} QPushButton:disabled{background:#69717d;color:#9ca3af;}");
    }else{
        setStyleSheet("DishCard{background:#ffffff;border:1px solid #ffffff;border-radius:24px;} DishCard QLabel{background:transparent;border:none;}");
        nameLabel->setStyleSheet("font-size:19px;font-weight:800;color:#1f2937;background:transparent;");
        priceLabel->setStyleSheet("font-size:24px;font-weight:900;color:#f97316;background:transparent;");
        stockLabel->setStyleSheet("font-size:12px;color:#9ca3af;background:transparent;");
        soldLabel->setStyleSheet("font-size:12px;color:#fb923c;background:transparent;");
        quantityLabel->setStyleSheet("font-size:18px;font-weight:bold;color:#374151;background:transparent;");
        const QString style="QPushButton{background:#f97316;color:white;border:none;border-radius:10px;font-size:24px;font-weight:bold;} QPushButton:hover{background:#ea580c;} QPushButton:disabled{background:#e5e7eb;color:#9ca3af;}";
        minusButton->setStyleSheet(style);plusButton->setStyleSheet(style);
    }
}


DishCard::~DishCard()
{
    delete ui;
}

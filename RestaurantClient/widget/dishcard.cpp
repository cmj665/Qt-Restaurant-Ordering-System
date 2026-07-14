#include "dishcard.h"
#include "ui_dishcard.h"

#include <QHBoxLayout>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVBoxLayout>

DishCard::DishCard(const Dish &dish, QWidget *parent, bool recommended)
    : QWidget(parent)
    , ui(new Ui::DishCard)
    , m_dish(dish)
    , manager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    setFixedSize(310, 390);

    imageLabel = new QLabel("图片加载中...", this);
    imageLabel->setFixedSize(278, 180);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("background:#f3f5f7;border-radius:10px;color:#888;");
    nameLabel = ui->nameLabel;
    priceLabel = ui->priceLabel;
    stockLabel = ui->stockLabel;
    ui->addButton->hide();

    nameLabel->setText(dish.name);
    nameLabel->setWordWrap(true);
    nameLabel->setMinimumHeight(42);
    nameLabel->setStyleSheet("font-size:19px;font-weight:bold;color:#2c3e50;border:none;");
    priceLabel->setText(QString("￥%1").arg(dish.price, 0, 'f', 2));
    priceLabel->setStyleSheet("font-size:20px;font-weight:bold;color:#e74c3c;border:none;");
    stockLabel->setText(QString("库存 %1").arg(dish.stock));
    stockLabel->setStyleSheet("font-size:13px;color:#7f8c8d;border:none;");
    soldLabel = new QLabel(QString("已售 %1 份").arg(dish.soldCount), this);
    soldLabel->setStyleSheet("font-size:13px;color:#e67e22;border:none;");

    minusButton = new QPushButton("−", this);
    plusButton = new QPushButton("+", this);
    quantityLabel = new QLabel("0", this);
    quantityLabel->setAlignment(Qt::AlignCenter);
    quantityLabel->setMinimumWidth(45);
    quantityLabel->setStyleSheet("font-size:18px;font-weight:bold;border:none;");
    for(QPushButton *button : {minusButton, plusButton})
    {
        button->setFixedSize(42, 38);
        button->setStyleSheet(
            "QPushButton{background:#3498db;color:white;border:none;border-radius:19px;font-size:24px;font-weight:bold;}"
            "QPushButton:hover{background:#2980b9;} QPushButton:disabled{background:#bdc3c7;}"
        );
    }

    QVBoxLayout *cardLayout = new QVBoxLayout(this);
    cardLayout->setContentsMargins(15, 15, 15, 15);
    cardLayout->setSpacing(7);
    cardLayout->addWidget(imageLabel, 0, Qt::AlignCenter);
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
    setStyleSheet("DishCard{background:white;border:1px solid #dfe6e9;border-radius:13px;}");

    if(recommended)
    {
        QLabel *badge = new QLabel("🔥 热销", this);
        badge->setAlignment(Qt::AlignCenter);
        badge->setFixedSize(78, 30);
        badge->move(20, 20);
        badge->setStyleSheet(
            "background:#e74c3c;color:white;border:none;border-radius:15px;"
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
            imageLabel->setPixmap(pixmap.scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
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

DishCard::~DishCard()
{
    delete ui;
}

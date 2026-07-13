#include "dishcard.h"
#include "ui_dishcard.h"

DishCard::DishCard(const Dish &dish,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DishCard)
    ,m_dish(dish)
{
    ui->setupUi(this);
    setFixedSize(300,250);

    //图片
    imageLabel =new QLabel(this);
    imageLabel->setFixedSize(260,150);   //看能不能缩放
    imageLabel->setText("加载图片...");

    //文字
    nameLabel = new QLabel(this);
    priceLabel = new QLabel(this);
    stockLabel = new QLabel(this);
    addButton = new QPushButton("加入购物车",this);

    nameLabel->setText(dish.name);
    priceLabel->setText("价格:"+QString::number(dish.price));
    stockLabel->setText("库存:"+QString::number(dish.stock));
    QVBoxLayout *layout =new QVBoxLayout(this);

    layout->addWidget(imageLabel);
    layout->addWidget(nameLabel);
    layout->addWidget(priceLabel);
    layout->addWidget(stockLabel);
    layout->addWidget(addButton);
    setLayout(layout);

    manager = new QNetworkAccessManager(this);

    //加载图片
    QString url = "http://localhost:8080/images/"+dish.picture;
    QNetworkRequest request{QUrl(url)};
    QNetworkReply *reply = manager->get(request);
    connect(reply,&QNetworkReply::finished,this,[this,reply](){
        QByteArray data =reply->readAll();
        QPixmap pix;
        if(pix.loadFromData(data))
        {
            imageLabel->setPixmap(pix.scaled(imageLabel->size(),Qt::KeepAspectRatio));
        }
        else
        {
            imageLabel->setText("图片加载失败");
        }
        reply->deleteLater();

    });

    //加入购物车
    //按钮点击
    connect(addButton,&QPushButton::clicked,this,[this](){
        emit addDish(m_dish);
    });

    setStyleSheet(
        "QWidget{"
        "border:1px solid gray;"
        "border-radius:10px;"
        "background:white;"
        "}"
        );
}

DishCard::~DishCard()
{
    delete ui;
}

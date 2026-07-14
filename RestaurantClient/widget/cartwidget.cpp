#include "cartwidget.h"
#include "ui_cartwidget.h"
#include "../network/networkmanager.h"

#include <QCloseEvent>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QTabBar>
#include <QJsonArray>
#include <QJsonObject>
#include <QResizeEvent>
#include <QPainterPath>
#include <QRegion>
#include <QSizePolicy>
#include <QPainter>
#include <QShowEvent>

class EqualTabBar final : public QTabBar
{
protected:
    QSize tabSizeHint(int index) const override
    {
        QSize size = QTabBar::tabSizeHint(index);
        if(count() > 0 && width() > 0)
            size.setWidth(width() / count());
        return size;
    }

    QSize minimumTabSizeHint(int index) const override
    {
        return tabSizeHint(index);
    }
};

class EqualTabWidget final : public QTabWidget
{
public:
    explicit EqualTabWidget(QWidget *parent = nullptr) : QTabWidget(parent)
    {
        setTabBar(new EqualTabBar);
    }
};

CartWidget::CartWidget(int tableIdValue, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CartWidget)
    , imageManager(new QNetworkAccessManager(this))
    , tableId(tableIdValue)
    , orderNetwork(new NetworkManager(this))
{
    ui->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowTitle("购物车");
    resize(560, 580);
    setMinimumSize(460, 440);
    QWidget *content = ui->verticalLayout->parentWidget();
    content->setObjectName("cartContent");
    content->setStyleSheet("QWidget#cartContent{background:transparent;}");
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 18, 22, 22);
    rootLayout->addWidget(content);
    setLayout(rootLayout);

    ui->titleLabel->setText(QString("%1号桌").arg(tableId));
    ui->titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->titleLabel->setStyleSheet("font-size:25px;font-weight:800;color:white;padding:8px;");
    ui->verticalLayout->removeWidget(ui->titleLabel);
    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->setContentsMargins(6, 2, 2, 4);
    headerLayout->addWidget(ui->titleLabel);
    headerLayout->addStretch();
    QPushButton *drawerCloseButton = new QPushButton("×", this);
    drawerCloseButton->setFixedSize(40, 40);
    drawerCloseButton->setCursor(Qt::PointingHandCursor);
    drawerCloseButton->setStyleSheet(
        "QPushButton{background:#343d4d;color:#e2e8f0;border:none;border-radius:20px;font-size:25px;}"
        "QPushButton:hover{background:#465267;color:white;}"
    );
    headerLayout->addWidget(drawerCloseButton);
    ui->verticalLayout->insertLayout(0, headerLayout);

    ui->verticalLayout->removeWidget(ui->cartListWidget);
    QWidget *newItemsPage = new QWidget(this);
    newItemsPage->setStyleSheet("background:#242e3a;");
    QVBoxLayout *newItemsLayout = new QVBoxLayout(newItemsPage);
    newItemsLayout->setContentsMargins(0, 12, 0, 10);
    newItemsLayout->addWidget(ui->cartListWidget);

    QWidget *orderedPage = new QWidget(this);
    orderedPage->setStyleSheet("background:#242e3a;");
    QVBoxLayout *orderedLayout = new QVBoxLayout(orderedPage);
    orderedLayout->setContentsMargins(0, 12, 0, 10);
    orderedListWidget = new QListWidget(orderedPage);
    orderedSummaryLabel = new QLabel("正在读取已下单菜品…", orderedPage);
    orderedSummaryLabel->setAlignment(Qt::AlignRight);
    orderedSummaryLabel->setStyleSheet("font-size:18px;font-weight:700;color:#ffb15c;padding:8px;");
    orderedLayout->addWidget(orderedListWidget, 1);
    orderedLayout->addWidget(orderedSummaryLabel);

    tabWidget = new EqualTabWidget(this);
    tabWidget->addTab(newItemsPage, "新加菜");
    tabWidget->addTab(orderedPage, "已下单");
    tabWidget->tabBar()->setExpanding(true);
    tabWidget->tabBar()->setUsesScrollButtons(false);
    tabWidget->tabBar()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tabWidget->setDocumentMode(false);
    tabWidget->setStyleSheet(
        "QTabWidget{background:#1d2530;}"
        "QTabWidget::pane{border:1px solid #4a5666;border-radius:0 0 15px 15px;background:#242e3a;top:-1px;}"
        "QTabBar{background:#1d2530;}"
        "QTabBar::tab{padding:13px 8px;background:#2a3542;color:#b8c2cf;border:1px solid #4a5666;font-size:17px;}"
        "QTabBar::tab:first{border-top-left-radius:15px;}"
        "QTabBar::tab:last{border-top-right-radius:15px;}"
        "QTabBar::tab:selected{background:#465568;color:#ffffff;border-bottom-color:#465568;font-weight:700;}"
        "QTabBar::tab:hover:!selected{background:#354252;color:#eef2f7;}"
    );
    QWidget *tabContainer = new QWidget(this);
    tabContainer->setStyleSheet("background:transparent;");
    QVBoxLayout *tabContainerLayout = new QVBoxLayout(tabContainer);
    tabContainerLayout->setContentsMargins(10, 0, 10, 0);
    tabContainerLayout->setSpacing(0);
    tabContainerLayout->addWidget(tabWidget);
    ui->verticalLayout->insertWidget(1, tabContainer, 1);
    ui->totalLabel->setAlignment(Qt::AlignRight);
    ui->totalLabel->setStyleSheet("font-size:20px;font-weight:bold;color:#ffb15c;padding:8px;");
    const QString listStyle =
        "QListWidget{background:#202936;color:#e8edf3;border:1px solid #4a5666;border-radius:14px;outline:none;padding:10px;}"
        "QListWidget::item{border-bottom:1px solid #3c4857;padding-left:4px;}"
        "QListWidget::item:selected{background:#354354;border-radius:9px;}";
    ui->cartListWidget->setStyleSheet(listStyle);
    orderedListWidget->setStyleSheet(listStyle);

    ui->removeButton->setText("删除选中");
    ui->clearButton->setText("清空购物车");
    ui->submitButton->setText("提交订单");
    ui->detailButton->setText("查看当前订单");
    ui->checkoutButton->setText("确认结账");
    ui->submitButton->setMinimumHeight(48);
    ui->detailButton->setMinimumHeight(42);
    ui->checkoutButton->setMinimumHeight(48);
    ui->submitButton->setStyleSheet("QPushButton{background:#263a30;color:#d9f3e3;border:1px solid #4b765d;border-radius:7px;font-size:17px;} QPushButton:hover{background:#30483b;} QPushButton:disabled{background:#181b20;color:#60656d;border-color:#30343a;}");
    ui->checkoutButton->setStyleSheet("QPushButton{background:#443426;color:#ffe2bd;border:1px solid #806044;border-radius:7px;font-size:17px;} QPushButton:hover{background:#523e2d;} QPushButton:disabled{background:#181b20;color:#60656d;border-color:#30343a;}");
    const QString secondaryButtonStyle =
        "QPushButton{background:#171a1f;color:#c9ced6;border:1px solid #40454e;border-radius:7px;padding:8px;font-size:14px;}"
        "QPushButton:hover{background:#25292f;border-color:#5a616c;color:white;} QPushButton:disabled{color:#555b64;background:#111318;border-color:#292d33;}";
    ui->removeButton->setStyleSheet(secondaryButtonStyle);
    ui->clearButton->setStyleSheet(secondaryButtonStyle);
    ui->detailButton->setStyleSheet(secondaryButtonStyle);

    connect(ui->cartListWidget, &QListWidget::itemSelectionChanged, this, &CartWidget::updateButtons);
    connect(ui->removeButton, &QPushButton::clicked, this, &CartWidget::removeCurrentDish);
    connect(ui->clearButton, &QPushButton::clicked, this, [this](){
        if(m_cart.isEmpty())
            return;
        if(QMessageBox::question(this, "确认清空", "确定清空购物车中的所有菜品吗？") == QMessageBox::Yes)
            clearCart();
    });
    connect(ui->submitButton, &QPushButton::clicked, this, [this](){
        if(m_cart.isEmpty() || submitting)
            return;
        setSubmitting(true);
        emit submitOrderRequested(cartItems());
    });
    connect(ui->checkoutButton, &QPushButton::clicked, this, [this](){
        if(!m_cart.isEmpty())
        {
            QMessageBox::information(this, "请先提交", "购物车还有未提交的菜品，请先提交订单再结账。");
            return;
        }
        emit checkoutRequested();
    });
    connect(ui->detailButton, &QPushButton::clicked, this, &CartWidget::detailRequested);
    connect(drawerCloseButton, &QPushButton::clicked, this, &CartWidget::closeRequested);
    connect(tabWidget, &QTabWidget::currentChanged, this, [this](int index){
        if(index == 1)
            refreshOrderedOrder();
    });
    connect(orderNetwork, &NetworkManager::orderDetailReceived,
            this, &CartWidget::renderOrderedOrder);

    refreshCart();
}

void CartWidget::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}

void CartWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if(tabWidget && tabWidget->tabBar())
    {
        tabWidget->tabBar()->setFixedWidth(tabWidget->width());
        tabWidget->tabBar()->updateGeometry();
    }
    QPainterPath roundedPath;
    roundedPath.addRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 24, 24);
    setMask(QRegion(roundedPath.toFillPolygon().toPolygon()));
}

void CartWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    QPainterPath roundedPath;
    roundedPath.addRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 24, 24);
    setMask(QRegion(roundedPath.toFillPolygon().toPolygon()));
    if(tabWidget && tabWidget->tabBar())
    {
        tabWidget->tabBar()->setFixedWidth(tabWidget->width());
        tabWidget->tabBar()->updateGeometry();
    }
}

void CartWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor("#4a5666"), 1));
    painter.setBrush(QColor("#1d2530"));
    painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 24, 24);
}

void CartWidget::setSubmitting(bool value)
{
    submitting = value;
    ui->submitButton->setText(value ? "订单处理中..." : "提交订单");
    updateButtons();
}

void CartWidget::addDish(const Dish &dish)
{
    if(submitting)
        return;
    if(!m_cart.contains(dish.id))
        m_cart.insert(dish.id, CartItem{dish, 1});
    else if(m_cart[dish.id].count < m_cart[dish.id].dish.stock)
        m_cart[dish.id].count++;
    emit quantityChanged(dish.id, m_cart[dish.id].count);
    refreshCart();
}

void CartWidget::decreaseDish(int dishId)
{
    changeQuantity(dishId, -1);
}

int CartWidget::quantityForDish(int dishId) const
{
    return m_cart.contains(dishId) ? m_cart.value(dishId).count : 0;
}

void CartWidget::changeQuantity(int dishId, int delta)
{
    if(submitting || !m_cart.contains(dishId))
        return;
    CartItem &item = m_cart[dishId];
    const int newCount = qBound(0, item.count + delta, item.dish.stock);
    if(newCount == 0)
        m_cart.remove(dishId);
    else
        item.count = newCount;
    emit quantityChanged(dishId, newCount);
    refreshCart();
}

void CartWidget::refreshCart()
{
    ui->cartListWidget->clear();
    for(const CartItem &item : m_cart)
    {
        const double subtotal = item.dish.price * item.count;
        QListWidgetItem *listItem = new QListWidgetItem;
        listItem->setData(Qt::UserRole, item.dish.id);
        listItem->setSizeHint(QSize(0, 92));
        ui->cartListWidget->addItem(listItem);

        QWidget *row = new QWidget(ui->cartListWidget);
        row->setStyleSheet("QWidget{background:transparent;} QLabel{color:#eef2f7;}");
        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(12, 7, 10, 7);
        QLabel *thumbnail = new QLabel("加载中", row);
        thumbnail->setFixedSize(82, 72);
        thumbnail->setAlignment(Qt::AlignCenter);
        thumbnail->setStyleSheet("background:#1b1e23;border:1px solid #343941;border-radius:7px;color:#8f96a1;");
        QLabel *description = new QLabel(
            QString("<b>%1</b><br><span style='color:#777'>单价 ￥%2</span><br>"
                    "<span style='color:#e74c3c;font-weight:bold'>小计 ￥%3</span>")
                .arg(item.dish.name)
                .arg(item.dish.price, 0, 'f', 2).arg(subtotal, 0, 'f', 2), row);
        description->setTextFormat(Qt::RichText);
        description->setStyleSheet("font-size:15px;");
        rowLayout->addWidget(thumbnail);
        rowLayout->addWidget(description, 1);
        QPushButton *minus = new QPushButton("−", row);
        QLabel *quantity = new QLabel(QString::number(item.count), row);
        QPushButton *plus = new QPushButton("+", row);
        quantity->setAlignment(Qt::AlignCenter);
        quantity->setFixedWidth(28);
        quantity->setStyleSheet("font-size:17px;font-weight:bold;");
        for(QPushButton *button : {minus, plus})
        {
            button->setFixedSize(32, 32);
            button->setStyleSheet("QPushButton{background:#252b33;color:#e4e8ee;border:1px solid #505967;border-radius:16px;font-size:19px;font-weight:bold;} QPushButton:hover{background:#343c47;} QPushButton:disabled{background:#17191d;color:#555b63;border-color:#292d33;}");
        }
        plus->setEnabled(item.count < item.dish.stock && !submitting);
        minus->setEnabled(!submitting);
        const int dishId = item.dish.id;
        connect(minus, &QPushButton::clicked, this, [this, dishId](){ changeQuantity(dishId, -1); });
        connect(plus, &QPushButton::clicked, this, [this, dishId](){ changeQuantity(dishId, 1); });
        rowLayout->addWidget(minus);
        rowLayout->addWidget(quantity);
        rowLayout->addWidget(plus);
        ui->cartListWidget->setItemWidget(listItem, row);

        if(imageCache.contains(item.dish.id))
        {
            thumbnail->setPixmap(imageCache[item.dish.id].scaled(
                thumbnail->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else
        {
            QPointer<QLabel> safeThumbnail(thumbnail);
            QNetworkReply *reply = imageManager->get(QNetworkRequest(
                QUrl("http://localhost:8080/images/" + item.dish.picture)));
            const int dishId = item.dish.id;
            connect(reply, &QNetworkReply::finished, this, [this, reply, safeThumbnail, dishId](){
                QPixmap pixmap;
                if(reply->error() == QNetworkReply::NoError && pixmap.loadFromData(reply->readAll()))
                {
                    imageCache.insert(dishId, pixmap);
                    if(safeThumbnail)
                        safeThumbnail->setPixmap(pixmap.scaled(safeThumbnail->size(),
                            Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }
                else if(safeThumbnail)
                {
                    safeThumbnail->setText("暂无图片");
                }
                reply->deleteLater();
            });
        }
    }
    ui->totalLabel->setText(QString("待提交合计：￥%1").arg(totalPrice(), 0, 'f', 2));
    int itemCount = 0;
    for(const CartItem &cartItem : m_cart)
        itemCount += cartItem.count;
    emit totalChanged(totalPrice(), itemCount);
    updateButtons();
}

void CartWidget::refreshOrderedOrder()
{
    orderedListWidget->clear();
    orderedListWidget->addItem("正在读取已下单菜品…");
    orderedSummaryLabel->setText("加载中…");
    orderNetwork->getOrderDetail(tableId);
}

void CartWidget::renderOrderedOrder(bool success, const QJsonObject &data)
{
    orderedListWidget->clear();
    const QJsonArray items = data.value("items").toArray();
    if(!success || items.isEmpty())
    {
        orderedListWidget->addItem("当前桌台暂无已下单菜品");
        orderedSummaryLabel->setText("已下单合计：￥0.00");
        return;
    }

    for(const QJsonValue &value : items)
    {
        const QJsonObject item = value.toObject();
        const int count = item.value("count").toInt();
        const double price = item.value("price").toDouble();
        QListWidgetItem *listItem = new QListWidgetItem;
        listItem->setSizeHint(QSize(0, 86));
        orderedListWidget->addItem(listItem);

        QWidget *row = new QWidget(orderedListWidget);
        row->setStyleSheet("QWidget{background:transparent;} QLabel{color:#eef2f7;}");
        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(12, 7, 10, 7);
        QLabel *thumbnail = new QLabel("图片", row);
        thumbnail->setFixedSize(76, 66);
        thumbnail->setAlignment(Qt::AlignCenter);
        thumbnail->setStyleSheet("background:#1b1e23;border:1px solid #343941;border-radius:8px;color:#8f96a1;");
        QLabel *name = new QLabel(QString("<b>%1</b><br><span style='color:#64748b'>￥%2 × %3</span>")
            .arg(item.value("dishName").toString()).arg(price, 0, 'f', 2).arg(count), row);
        name->setTextFormat(Qt::RichText);
        QLabel *subtotal = new QLabel(QString("￥%1").arg(price * count, 0, 'f', 2), row);
        subtotal->setStyleSheet("font-size:17px;font-weight:700;color:#e74c3c;");
        rowLayout->addWidget(thumbnail);
        rowLayout->addWidget(name, 1);
        rowLayout->addWidget(subtotal);
        orderedListWidget->setItemWidget(listItem, row);

        const QString picture = item.value("picture").toString();
        if(!picture.isEmpty())
        {
            QPointer<QLabel> safeThumbnail(thumbnail);
            QNetworkReply *reply = imageManager->get(QNetworkRequest(QUrl("http://localhost:8080/images/" + picture)));
            connect(reply, &QNetworkReply::finished, this, [reply, safeThumbnail](){
                QPixmap pixmap;
                if(reply->error() == QNetworkReply::NoError && pixmap.loadFromData(reply->readAll()) && safeThumbnail)
                    safeThumbnail->setPixmap(pixmap.scaled(safeThumbnail->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                else if(safeThumbnail)
                    safeThumbnail->setText("暂无图片");
                reply->deleteLater();
            });
        }
    }
    orderedSummaryLabel->setText(QString("已下单合计：￥%1")
        .arg(data.value("totalPrice").toDouble(), 0, 'f', 2));
}

void CartWidget::updateButtons()
{
    const bool hasItems = !m_cart.isEmpty();
    const bool editable = !submitting;
    ui->removeButton->setEnabled(editable && ui->cartListWidget->currentItem());
    ui->clearButton->setEnabled(editable && hasItems);
    ui->submitButton->setEnabled(editable && hasItems);
    ui->checkoutButton->setEnabled(editable && !hasItems);
    ui->detailButton->setEnabled(editable);
    ui->checkoutButton->setToolTip(hasItems ? "请先提交购物车中的菜品" : "确认停止加菜并进入待结账状态");
}

QList<CartItem> CartWidget::cartItems() const
{
    return m_cart.values();
}

double CartWidget::totalPrice() const
{
    double total = 0;
    for(const CartItem &item : m_cart)
        total += item.dish.price * item.count;
    return total;
}

void CartWidget::removeCurrentDish()
{
    QListWidgetItem *current = ui->cartListWidget->currentItem();
    if(!current)
        return;
    m_cart.remove(current->data(Qt::UserRole).toInt());
    emit quantityChanged(current->data(Qt::UserRole).toInt(), 0);
    refreshCart();
}

void CartWidget::clearCart()
{
    const QList<int> dishIds = m_cart.keys();
    m_cart.clear();
    for(int dishId : dishIds)
        emit quantityChanged(dishId, 0);
    refreshCart();
}

CartWidget::~CartWidget()
{
    delete ui;
}

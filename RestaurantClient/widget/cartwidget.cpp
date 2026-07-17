#include "cartwidget.h"
#include "ui_cartwidget.h"
#include "../network/networkmanager.h"
#include "../network/serverconfig.h"

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

//构造函数负责搭建整个界面
CartWidget::CartWidget(int tableIdValue, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CartWidget)
    , imageManager(new QNetworkAccessManager(this)) //从服务器下载菜品图片
    , tableId(tableIdValue)
    , orderNetwork(new NetworkManager(this))  //请求订单详情
{
    ui->setupUi(this);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowTitle("购物车");
    resize(560, 580);
    setMinimumSize(460, 440);

    QWidget *content = ui->verticalLayout->parentWidget();
    content->setObjectName("cartContent");
    content->setStyleSheet("QWidget#cartContent{background:transparent;}");

    //布局
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 18, 22, 22);
    rootLayout->addWidget(content);
    setLayout(rootLayout);

    //顶部标题栏
    ui->titleLabel->setText(QString("%1号桌").arg(tableId));
    ui->titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->titleLabel->setStyleSheet("font-size:25px;font-weight:800;color:#9a4312;padding:8px;");
    ui->verticalLayout->removeWidget(ui->titleLabel);

    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->setContentsMargins(6, 2, 2, 4);
    headerLayout->addWidget(ui->titleLabel);
    headerLayout->addStretch();

    //创建关闭按钮
    QPushButton *drawerCloseButton = new QPushButton("×", this);
    drawerCloseButton->setFixedSize(40, 40);
    drawerCloseButton->setCursor(Qt::PointingHandCursor);
    drawerCloseButton->setStyleSheet(
        "QPushButton{background:#ffedd5;color:#ea580c;border:none;border-radius:20px;font-size:25px;}"
        "QPushButton:hover{background:#fed7aa;}"
    );
    headerLayout->addWidget(drawerCloseButton);
    ui->verticalLayout->insertLayout(0, headerLayout);
    ui->verticalLayout->removeWidget(ui->cartListWidget);

    //新加菜”和“已下单”两个分页
    //QWidget实现两个标签页
    QWidget *newItemsPage = new QWidget(this);
    newItemsPage->setStyleSheet("background:#fffaf0;");
    QVBoxLayout *newItemsLayout = new QVBoxLayout(newItemsPage);
    newItemsLayout->setContentsMargins(0, 12, 0, 10);
    newItemsLayout->addWidget(ui->cartListWidget);

    QWidget *orderedPage = new QWidget(this);
    orderedPage->setStyleSheet("background:#fffaf0;");
    QVBoxLayout *orderedLayout = new QVBoxLayout(orderedPage);
    orderedLayout->setContentsMargins(0, 12, 0, 10);

    //创建已下单菜品列表，父容器orderedPage
    orderedListWidget = new QListWidget(orderedPage);
    //总价标签，父容器orderedPage
    orderedSummaryLabel = new QLabel("正在读取已下单菜品…", orderedPage);
    orderedSummaryLabel->setAlignment(Qt::AlignRight);
    orderedSummaryLabel->setStyleSheet("font-size:18px;font-weight:700;color:#f97316;padding:8px;");
    //列表拉伸权重1，占大部分高度
    orderedLayout->addWidget(orderedListWidget, 1);
    orderedLayout->addWidget(orderedSummaryLabel);

    //等分标签控件 EqualTabWidget（tabWidget）
    tabWidget = new EqualTabWidget(this);
    //把两个页面分别加入两个标签
    tabWidget->addTab(newItemsPage, "新加菜");
    tabWidget->addTab(orderedPage, "已下单");

    //标签条自动铺满整行
    tabWidget->tabBar()->setExpanding(true);
    tabWidget->tabBar()->setUsesScrollButtons(false);
    tabWidget->tabBar()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tabWidget->setDocumentMode(false);
    tabWidget->setStyleSheet(
        "QTabWidget{background:#fffaf0;}"
        "QTabWidget::pane{border:1px solid #fed7aa;border-radius:0 0 15px 15px;background:#fffaf0;top:-1px;}"
        "QTabBar{background:#fffaf0;}"
        "QTabBar::tab{padding:13px 8px;background:#ffedd5;color:#9a4312;border:1px solid #fed7aa;font-size:17px;}"
        "QTabBar::tab:first{border-top-left-radius:15px;}"
        "QTabBar::tab:last{border-top-right-radius:15px;}"
        "QTabBar::tab:selected{background:#ff8c42;color:white;border-bottom-color:#ff8c42;font-weight:700;}"
        "QTabBar::tab:hover:!selected{background:#fed7aa;color:#9a4312;}"
    );

    //创建透明外层容器
    QWidget *tabContainer = new QWidget(this);
    tabContainer->setStyleSheet("background:transparent;");
    QVBoxLayout *tabContainerLayout = new QVBoxLayout(tabContainer);

    tabContainerLayout->setContentsMargins(10, 0, 10, 0);
    tabContainerLayout->setSpacing(0);
    tabContainerLayout->addWidget(tabWidget);

    ui->verticalLayout->insertWidget(1, tabContainer, 1);
    ui->totalLabel->setAlignment(Qt::AlignRight);
    ui->totalLabel->setStyleSheet("font-size:20px;font-weight:bold;color:#f97316;padding:8px;");

    const QString listStyle =
        "QListWidget{background:white;color:#374151;border:1px solid #fed7aa;border-radius:14px;outline:none;padding:10px;}"
        "QListWidget::item{border-bottom:1px solid #ffedd5;padding-left:4px;}"
        "QListWidget::item:selected{background:#fff7ed;border-radius:9px;}";
    ui->cartListWidget->setStyleSheet(listStyle);
    orderedListWidget->setStyleSheet(listStyle);

    //所有按钮文字、高度、样式配置
    ui->removeButton->setText("删除选中");
    ui->clearButton->setText("清空购物车");
    ui->submitButton->setText("提交订单");
    ui->detailButton->setText("查看当前订单");
    ui->checkoutButton->setText("确认结账");
    ui->submitButton->setMinimumHeight(48);
    ui->detailButton->setMinimumHeight(42);
    ui->checkoutButton->setMinimumHeight(48);
    ui->submitButton->setStyleSheet("QPushButton{background:#f97316;color:white;border:none;border-radius:10px;font-size:17px;font-weight:700;} QPushButton:hover{background:#ea580c;} QPushButton:disabled{background:#e5e7eb;color:#9ca3af;}");
    ui->checkoutButton->setStyleSheet("QPushButton{background:#ffd166;color:#9a4312;border:none;border-radius:10px;font-size:17px;font-weight:700;} QPushButton:hover{background:#ffca45;} QPushButton:disabled{background:#e5e7eb;color:#9ca3af;}");
    const QString secondaryButtonStyle =
        "QPushButton{background:white;color:#ea580c;border:1px solid #fed7aa;border-radius:8px;padding:8px;font-size:14px;}"
        "QPushButton:hover{background:#fff7ed;} QPushButton:disabled{color:#9ca3af;background:#f3f4f6;border-color:#e5e7eb;}";
    ui->removeButton->setStyleSheet(secondaryButtonStyle);
    ui->clearButton->setStyleSheet(secondaryButtonStyle);
    ui->detailButton->setStyleSheet(secondaryButtonStyle);

    //用户选中、取消选中，或者换选了另一条菜品，就会更新购物车
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
    //index = 0 → 「新加菜」 index = 1 → 「已下单」
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
    event->ignore();  //拒绝真正关闭窗口，不会执行析构，只是隐藏业务作用
}


//resizeEvent 窗口大小变化事件
void CartWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // 1. 标签栏宽度强制等于tabWidget总宽度，铺满
    if(tabWidget && tabWidget->tabBar())
    {
        tabWidget->tabBar()->setFixedWidth(tabWidget->width());
        tabWidget->tabBar()->updateGeometry();
    }
    // 2. 生成圆角遮罩，实现整体24px大圆角窗口
    QPainterPath roundedPath;
    roundedPath.addRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 24, 24);
    setMask(QRegion(roundedPath.toFillPolygon().toPolygon()));
}


//showEvent 窗口显示事件
void CartWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // 同样生成圆角遮罩
    QPainterPath roundedPath;
    roundedPath.addRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 24, 24);
    setMask(QRegion(roundedPath.toFillPolygon().toPolygon()));
    //刷新Tab标签宽度
    if(tabWidget && tabWidget->tabBar())
    {
        tabWidget->tabBar()->setFixedWidth(tabWidget->width());
        tabWidget->tabBar()->updateGeometry();
    }
}

//paintEvent 自定义绘制窗口背景
void CartWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    //根据明暗模式切换边框+背景色
    painter.setPen(QPen(QColor(darkMode?"#4a5666":"#fed7aa"), 1));
    painter.setBrush(QColor(darkMode?"#1d2530":"#fffaf0"));
    // 绘制圆角底层背景
    painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 24, 24);
}


//切换明暗主题
void CartWidget::setDarkMode(bool enabled)
{
    darkMode=enabled;
    if(enabled){
        // 深色模式：修改所有分页背景、标题、Tab样式、列表样式、总价文字
        for(int i=0;i<tabWidget->count();++i)tabWidget->widget(i)->setStyleSheet("background:#242e3a;");
        ui->titleLabel->setStyleSheet("font-size:25px;font-weight:800;color:white;padding:8px;");
        tabWidget->setStyleSheet("QTabWidget{background:#1d2530;} QTabWidget::pane{border:1px solid #4a5666;background:#242e3a;} QTabBar::tab{padding:13px;background:#2a3542;color:#b8c2cf;border:1px solid #4a5666;font-size:17px;} QTabBar::tab:selected{background:#465568;color:white;font-weight:700;}");
        // 两个菜品列表统一深色样式
        const QString lists="QListWidget{background:#202936;color:#e8edf3;border:1px solid #4a5666;border-radius:14px;padding:10px;} QListWidget::item{border-bottom:1px solid #3c4857;}";
        ui->cartListWidget->setStyleSheet(lists);orderedListWidget->setStyleSheet(lists);
        ui->totalLabel->setStyleSheet("font-size:20px;font-weight:bold;color:#ffb15c;padding:8px;");
    }else{
         // 浅色模式：恢复餐厅暖橙浅黄配色
        for(int i=0;i<tabWidget->count();++i)tabWidget->widget(i)->setStyleSheet("background:#fffaf0;");
        ui->titleLabel->setStyleSheet("font-size:25px;font-weight:800;color:#9a4312;padding:8px;");
        tabWidget->setStyleSheet("QTabWidget{background:#fffaf0;} QTabWidget::pane{border:1px solid #fed7aa;background:#fffaf0;} QTabBar::tab{padding:13px;background:#ffedd5;color:#9a4312;border:1px solid #fed7aa;font-size:17px;} QTabBar::tab:selected{background:#ff8c42;color:white;font-weight:700;}");
        const QString lists="QListWidget{background:white;color:#374151;border:1px solid #fed7aa;border-radius:14px;padding:10px;} QListWidget::item{border-bottom:1px solid #ffedd5;}";
        ui->cartListWidget->setStyleSheet(lists);orderedListWidget->setStyleSheet(lists);
        ui->totalLabel->setStyleSheet("font-size:20px;font-weight:bold;color:#f97316;padding:8px;");
    }
    update();   //触发重绘paintEvent刷新背景
    refreshCart();
}

void CartWidget::setSubmitting(bool value)
{
    submitting = value;
    ui->submitButton->setText(value ? "订单处理中..." : "提交订单");
    updateButtons(); //更新按钮启用/禁用状态
}

//加菜
void CartWidget::addDish(const Dish &dish)
{
    if(submitting)     //如果正在提交订单，不允许修改
        return;
    //菜品第一次加入
    if(!m_cart.contains(dish.id))
        m_cart.insert(dish.id, CartItem{dish, 1});
    //已经存在则数量加一
    else if(m_cart[dish.id].count < m_cart[dish.id].dish.stock)
        m_cart[dish.id].count++;
    //通知其他界面数量发生变化
    emit quantityChanged(dish.id, m_cart[dish.id].count);
    refreshCart();
}

//减菜
void CartWidget::decreaseDish(int dishId)
{
    changeQuantity(dishId, -1);
}

//查询当前购物车中点了几份该菜品
int CartWidget::quantityForDish(int dishId) const
{
    return m_cart.contains(dishId) ? m_cart.value(dishId).count : 0;
}

//统一修改菜品份数
void CartWidget::changeQuantity(int dishId, int delta)
{
    if(submitting || !m_cart.contains(dishId))
        return;
    CartItem &item = m_cart[dishId];
     //限制范围：最小0，最大菜品库存
    const int newCount = qBound(0, item.count + delta, item.dish.stock);
    if(newCount == 0)
        m_cart.remove(dishId);  //份数0，直接移除菜品
    else
        item.count = newCount;  //更新份数
    emit quantityChanged(dishId, newCount);
    refreshCart();
}

//刷新购物车，整个购物车显示的核心
void CartWidget::refreshCart()
{
    ui->cartListWidget->clear();
    //每一道点的菜品，都会单独生成一行完整条目
    for(const CartItem &item : m_cart)
    {
        //计算单品小计=份数*单价
        const double subtotal = item.dish.price * item.count;
        // 创建空列表条目，存储菜品 ID
        QListWidgetItem *listItem = new QListWidgetItem;
        listItem->setData(Qt::UserRole, item.dish.id);  //UserRole 存菜品 ID，后续删除 / 操作可以拿到；
        listItem->setSizeHint(QSize(0, 92));
        ui->cartListWidget->addItem(listItem);

        QWidget *row = new QWidget(ui->cartListWidget);
        //明暗模式文字颜色切换
        row->setStyleSheet(darkMode?"QWidget{background:transparent;} QLabel{color:#eef2f7;}":"QWidget{background:transparent;} QLabel{color:#374151;}");
        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(12, 7, 10, 7);

        //左侧菜品图片占位框
        QLabel *thumbnail = new QLabel("加载中", row);
        thumbnail->setFixedSize(82, 72);
        thumbnail->setAlignment(Qt::AlignCenter);
        thumbnail->setStyleSheet("background:#fff7ed;border:1px solid #fed7aa;border-radius:9px;color:#9ca3af;");

        //中间菜品文字信息
        QLabel *description = new QLabel(
            QString("<b>%1</b><br><span style='color:#777'>单价 ￥%2</span><br>"
                    "<span style='color:#e74c3c;font-weight:bold'>小计 ￥%3</span>")
                .arg(item.dish.name)
                .arg(item.dish.price, 0, 'f', 2).arg(subtotal, 0, 'f', 2), row);
        description->setTextFormat(Qt::RichText);
        description->setStyleSheet("font-size:15px;");
        rowLayout->addWidget(thumbnail);
        rowLayout->addWidget(description, 1);

        //右侧加减按钮 + 数量数字
        QPushButton *minus = new QPushButton("−", row);
        QLabel *quantity = new QLabel(QString::number(item.count), row);
        QPushButton *plus = new QPushButton("+", row);
        quantity->setAlignment(Qt::AlignCenter);
        quantity->setFixedWidth(28);
        quantity->setStyleSheet("font-size:17px;font-weight:bold;");
        // 按钮样式 + 可用性控制
        for(QPushButton *button : {minus, plus})
        {
            button->setFixedSize(32, 32);
            button->setStyleSheet("QPushButton{background:#f97316;color:white;border:none;border-radius:9px;font-size:19px;font-weight:bold;} QPushButton:hover{background:#ea580c;} QPushButton:disabled{background:#e5e7eb;color:#9ca3af;}");
        }
        // 加号限制：库存充足 + 不在提交订单状态才能点
        plus->setEnabled(item.count < item.dish.stock && !submitting);
        // 减号只要不在提交就能点
        minus->setEnabled(!submitting);
        //加减按钮绑定事件
        const int dishId = item.dish.id;
        connect(minus, &QPushButton::clicked, this, [this, dishId](){ changeQuantity(dishId, -1); });
        connect(plus, &QPushButton::clicked, this, [this, dishId](){ changeQuantity(dishId, 1); });
        //把所有控件横向放入布局，挂载到列表条目
        rowLayout->addWidget(minus);
        rowLayout->addWidget(quantity);
        rowLayout->addWidget(plus);
        ui->cartListWidget->setItemWidget(listItem, row);

        //图片加载逻辑（缓存 + 异步网络请求）
        if(imageCache.contains(item.dish.id))
        {
            thumbnail->setPixmap(imageCache[item.dish.id].scaled(
                thumbnail->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else
        {
            // 缓存没有，发起网络请求下载菜品图片
            //安全弱指针，防止一个极端 bug：图片还没下载完，用户关闭购物车、销毁了 thumbnail 控件，回调里访问空指针直接崩溃。
            //用QPointer判断控件是否还存在，不存在就不执行绘图。
            QPointer<QLabel> safeThumbnail(thumbnail);
            //请求完成回调
            QNetworkReply *reply = imageManager->get(QNetworkRequest(
                    ServerConfig::imageUrl(item.dish.picture)));
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
     //统计总菜品份数，发送信号通知外部，同步到其他要修改的地方
    int itemCount = 0;
    for(const CartItem &cartItem : m_cart)
        itemCount += cartItem.count;
    emit totalChanged(totalPrice(), itemCount);
    updateButtons();
}

//切换到「已下单」标签页时执行：更新已下单的
void CartWidget::refreshOrderedOrder()
{
    //清空「已下单」分页里旧的菜品列表，删掉之前所有条目。
    orderedListWidget->clear();
    orderedListWidget->addItem("正在读取已下单菜品…");
    orderedSummaryLabel->setText("加载中…");
    //切换到「已下单」标签页时执行：
    orderNetwork->getOrderDetail(tableId);
}


//后端 JSON 数据渲染，没有加减操作按钮，只做展示，不允许修改；
void CartWidget::renderOrderedOrder(bool success, const QJsonObject &data)
{
    //清空就列表
    orderedListWidget->clear();
    // 取出后端返回的菜品数组
    const QJsonArray items = data.value("items").toArray();
    // 请求失败 / 订单没有菜品
    if(!success || items.isEmpty())
    {
        orderedListWidget->addItem("当前桌台暂无已下单菜品");
        orderedSummaryLabel->setText("已下单合计：￥0.00");
        return;
    }

    // 循环每条已下单菜品JSON
    for(const QJsonValue &value : items)
    {
        const QJsonObject item = value.toObject();
        const int count = item.value("count").toInt();  //点餐份数
        const double price = item.value("price").toDouble();  //点餐单价

        //--------------UI-------------
        // 创建列表条目，固定行高86px
        QListWidgetItem *listItem = new QListWidgetItem;
        listItem->setSizeHint(QSize(0, 86));
        orderedListWidget->addItem(listItem);
        // 单行整体容器，水平布局：图片 + 菜品信息 + 单品总价
        QWidget *row = new QWidget(orderedListWidget);
        //根据明暗模式切换文字颜色
        row->setStyleSheet(darkMode?"QWidget{background:transparent;} QLabel{color:#eef2f7;}":"QWidget{background:transparent;} QLabel{color:#374151;}");
        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(12, 7, 10, 7);

        // 左侧菜品图片框
        QLabel *thumbnail = new QLabel("图片", row);
        thumbnail->setFixedSize(76, 66);
        thumbnail->setAlignment(Qt::AlignCenter);
        thumbnail->setStyleSheet("background:#fff7ed;border:1px solid #fed7aa;border-radius:8px;color:#9ca3af;");

        // 中间富文本：加粗菜名 + 灰色小字 单价 × 份数
        QLabel *name = new QLabel(QString("<b>%1</b><br><span style='color:#64748b'>￥%2 × %3</span>")
            .arg(item.value("dishName").toString()).arg(price, 0, 'f', 2).arg(count), row);
        name->setTextFormat(Qt::RichText);

        //右侧红色加粗：该菜品小计总价
        QLabel *subtotal = new QLabel(QString("￥%1").arg(price * count, 0, 'f', 2), row);
        subtotal->setStyleSheet("font-size:17px;font-weight:700;color:#e74c3c;");
        // 横向布局组装一行
        rowLayout->addWidget(thumbnail);
        rowLayout->addWidget(name, 1); // 文字区域自动拉伸占空间
        rowLayout->addWidget(subtotal);
        // 把自定义行挂载到列表条目上
        orderedListWidget->setItemWidget(listItem, row);


        //--------------异步加载菜品-----------------
        //异步加载菜品图片（无缓存，不缓存，每次切换页面重新下载）
        const QString picture = item.value("picture").toString();
        if(!picture.isEmpty())
        {
            QPointer<QLabel> safeThumbnail(thumbnail);
            //请求后端图片接口
        QNetworkReply *reply = imageManager->get(QNetworkRequest(ServerConfig::imageUrl(picture)));
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
    // 底部更新整张订单总金额
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

//对外提供接口，返回当前购物车内所有菜品的列表。
QList<CartItem> CartWidget::cartItems() const
{
    return m_cart.values();
}

//计算购物车总价
double CartWidget::totalPrice() const
{
    double total = 0;
    for(const CartItem &item : m_cart)
        total += item.dish.price * item.count;
    return total;
}

//删除购物车选中的一道菜
void CartWidget::removeCurrentDish()
{
    QListWidgetItem *current = ui->cartListWidget->currentItem();
    if(!current)
        return;
    //取出之前存在UserRole里的菜品ID，从内存购物车删除
    m_cart.remove(current->data(Qt::UserRole).toInt());
    // 发送信号：该菜品数量变为0
    emit quantityChanged(current->data(Qt::UserRole).toInt(), 0);
    refreshCart();
}

//清空购物车
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

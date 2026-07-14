#include "dishwidget.h"
#include "ui_dishwidget.h"
#include "dishcard.h"
#include "paywidget.h"
#include "orderdetailwidget.h"

#include <QMessageBox>
#include <QDateTime>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrinterInfo>
#include <QTextDocument>
#include <algorithm>
#include <QSet>
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QColor>
#include <QResizeEvent>
#include <QEasingCurve>

DishWidget::DishWidget(int tableId,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DishWidget)
    ,currentTableId(tableId)
    ,pendingOrderId(0)
    ,pendingMoney(0)
{
    ui->setupUi(this);
    //-------------顶部区域--------------
    //显示当前桌号
    titleLabel = new QLabel(QString("%1号桌点餐").arg(currentTableId),this);

    titleLabel->setText(QString("%1号桌 · 点餐").arg(currentTableId));
    titleLabel->setMinimumHeight(54);
    titleLabel->setStyleSheet(
        "QLabel{font-size:27px;font-weight:700;color:#1f2d3d;"
        // "background:#eef6ff;border:1px solid #cfe5ff;border-radius:12px;"
        "padding:8px 20px;}"
    );

    //换桌按钮
    changeTableButton = new QPushButton("换桌",this);
    //返回按钮
    backTableButton = new QPushButton("返回桌台",this);
    changeTableButton->setText("换桌");
    backTableButton->setText("返回桌台");
    changeTableButton->setMinimumSize(108,48);
    backTableButton->setMinimumSize(128,48);
    for(QPushButton *button : {changeTableButton, backTableButton})
    {
        button->setCursor(Qt::PointingHandCursor);
        button->setFont(QFont("Microsoft YaHei", 11, QFont::DemiBold));
    }
    changeTableButton->setStyleSheet(
        "QPushButton{background:#fff7e6;color:#d97706;border:1px solid #f6c56f;border-radius:10px;padding:8px 16px;}"
        "QPushButton:hover{background:#ffedd5;border-color:#f59e0b;}"
        "QPushButton:pressed{background:#fed7aa;padding-top:10px;}"
    );
    backTableButton->setStyleSheet(
        "QPushButton{background:#f4f6f8;color:#435466;border:1px solid #cbd5df;border-radius:10px;padding:8px 16px;}"
        "QPushButton:hover{background:#e8edf2;border-color:#94a3b2;}"
        "QPushButton:pressed{background:#dce3e9;padding-top:10px;}"
    );

    QWidget *topBar = new QWidget(this);
    topBar->setObjectName("topBar");
    topBar->setStyleSheet(
        "QWidget#topBar{background:white;border:1px solid #e1e8ef;border-radius:15px;}"
    );
    QGraphicsDropShadowEffect *topShadow = new QGraphicsDropShadowEffect(topBar);
    topShadow->setBlurRadius(20);
    topShadow->setOffset(0, 4);
    topShadow->setColor(QColor(31, 45, 61, 35));
    topBar->setGraphicsEffect(topShadow);
    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(14, 12, 14, 12);
    topLayout->setSpacing(12);

    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(changeTableButton);
    topLayout->addWidget(backTableButton);


    //链接返回桌台按钮
    connect(backTableButton,&QPushButton::clicked,this,[this](){
        emit backToTable();
    });


    //换桌按钮
    connect(changeTableButton,&QPushButton::clicked,this,[this](){
        QMessageBox box(
            QMessageBox::Question,
            "换桌",
            "换桌会清空当前操作，是否继续?",
            QMessageBox::Yes |
                QMessageBox::No,
            this
            );


        if(box.exec()==QMessageBox::Yes)
        {
            emit backToTable();
        }
            });

    //-------------菜品区域--------------

    //滚动区域
    scrollArea =new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    //真正放卡片的容器

    container =new QWidget();

    layout =new QGridLayout(container);

    // container->setMinimumWidth(1000);

    layout->setSpacing(20);

    layout->setAlignment(Qt::AlignTop);

    scrollArea->setWidget(container);

    //----------------页面总布局-------------
    QVBoxLayout *mainLayout =new QVBoxLayout(this);

    mainLayout->addWidget(topBar);

    categoryWidget = new QWidget(this);
    categoryWidget->setObjectName("categoryRail");
    categoryWidget->setFixedWidth(150);
    categoryWidget->setStyleSheet("QWidget#categoryRail{background:#182433;border-radius:14px;}");
    categoryLayout = new QVBoxLayout(categoryWidget);
    categoryLayout->setContentsMargins(10, 14, 10, 14);
    categoryLayout->setSpacing(9);
    QHBoxLayout *orderingLayout = new QHBoxLayout;
    orderingLayout->setSpacing(14);
    orderingLayout->addWidget(categoryWidget);
    orderingLayout->addWidget(scrollArea, 1);
    mainLayout->addLayout(orderingLayout, 1);

    mainLayout->setContentsMargins(15,15,15,15);

    mainLayout->setSpacing(15);


    //---------------创建购物车窗口----------

    cartWidget = new CartWidget(currentTableId, this);
    cartBackdrop = new QPushButton(this);
    cartBackdrop->setFlat(true);
    cartBackdrop->setStyleSheet("QPushButton{background:rgba(15,23,42,105);border:none;}");
    cartBackdrop->hide();
    cartWidget->hide();
    cartWidget->setStyleSheet(
        "CartWidget{background:transparent;border:none;}"
    );
    cartAnimation = new QPropertyAnimation(cartWidget, "geometry", this);
    cartAnimation->setDuration(260);
    cartAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(cartBackdrop, &QPushButton::clicked, this, &DishWidget::closeCartDrawer);
    connect(cartWidget, &CartWidget::closeRequested, this, &DishWidget::closeCartDrawer);
    connect(cartAnimation, &QPropertyAnimation::finished, this, [this](){
        if(!cartDrawerOpen)
        {
            cartWidget->hide();
            cartBackdrop->hide();
            cartSummaryBar->raise();
        }
    });

    cartSummaryBar = new QWidget(this);
    cartSummaryBar->setObjectName("cartSummaryBar");
    cartSummaryBar->setStyleSheet(
        "QWidget#cartSummaryBar{background:#202b3c;border:1px solid #354157;border-radius:14px;}"
    );
    QHBoxLayout *summaryLayout = new QHBoxLayout(cartSummaryBar);
    summaryLayout->setContentsMargins(18, 8, 8, 8);
    cartCountLabel = new QLabel("尚未选菜", cartSummaryBar);
    cartCountLabel->setStyleSheet("color:#cbd5e1;font-size:14px;");
    cartTotalLabel = new QLabel("￥0.00", cartSummaryBar);
    cartTotalLabel->setStyleSheet("color:white;font-size:24px;font-weight:800;");
    cartConfirmButton = new QPushButton("确认菜品", cartSummaryBar);
    cartConfirmButton->setMinimumSize(132, 48);
    cartConfirmButton->setCursor(Qt::PointingHandCursor);
    cartConfirmButton->setStyleSheet(
        "QPushButton{background:#e64b35;color:white;border:none;border-radius:10px;font-size:17px;font-weight:700;}"
        "QPushButton:hover{background:#f05a43;} QPushButton:pressed{background:#c93e2b;}"
    );
    summaryLayout->addWidget(cartCountLabel);
    summaryLayout->addWidget(cartTotalLabel);
    summaryLayout->addStretch();
    summaryLayout->addWidget(cartConfirmButton);
    connect(cartConfirmButton, &QPushButton::clicked, this, &DishWidget::openCartDrawer);
    connect(cartWidget, &CartWidget::totalChanged, this, [this](double total, int count){
        cartTotalLabel->setText(QString("￥%1").arg(total, 0, 'f', 2));
        cartCountLabel->setText(count > 0 ? QString("已选 %1 份").arg(count) : "尚未选菜");
    });



    //--------------网络对象----------------
    network =new NetworkManager(this);

    connect(cartWidget,&CartWidget::submitOrderRequested,
            this,
            [this](const QList <CartItem> &items ){

                network->submitOrder(currentTableId,items);
                // // qDebug()<<"准备提交订单";

                // for(const CartItem &item : items)
                // {
                //     qDebug()<<"菜品："<<item.dish.name
                //              <<"dishId:"<<item.dish.id
                //              <<"数量:"<<item.count;
                // }


    });

    connect(network,&NetworkManager::orderSubmitted,this,[this](bool success,const QString &message){
        cartWidget->setSubmitting(false);
        if(success)
        {
            QMessageBox::information(this,"成功","订单提交成功");

            //清空购物车
            cartWidget->clearCart();
            //关闭购物车窗口
            closeCartDrawer();
            //重新刷新菜品
            network->getDishList();
        }
        else
        {
            QMessageBox::warning(this,"失败",message);
        }


    });


    //收到菜品
    connect(network,
            &NetworkManager::dishListReceived,
            this,
            [this](QList<Dish> &list){

                dishes = list;
                renderDishes();
                return;

                //清空旧的菜品卡片
                while(layout->count()>0)
                {
                    QLayoutItem *item = layout->takeAt(0);
                    if(item->widget())
                    {
                        item->widget()->deleteLater();
                    }
                    delete item;
                }

                qDebug()<<"菜品数量:"<<list.size();
                int row=0;
                int col=0;
                for(const Dish &dish:list)
                {
                    DishCard *card =new DishCard(dish,container);

                    connect(card,&DishCard::increaseDish,this,[this](const Dish &dish){
                        // 真正加入购物车
                        cartWidget->addDish(dish);
                        qDebug()<<"已加入购物车:"<<dish.name;

                    });
                    layout->addWidget(card,row,col);
                    col++;
                    if(col==3)
                    {
                        col=0;
                        row++;
                    }

                }
                // 下一步：
                // 调用 POST /order/submit
            });

    connect(network, &NetworkManager::dishCategoriesReceived, this,
            [this](const QMap<int, QString> &categories){
        while(QLayoutItem *item = categoryLayout->takeAt(0))
        {
            delete item->widget();
            delete item;
        }

        auto addCategoryButton = [this](int categoryId, const QString &name){
            QPushButton *button = new QPushButton(name, categoryWidget);
            button->setCheckable(true);
            button->setChecked(selectedCategory == categoryId);
            button->setMinimumSize(128, 48);
            button->setStyleSheet(
                "QPushButton{background:transparent;color:#d7e0ea;border:none;border-radius:10px;padding:8px 12px;font-size:16px;text-align:left;}"
                "QPushButton:hover{background:#26364a;color:white;}"
                "QPushButton:checked{background:#f4d66d;color:#202938;font-weight:bold;}"
            );
            connect(button, &QPushButton::clicked, this, [this, categoryId](){
                selectedCategory = categoryId;
                for(QPushButton *other : categoryWidget->findChildren<QPushButton *>())
                    other->setChecked(other->property("categoryId").toInt() == categoryId);
                renderDishes();
            });
            button->setProperty("categoryId", categoryId);
            categoryLayout->addWidget(button);
        };

        addCategoryButton(-1, "🔥 热销榜");
        addCategoryButton(0, "全部");
        for(auto it = categories.cbegin(); it != categories.cend(); ++it)
            addCategoryButton(it.key(), it.value());
        categoryLayout->addStretch();
    });



    //支付的连接购物车按钮
    connect(cartWidget,&CartWidget::checkoutRequested,this,[this](){
        qDebug()<<"点击结账 当前桌:"<<currentTableId;
        //查询该桌未支付订单
        if(checkoutInProgress)
            return;
        if(QMessageBox::question(this, "确认停止加菜",
               "进入待结账后将不能继续加菜，确定要结账吗？",
               QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
            return;

        checkoutInProgress = true;
        cartWidget->setSubmitting(true);
        network->checkoutTable(currentTableId);
    });

    //接受金额
    connect(network,&NetworkManager::unpaidOrderReceived
            ,this,[this](bool hasOrder,int orderId,double money){
        if(!hasOrder)
        {
            QMessageBox::warning(this,"提示","当前桌没有消费记录");
            return;
        }

        //确认金额
        QMessageBox box(QMessageBox::Question,
                        "确认结账",
                        QString("本桌消费金额：%1元\n是否结账？")
                            .arg(money),
                        QMessageBox::Yes|
                        QMessageBox::No,
                        this
                        );

        if(box.exec()!=QMessageBox::Yes)
        {
            return;
        }

        //保存支付信息
        pendingOrderId = orderId;
        pendingMoney = money;


        //用餐中1->待结账2

        network->updateTableStatus(
            currentTableId,
            2
            );
    });

        connect(network,&NetworkManager::tableStatusUpdated,
                this,[this](bool success){
            if(!success)
            {
                QMessageBox::warning(this,"错误","修改桌台状态失败");
                return;
            }

            //状态修改成功

            //打开支付窗口        int pendingOrderId;        double pendingMoney;
            openPaymentWindow();

        });




        // //打开支付界面
        // PayWidget *pay = new PayWidget(orderId,money,this);
        // pay->setAttribute(Qt::WA_DeleteOnClose);

        // pay->setWindowTitle("订单支付");

        // pay->resize(450,350);
        // pay->show();
        // pay->raise();
        // pay->activateWindow();

    //     //支付成功
    //     connect(pay,
    //             &PayWidget::paySuccess,
    //             this,
    //             [this](){

    //             QMessageBox::information(this,"支付成功","订单支付完成");
    //              });
    //                 //这里不用再改状态了，因为后端springboot已经修改了
    //     }
    // );


    //查看订单详情
    connect(network, &NetworkManager::checkoutReady, this,
            [this](bool success, int orderId, double money, const QString &message){
        checkoutInProgress = false;
        paymentOpening = false;
        cartWidget->setSubmitting(false);
        if(!success)
        {
            QMessageBox::warning(this, "暂时不能结账", message);
            return;
        }

        pendingOrderId = orderId;
        pendingMoney = money;
        closeCartDrawer();
        QMessageBox::information(this, "已进入待结账",
            QString("本桌消费金额：￥%1\n请继续选择支付方式。").arg(money, 0, 'f', 2));
        openPaymentWindow();
    });

    connect(cartWidget,&CartWidget::detailRequested,
            this,[this](){
        if(activeDetailWidget)
        {
            activeDetailWidget->refresh();
            activeDetailWidget->showNormal();
            activeDetailWidget->raise();
            activeDetailWidget->activateWindow();
            return;
        }

        activeDetailWidget = new OrderDetailWidget(currentTableId, this);
        OrderDetailWidget *detail = activeDetailWidget.data();

        detail->setWindowFlag(Qt::Window);
        detail->setAttribute(Qt::WA_DeleteOnClose);
        detail->setWindowTitle("订单详情");
        detail->move(window()->frameGeometry().center() - detail->rect().center());

        connect(detail, &QObject::destroyed, this, [this](){
            activeDetailWidget = nullptr;
        });

        connect(detail,&OrderDetailWidget::payRequested,this,[this](int orderId,double money){

            if(paymentOpening || activePayWidget)
            {
                if(activePayWidget)
                {
                    activePayWidget->show();
                    activePayWidget->raise();
                    activePayWidget->activateWindow();
                }
                return;
            }

            Q_UNUSED(orderId);
            Q_UNUSED(money);
            if(checkoutInProgress)
                return;

            checkoutInProgress = true;
            paymentOpening = true;


            //检查订单状态
            network->checkoutTable(currentTableId);

            // PayWidget *pay = new PayWidget(orderId,money,this);
            // pay->setAttribute(Qt::WA_DeleteOnClose);

            // connect(pay,&PayWidget::paySuccess,this,[this,detail](){
            //             QMessageBox::information( this,"成功","支付完成");
            //             //关闭订单详情窗口
            //             detail->close();
            //         });
            // pay->show();

        });
        detail->show();

    });


    connect(network,
            &NetworkManager::orderCanPay,
            this,
            [this](bool canPay,QString msg){

                if(!canPay)
                {
                    paymentOpening = false;
                    QMessageBox::information(
                        this,
                        "提示",
                        msg
                        );

                    return;
                }

                openPaymentWindow();

            });



    network->getDishCategories();
    network->getDishList();
}

void DishWidget::renderDishes()
{
    while(layout->count() > 0)
    {
        QLayoutItem *item = layout->takeAt(0);
        delete item->widget();
        delete item;
    }

    QList<Dish> ranking = dishes;
    std::sort(ranking.begin(), ranking.end(), [](const Dish &left, const Dish &right){
        return left.soldCount > right.soldCount;
    });

    QSet<int> hotDishIds;
    for(const Dish &dish : ranking)
    {
        if(dish.soldCount <= 0 || hotDishIds.size() >= 10)
            break;
        hotDishIds.insert(dish.id);
    }

    int visibleIndex = 0;
    auto addCard = [this, &visibleIndex](const Dish &dish, bool recommended){
        DishCard *card = new DishCard(dish, container, recommended);
        card->setQuantity(cartWidget->quantityForDish(dish.id));
        connect(card, &DishCard::increaseDish, this, [this](const Dish &selectedDish){
            cartWidget->addDish(selectedDish);
        });
        connect(card, &DishCard::decreaseDish, cartWidget, &CartWidget::decreaseDish);
        connect(cartWidget, &CartWidget::quantityChanged, card,
                [card, dishId = dish.id](int changedDishId, int quantity){
            if(changedDishId == dishId)
                card->setQuantity(quantity);
        });
        layout->addWidget(card, visibleIndex / 3, visibleIndex % 3, Qt::AlignTop | Qt::AlignHCenter);
        ++visibleIndex;
    };

    if(selectedCategory == -1)
    {
        int shown = 0;
        for(const Dish &dish : ranking)
        {
            if(dish.soldCount <= 0 || shown >= 10)
                break;
            addCard(dish, true);
            ++shown;
        }
        return;
    }

    // “全部”页面先复制前三名作为本店推荐，后面仍保留完整菜品列表。
    if(selectedCategory == 0)
    {
        int recommendedCount = 0;
        for(const Dish &dish : ranking)
        {
            if(dish.soldCount <= 0 || recommendedCount >= 3)
                break;
            addCard(dish, true);
            ++recommendedCount;
        }
    }

    for(const Dish &dish : dishes)
    {
        if(selectedCategory > 0 && dish.catId != selectedCategory)
            continue;
        addCard(dish, hotDishIds.contains(dish.id));
    }
}

QRect DishWidget::cartDrawerGeometry(bool opened) const
{
    const int drawerWidth = qMin(430, width() - 40);
    const int drawerTop = 92;
    const int drawerHeight = qMax(440, height() - drawerTop - 18);
    const int x = opened ? width() - drawerWidth - 28 : width() + 8;
    return QRect(x, drawerTop, drawerWidth, drawerHeight);
}

void DishWidget::openCartDrawer()
{
    cartDrawerOpen = true;
    cartWidget->refreshOrderedOrder();
    cartBackdrop->setGeometry(rect());
    cartBackdrop->show();
    cartBackdrop->raise();
    cartWidget->setGeometry(cartDrawerGeometry(false));
    cartWidget->show();
    cartWidget->raise();
    cartAnimation->stop();
    cartAnimation->setStartValue(cartDrawerGeometry(false));
    cartAnimation->setEndValue(cartDrawerGeometry(true));
    cartAnimation->start();
}

void DishWidget::closeCartDrawer()
{
    if(!cartDrawerOpen)
        return;
    cartDrawerOpen = false;
    cartAnimation->stop();
    cartAnimation->setStartValue(cartWidget->geometry());
    cartAnimation->setEndValue(cartDrawerGeometry(false));
    cartAnimation->start();
}

void DishWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if(cartSummaryBar)
    {
        const int barWidth = qMin(430, width() - 40);
        cartSummaryBar->setGeometry(width() - barWidth - 28, height() - 82, barWidth, 64);
        if(!cartDrawerOpen)
            cartSummaryBar->raise();
    }
    if(!cartBackdrop || !cartWidget)
        return;
    cartBackdrop->setGeometry(rect());
    if(cartDrawerOpen && cartAnimation->state() != QAbstractAnimation::Running)
        cartWidget->setGeometry(cartDrawerGeometry(true));
}

void DishWidget::openPaymentWindow()
{
    paymentOpening = false;

    if(activePayWidget)
    {
        activePayWidget->show();
        activePayWidget->raise();
        activePayWidget->activateWindow();
        return;
    }

    if(pendingOrderId <= 0)
    {
        return;
    }

    activePayWidget = new PayWidget(pendingOrderId, pendingMoney, this);
    activePayWidget->setAttribute(Qt::WA_DeleteOnClose);
    activePayWidget->setWindowFlag(Qt::Window);

    connect(activePayWidget.data(), &PayWidget::paySuccess, this, [this](int payType){
        const int paidOrderId = pendingOrderId;
        const double paidMoney = pendingMoney;
        pendingOrderId = 0;
        pendingMoney = 0;
        if(activeDetailWidget)
            activeDetailWidget->refresh();
        network->getDishList();
        QMessageBox::information(this, "成功", "订单支付完成");
        if(QMessageBox::question(this, "打印小票", "是否现在打印结账小票？") == QMessageBox::Yes)
            printReceipt(paidOrderId, paidMoney, payType);
    });

    connect(activePayWidget.data(), &QObject::destroyed, this, [this](){
        activePayWidget = nullptr;
        paymentOpening = false;
    });

    activePayWidget->show();
    activePayWidget->raise();
    activePayWidget->activateWindow();
}

void DishWidget::printReceipt(int orderId, double money, int payType)
{
    if(QPrinterInfo::availablePrinters().isEmpty())
    {
        QMessageBox::information(this, "没有打印机", "系统中没有可用打印机，请先安装或配置打印机。");
        return;
    }

    QPrinter printer(QPrinter::HighResolution);
    printer.setDocName(QString("订单%1结账小票").arg(orderId));
    QPrintDialog dialog(&printer, this);
    dialog.setWindowTitle("打印结账小票");
    if(dialog.exec() != QDialog::Accepted)
        return;

    const QString channel = payType == 1 ? "微信支付（模拟）" : "支付宝（模拟）";
    QTextDocument receipt;
    receipt.setHtml(QString(
        "<div style='font-family:Microsoft YaHei;text-align:center;'>"
        "<h2>餐厅结账小票</h2><hr>"
        "<table width='100%' style='font-size:14px;text-align:left;'>"
        "<tr><td>桌台</td><td align='right'>%1号桌</td></tr>"
        "<tr><td>订单号</td><td align='right'>%2</td></tr>"
        "<tr><td>支付方式</td><td align='right'>%3</td></tr>"
        "<tr><td>支付时间</td><td align='right'>%4</td></tr>"
        "</table><hr>"
        "<h2 style='text-align:right;'>实付：￥%5</h2>"
        "<p>谢谢惠顾，欢迎再次光临！</p>"
        "</div>"
    ).arg(currentTableId).arg(orderId).arg(channel)
     .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))
     .arg(money, 0, 'f', 2));
    receipt.print(&printer);
}

//实现刷新标题
void DishWidget::changeTable(int tableId)
{
    currentTableId = tableId;
    titleLabel->setText(QString("%1号桌 · 点餐").arg(currentTableId));
}



DishWidget::~DishWidget()
{
    delete ui;
}

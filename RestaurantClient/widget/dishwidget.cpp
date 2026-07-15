#include "dishwidget.h"
#include "ui_dishwidget.h"
#include "dishcard.h"
#include "paywidget.h"
#include "orderdetailwidget.h"
#include "blindboxdialog.h"

#include <QMessageBox>
#include <QDateTime>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrinterInfo>
#include <QTextDocument>
#include <QPdfWriter>
#include <QPageSize>
#include <QPageLayout>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <algorithm>
#include <QSet>
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QColor>
#include <QResizeEvent>
#include <QEasingCurve>
#include <QScrollBar>
#include <QTimer>
#include <QStringList>

namespace {
int categoryPriority(const QString &name)
{
    const QString normalized = name.trimmed();
    if(normalized.contains("鱼锅")) return 0;
    if(normalized.contains("热菜")) return 1;
    if(normalized.contains("甜品") || normalized.contains("甜点")) return 2;
    if(normalized.contains("饮料") || normalized.contains("饮品")) return 3;
    return 100;
}

QString categoryDisplayName(const QString &name)
{
    const int priority = categoryPriority(name);
    if(priority == 0) return "鱼锅";
    if(priority == 1) return "热菜";
    if(priority == 2) return "甜品";
    if(priority == 3) return "饮料";
    return name;
}
}

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
        "QLabel{font-size:25px;font-weight:800;color:#ea6a20;"
        // "background:#eef6ff;border:1px solid #cfe5ff;border-radius:12px;"
        "padding:8px 20px;}"
    );

    //换桌按钮
    changeTableButton = new QPushButton("换桌",this);
    //返回按钮
    backTableButton = new QPushButton("返回桌台",this);
    themeButton = new QPushButton("夜间模式",this);
    changeTableButton->setText("换桌");
    backTableButton->setText("返回桌台");
    changeTableButton->setMinimumSize(108,48);
    backTableButton->setMinimumSize(128,48);
    themeButton->setMinimumWidth(128);
    for(QPushButton *button : {changeTableButton, backTableButton, themeButton})
    {
        button->setFixedHeight(48);
        button->setCursor(Qt::PointingHandCursor);
        button->setFont(QFont("Microsoft YaHei", 11, QFont::DemiBold));
    }
    changeTableButton->setStyleSheet(
        "QPushButton{background:#ffd166;color:#9a4312;border:none;border-radius:12px;padding:8px 18px;}"
        "QPushButton:hover{background:#ffca45;}"
        "QPushButton:pressed{background:#fed7aa;padding-top:10px;}"
    );
    backTableButton->setStyleSheet(
        "QPushButton{background:#fff7ed;color:#ea6a20;border:1px solid #fed7aa;border-radius:12px;padding:8px 18px;}"
        "QPushButton:hover{background:#ffedd5;border-color:#fb923c;}"
        "QPushButton:pressed{background:#dce3e9;padding-top:10px;}"
    );

    QWidget *topBar = new QWidget(this);
    topBar->setAttribute(Qt::WA_StyledBackground,true);
    topBar->setObjectName("topBar");
    topBar->setStyleSheet(
        "QWidget#topBar{background:white;border:none;border-radius:20px;}"
    );
    QGraphicsDropShadowEffect *topShadow = new QGraphicsDropShadowEffect(topBar);
    topShadow->setBlurRadius(20);
    topShadow->setOffset(0, 4);
    topShadow->setColor(QColor(224, 122, 47, 35));
    topBar->setGraphicsEffect(topShadow);
    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(14, 12, 14, 12);
    topLayout->setSpacing(12);

    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(changeTableButton);
    topLayout->addWidget(backTableButton);
    topLayout->addWidget(themeButton);
    connect(themeButton,&QPushButton::clicked,this,[this](){darkMode=!darkMode;applyTheme();});


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
    container->setAttribute(Qt::WA_StyledBackground,true);

    layout =new QGridLayout(container);

    // container->setMinimumWidth(1000);

    layout->setSpacing(24);
    layout->setContentsMargins(24, 24, 24, 24);

    layout->setAlignment(Qt::AlignTop);

    scrollArea->setWidget(container);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea{background:#fffaf0;border:none;border-radius:20px;} QScrollBar:vertical{width:0px;}");
    container->setStyleSheet("background:#fffaf0;");

    //----------------页面总布局-------------
    QVBoxLayout *mainLayout =new QVBoxLayout(this);

    mainLayout->addWidget(topBar);

    categoryWidget = new QWidget(this);
    categoryWidget->setAttribute(Qt::WA_StyledBackground,true);
    categoryWidget->setObjectName("categoryRail");
    categoryWidget->setFixedWidth(160);
    categoryWidget->setStyleSheet("QWidget#categoryRail{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #e07a2f,stop:1 #ff8c42);border-radius:20px;}");
    categoryLayout = new QVBoxLayout(categoryWidget);
    categoryLayout->setContentsMargins(10, 14, 10, 14);
    categoryLayout->setSpacing(9);
    QHBoxLayout *orderingLayout = new QHBoxLayout;
    orderingLayout->setSpacing(14);
    orderingLayout->addWidget(categoryWidget);
    orderingLayout->addWidget(scrollArea, 1);
    mainLayout->addLayout(orderingLayout, 1);

    mainLayout->setContentsMargins(18,18,18,18);

    mainLayout->setSpacing(15);

    connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged,
            this, [this](){ updateCurrentCategory(); });


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
    cartSummaryBar->setAttribute(Qt::WA_StyledBackground,true);
    cartSummaryBar->setObjectName("cartSummaryBar");
    cartSummaryBar->setStyleSheet(
        "QWidget#cartSummaryBar{background:rgba(255,255,255,245);border:1px solid #ffedd5;border-radius:20px;}"
    );
    QHBoxLayout *summaryLayout = new QHBoxLayout(cartSummaryBar);
    summaryLayout->setContentsMargins(18, 8, 8, 8);
    cartCountLabel = new QLabel("尚未选菜", cartSummaryBar);
    cartCountLabel->setStyleSheet("color:#6b7280;font-size:15px;");
    cartTotalLabel = new QLabel("￥0.00", cartSummaryBar);
    cartTotalLabel->setStyleSheet("color:#f97316;font-size:25px;font-weight:900;");
    cartConfirmButton = new QPushButton("确认菜品", cartSummaryBar);
    cartConfirmButton->setMinimumSize(132, 48);
    cartConfirmButton->setCursor(Qt::PointingHandCursor);
    cartConfirmButton->setStyleSheet(
        "QPushButton{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #fb923c,stop:1 #ea580c);color:white;border:none;border-radius:16px;font-size:18px;font-weight:800;padding:0 28px;}"
        "QPushButton:hover{background:#f97316;} QPushButton:pressed{background:#ea580c;}"
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
    connect(network,&NetworkManager::tableListReceived,this,[this](const QList<DiningTable>&tables){
        for(const DiningTable &table:tables)if(table.id==currentTableId){currentTableName=table.tableName;break;}
        titleLabel->setText(QString("%1 · %2号桌 · 点餐").arg(currentTableName.isEmpty()?QString("桌台%1").arg(currentTableId):currentTableName).arg(currentTableId));
    });

    connect(network,&NetworkManager::orderReceiptReceived,this,[this](bool success,const QJsonObject &receipt,const QString &message){
        if(!success){QMessageBox::warning(this,"小票生成失败",message);return;}
        createPdfAndPrintReceipt(receipt,pendingReceiptPayType);
        network->getRewardChancesByOrder(receipt["orderId"].toInt());
    });
    connect(network,&NetworkManager::rewardChancesReceived,this,[this](bool success,int orderId,int chances,const QString &message){
        if(!success){QMessageBox::warning(this,"盲盒查询失败",message);return;}
        rewardOrderId=orderId;rewardChances=chances;if(chances>0)offerRewardDraw();
    });
    connect(network,&NetworkManager::rewardDrawFinished,this,[this](bool success,const QString &name,int remaining,const QString &message){
        if(!success){if(activeBlindBox)activeBlindBox->showError(message);else QMessageBox::warning(this,"抽奖失败",message);return;}
        rewardChances=remaining;
        if(activeBlindBox)activeBlindBox->showReward(name,remaining);
    });

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
        dishCategories = categories;
        while(QLayoutItem *item = categoryLayout->takeAt(0))
        {
            delete item->widget();
            delete item;
        }

        QStringList sectionNames{"热销"};
        QList<int> categoryIds = categories.keys();
        std::sort(categoryIds.begin(), categoryIds.end(), [&categories](int leftId, int rightId){
            const int leftPriority = categoryPriority(categories.value(leftId));
            const int rightPriority = categoryPriority(categories.value(rightId));
            return leftPriority == rightPriority ? leftId < rightId : leftPriority < rightPriority;
        });
        for(int categoryId : categoryIds)
            sectionNames.append(categoryDisplayName(categories.value(categoryId)));

        for(const QString &sectionName : sectionNames)
        {
            QPushButton *button = new QPushButton(sectionName, categoryWidget);
            button->setCheckable(true);
            button->setAutoExclusive(true);
            button->setProperty("sectionName", sectionName);
            button->setMinimumSize(128, 48);
            button->setCursor(Qt::PointingHandCursor);
            connect(button, &QPushButton::clicked, this, [this, sectionName](){
                for(const auto &section : sectionMarkers)
                {
                    if(section.second == sectionName && section.first)
                    {
                        // 所有分类标题统一停在视口顶部下方 12px；底部占位保证最后一项也能到达。
                        scrollArea->verticalScrollBar()->setValue(qMax(0, section.first->y() - 12));
                        break;
                    }
                }
            });
            categoryLayout->addWidget(button);
        }
        categoryLayout->addStretch();
        renderDishes();
        applyTheme();
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
    network->getTableList();
    applyTheme();
}

void DishWidget::renderDishes()
{
    sectionMarkers.clear();
    while(layout->count() > 0)
    {
        QLayoutItem *item = layout->takeAt(0);
        delete item->widget();
        delete item;
    }

    QList<Dish> ranking = dishes;
    std::sort(ranking.begin(), ranking.end(), [](const Dish &left, const Dish &right){
        if(left.soldCount != right.soldCount)
            return left.soldCount > right.soldCount;
        return left.id < right.id;
    });

    QSet<int> hotDishIds;
    for(const Dish &dish : ranking)
    {
        if(dish.soldCount <= 0 || hotDishIds.size() >= 5)
            break;
        hotDishIds.insert(dish.id);
    }

    int row = 0;
    int column = 0;
    auto addSection = [this, &row, &column](const QString &title){
        if(column != 0)
            ++row;
        column = 0;
        QLabel *heading = new QLabel(title, container);
        heading->setObjectName("dishSectionHeading");
        heading->setMinimumHeight(58);
        heading->setStyleSheet(darkMode
            ? "QLabel{color:#f5f5f5;font-size:25px;font-weight:900;padding:12px 8px;border-bottom:2px solid #f97316;}"
            : "QLabel{color:#7c2d12;font-size:25px;font-weight:900;padding:12px 8px;border-bottom:2px solid #fed7aa;}");
        layout->addWidget(heading, row++, 0, 1, 3);
        sectionMarkers.append(qMakePair(static_cast<QWidget *>(heading), title));
    };
    auto addCard = [this, &row, &column](const Dish &dish, bool recommended){
        DishCard *card = new DishCard(dish, container, recommended);
        card->setDarkMode(darkMode);
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
        layout->addWidget(card, row, column, Qt::AlignTop | Qt::AlignHCenter);
        if(++column == 3)
        {
            column = 0;
            ++row;
        }
    };

    addSection("热销");
    int hotCount = 0;
    for(const Dish &dish : ranking)
    {
        if(dish.soldCount <= 0 || hotCount >= 5)
            break;
        addCard(dish, true);
        ++hotCount;
    }

    QList<int> categoryIds = dishCategories.keys();
    std::sort(categoryIds.begin(), categoryIds.end(), [this](int leftId, int rightId){
        const int leftPriority = categoryPriority(dishCategories.value(leftId));
        const int rightPriority = categoryPriority(dishCategories.value(rightId));
        if(leftPriority != rightPriority)
            return leftPriority < rightPriority;
        return leftId < rightId;
    });

    for(int categoryId : categoryIds)
    {
        QList<Dish> categoryDishes;
        for(const Dish &dish : dishes)
            if(dish.catId == categoryId)
                categoryDishes.append(dish);
        if(categoryDishes.isEmpty())
            continue;
        std::sort(categoryDishes.begin(), categoryDishes.end(), [](const Dish &left, const Dish &right){
            return left.id < right.id;
        });
        addSection(categoryDisplayName(dishCategories.value(categoryId)));
        for(const Dish &dish : categoryDishes)
            addCard(dish, hotDishIds.contains(dish.id));
    }

    QTimer::singleShot(0, this, [this](){ updateCurrentCategory(); });
}

void DishWidget::updateCurrentCategory()
{
    if(sectionMarkers.isEmpty())
        return;

    const int viewportTop = scrollArea->verticalScrollBar()->value() + 90;
    QString current = sectionMarkers.first().second;
    for(const auto &section : sectionMarkers)
    {
        if(section.first && section.first->y() <= viewportTop)
            current = section.second;
        else
            break;
    }
    for(QPushButton *button : categoryWidget->findChildren<QPushButton *>())
        button->setChecked(button->property("sectionName").toString() == current);
}

QRect DishWidget::cartDrawerGeometry(bool opened) const
{
    const int drawerWidth = qMin(430, width() - 40);
    const int drawerTop = 92;
    const int drawerHeight = qMax(440, height() - drawerTop - 18);
    const int x = opened ? width() - drawerWidth - 28 : width() + 8;
    return QRect(x, drawerTop, drawerWidth, drawerHeight);
}

void DishWidget::applyTheme()
{
    themeButton->setText(darkMode?"白天模式":"夜间模式");
    QWidget *top=findChild<QWidget*>("topBar");
    if(top)
        if(auto *shadow=qobject_cast<QGraphicsDropShadowEffect*>(top->graphicsEffect()))
            shadow->setColor(darkMode ? QColor(0,0,0,95) : QColor(126,84,46,45));
    if(darkMode){
        setStyleSheet("DishWidget{background:#1a1a1a;}");
        if(top)top->setStyleSheet("QWidget#topBar{background:#2a2a2a;border:1px solid #353535;border-radius:20px;}");
        titleLabel->setStyleSheet("font-size:25px;font-weight:800;color:#ffffff;padding:8px 20px;background:transparent;");
        categoryWidget->setStyleSheet("QWidget#categoryRail{background:#121212;border:1px solid #242424;border-radius:20px;}");
        scrollArea->setStyleSheet("QScrollArea{background:#1e1e1e;border:none;border-radius:20px;} QScrollArea>QWidget>QWidget{background:#1e1e1e;border-radius:20px;} QScrollBar:vertical{width:0px;}");
        scrollArea->viewport()->setStyleSheet("background:#1e1e1e;border:none;border-radius:20px;");
        container->setStyleSheet("background:#1e1e1e;");
        cartSummaryBar->setStyleSheet("QWidget#cartSummaryBar{background:#181818;border:1px solid #333333;border-radius:20px;}");
        cartCountLabel->setStyleSheet("color:#b0b0b0;font-size:15px;");
        cartTotalLabel->setStyleSheet("color:#f97316;font-size:25px;font-weight:900;");
        themeButton->setStyleSheet("QPushButton{background:#2a2a2a;color:white;border:1px solid #f97316;border-radius:12px;padding:8px 18px;} QPushButton:hover{background:#333333;}");
        changeTableButton->setStyleSheet("QPushButton{background:#2a2a2a;color:white;border:1px solid #f97316;border-radius:12px;padding:8px 18px;} QPushButton:hover{background:#333333;}");
        backTableButton->setStyleSheet("QPushButton{background:#2a2a2a;color:#f5f5f5;border:1px solid #f97316;border-radius:12px;padding:8px 18px;} QPushButton:hover{background:#333333;color:#f97316;}");
    }else{
        setStyleSheet("DishWidget{background:#fffaf0;}");
        if(top)top->setStyleSheet("QWidget#topBar{background:white;border:none;border-radius:20px;}");
        titleLabel->setStyleSheet("font-size:25px;font-weight:800;color:#ea6a20;padding:8px 20px;");
        categoryWidget->setStyleSheet("QWidget#categoryRail{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #e07a2f,stop:1 #ff8c42);border-radius:20px;}");
        scrollArea->setStyleSheet("QScrollArea{background:#fffaf0;border:none;border-radius:20px;} QScrollBar:vertical{width:0px;}");
        scrollArea->viewport()->setStyleSheet("background:#fffaf0;border:none;border-radius:20px;");
        container->setStyleSheet("background:#fffaf0;");
        cartSummaryBar->setStyleSheet("QWidget#cartSummaryBar{background:rgba(255,255,255,245);border:1px solid #ffedd5;border-radius:20px;}");
        cartCountLabel->setStyleSheet("color:#6b7280;font-size:15px;");
        cartTotalLabel->setStyleSheet("color:#f97316;font-size:25px;font-weight:900;");
        themeButton->setStyleSheet("QPushButton{background:#374151;color:white;border:none;border-radius:12px;padding:8px 18px;}");
        changeTableButton->setStyleSheet("QPushButton{background:#ffd166;color:#9a4312;border:none;border-radius:12px;padding:8px 18px;} QPushButton:hover{background:#ffca45;}");
        backTableButton->setStyleSheet("QPushButton{background:#fff7ed;color:#ea6a20;border:1px solid #fed7aa;border-radius:12px;padding:8px 18px;} QPushButton:hover{background:#ffedd5;}");
    }
    for(QPushButton *button:categoryWidget->findChildren<QPushButton*>()){
        button->setStyleSheet(darkMode?
            "QPushButton{background:#2a2a2a;color:#b0b0b0;border:none;border-radius:16px;padding:12px;font-size:16px;font-weight:700;} QPushButton:hover{background:#333333;color:#e5e5e5;} QPushButton:checked{background:#f97316;color:white;font-weight:900;}":
            "QPushButton{background:transparent;color:rgba(255,255,255,210);border:none;border-radius:16px;padding:12px;font-size:16px;font-weight:700;} QPushButton:hover{background:rgba(255,255,255,35);} QPushButton:checked{background:#ffd166;color:#7c2d12;font-weight:900;}");
    }
    for(QLabel *heading : container->findChildren<QLabel *>("dishSectionHeading"))
    {
        heading->setStyleSheet(darkMode
            ? "QLabel{color:#ffffff;font-size:25px;font-weight:900;padding:12px 8px;border-bottom:2px solid #f97316;}"
            : "QLabel{color:#7c2d12;font-size:25px;font-weight:900;padding:12px 8px;border-bottom:2px solid #fed7aa;}");
    }
    for(DishCard *card:container->findChildren<DishCard*>())card->setDarkMode(darkMode);
    cartWidget->setDarkMode(darkMode);
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
        const int left = 18 + 160 + 14;
        cartSummaryBar->setGeometry(left, height() - 108, qMax(300, width() - left - 18), 90);
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
        Q_UNUSED(paidMoney);
        pendingReceiptPayType=payType;
        network->getOrderReceipt(paidOrderId);
    });

    connect(activePayWidget.data(), &QObject::destroyed, this, [this](){
        activePayWidget = nullptr;
        paymentOpening = false;
    });

    activePayWidget->show();
    activePayWidget->raise();
    activePayWidget->activateWindow();
}

void DishWidget::createPdfAndPrintReceipt(const QJsonObject &data, int payType)
{
    const int orderId=data["orderId"].toInt();
    const int tableId=data["tableId"].toInt();
    const double money=data["totalPrice"].toDouble();
    const QString channel=payType==1?"微信支付（模拟）":"支付宝（模拟）";
    const QString createTime=data["createTime"].toString().replace('T',' ');
    const QString finishTime=data["finishTime"].toString().replace('T',' ');
    QString rows;
    int number=1;
    for(const QJsonValue &value:data["items"].toArray()){
        const QJsonObject item=value.toObject();
        const int status=item["itemStatus"].toInt();
        const QString state=status==1?"已出餐":status==2?"已取消":"未出餐";
        const double subtotal=status==2?0:item["price"].toDouble()*item["count"].toInt();
        rows+=QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>￥%4</td><td>￥%5</td><td>%6</td></tr>")
            .arg(number++).arg(item["dishName"].toString().toHtmlEscaped())
            .arg(item["count"].toInt()).arg(item["price"].toDouble(),0,'f',2)
            .arg(subtotal,0,'f',2).arg(state);
    }
    QTextDocument document;
    document.setHtml(QString("<html><body style='font-family:Microsoft YaHei;color:#222;'>"
        "<h1 style='text-align:center;'>餐厅结账小票</h1><hr>"
        "<table width='100%' cellspacing='6'><tr><td>桌台：%1号桌</td><td>订单号：%2</td></tr>"
        "<tr><td>创建时间：%3</td><td>结束时间：%4</td></tr><tr><td colspan='2'>支付方式：%5</td></tr></table><br>"
        "<table width='100%' border='1' cellspacing='0' cellpadding='6' style='border-collapse:collapse;'>"
        "<tr style='background:#eeeeee;'><th>序号</th><th>菜品</th><th>数量</th><th>单价</th><th>小计</th><th>状态</th></tr>%6</table>"
        "<h2 style='text-align:right;'>实付金额：￥%7</h2><hr><p style='text-align:center;'>谢谢惠顾，欢迎再次光临！</p>"
        "</body></html>").arg(tableId).arg(orderId).arg(createTime,finishTime,channel,rows).arg(money,0,'f',2));

    const QString directory=QStringLiteral("D:/shixun/pdfxiaopiao");
    QDir().mkpath(directory);
    const QString pdfPath=directory+QString("/订单_%1_小票.pdf").arg(orderId);
    QPdfWriter writer(pdfPath);
    writer.setTitle(QString("订单%1结账小票").arg(orderId));
    writer.setCreator("RestaurantClient");
    writer.setPageSize(QPageSize(QPageSize::A5));
    writer.setPageMargins(QMarginsF(12,12,12,12),QPageLayout::Millimeter);
    document.print(&writer);
    if(!QFileInfo::exists(pdfPath)){
        QMessageBox::warning(this,"PDF生成失败","无法保存小票PDF");
        return;
    }
    if(QMessageBox::question(this,"PDF小票已生成",QString("小票已保存到：\n%1\n\n是否立即打印？").arg(QDir::toNativeSeparators(pdfPath)))!=QMessageBox::Yes)
        return;
    if(QPrinterInfo::availablePrinters().isEmpty()){
        QMessageBox::information(this,"没有打印机","PDF已保存，但系统中没有可用打印机。");
        return;
    }
    QPrinter printer(QPrinter::HighResolution);
    printer.setDocName(QString("订单%1结账小票").arg(orderId));
    QPrintDialog dialog(&printer,this);
    dialog.setWindowTitle("打印PDF小票");
    if(dialog.exec()==QDialog::Accepted)
        document.print(&printer);
}

void DishWidget::offerRewardDraw()
{
    if(rewardOrderId<=0||rewardChances<=0)return;
    if(activeBlindBox){activeBlindBox->show();activeBlindBox->raise();return;}
    activeBlindBox=new BlindBoxDialog(rewardChances,[this](){network->drawReward(rewardOrderId);},this);
    activeBlindBox->setAttribute(Qt::WA_DeleteOnClose);activeBlindBox->setWindowFlag(Qt::Window);
    connect(activeBlindBox,&QObject::destroyed,this,[this](){activeBlindBox=nullptr;});
    activeBlindBox->show();activeBlindBox->raise();activeBlindBox->activateWindow();
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

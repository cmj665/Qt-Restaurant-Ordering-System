#include "dishwidget.h"
#include "ui_dishwidget.h"
#include "dishcard.h"
#include "paywidget.h"
#include "orderdetailwidget.h"

#include <QMessageBox>

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

    titleLabel->setStyleSheet(
            "font-size:24px;"
            "font-weight:bold;"
        );

    //换桌按钮
    changeTableButton = new QPushButton("换桌",this);
    //返回按钮
    backTableButton = new QPushButton("返回桌台",this);
    //购物车
    cartButton = new QPushButton("购物车",this);


    changeTableButton->setMinimumSize(100,45);
    backTableButton->setMinimumSize(120,45);
    cartButton->setMinimumSize(120,45);


    QHBoxLayout *topLayout = new QHBoxLayout();

    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(changeTableButton);
    topLayout->addWidget(backTableButton);
    topLayout->addWidget(cartButton);


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

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(scrollArea);

    mainLayout->setContentsMargins(15,15,15,15);

    mainLayout->setSpacing(15);


    //---------------创建购物车窗口----------

    cartWidget = new CartWidget(this);

    cartWidget->setWindowFlag(Qt::Window);
    cartWidget->hide();



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

    //购物车按钮
    connect(cartButton,&QPushButton::clicked,this,[this](){
        cartWidget->show();
        cartWidget->raise();
        cartWidget->activateWindow();
    });

    connect(network,&NetworkManager::orderSubmitted,this,[this](bool success,const QString &message){
        if(success)
        {
            QMessageBox::information(this,"成功","订单提交成功");

            //清空购物车
            cartWidget->clearCart();
            //关闭购物车窗口
            cartWidget->hide();
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

                    connect(card,&DishCard::addDish,this,[this](const Dish &dish){
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



    //支付的连接购物车按钮
    connect(cartWidget,&CartWidget::checkoutRequested,this,[this](){
        qDebug()<<"点击结账 当前桌:"<<currentTableId;
        //查询该桌未支付订单
        network->getUnpaidOrder(currentTableId);
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
            PayWidget *pay = new PayWidget(pendingOrderId,pendingMoney,this);
            pay->setAttribute(Qt::WA_DeleteOnClose);
            pay->setWindowFlag(Qt::Window);
            pay->show();

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
    connect(cartWidget,&CartWidget::detailRequested,
            this,[this](){
        OrderDetailWidget *detail = new OrderDetailWidget(currentTableId,nullptr);

        detail->setWindowFlag(Qt::Window);
        detail->setAttribute(Qt::WA_DeleteOnClose);
        detail->setWindowTitle("订单详情");
        detail->resize(500,600);

        connect(detail,&OrderDetailWidget::payRequested,this,[this,detail](int orderId,double money){

            pendingOrderId = orderId;
            pendingMoney = money;


            //检查订单状态
            network->checkOrderCanPay(orderId);

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
                    QMessageBox::information(
                        this,
                        "提示",
                        msg
                        );

                    return;
                }

                PayWidget *pay =
                    new PayWidget(
                        pendingOrderId,
                        pendingMoney,
                        this
                        );

                pay->setAttribute(
                    Qt::WA_DeleteOnClose
                    );

                pay->show();

            });



    network->getDishList();
}

//实现刷新标题
void DishWidget::changeTable(int tableId)
{
    currentTableId = tableId;
    titleLabel->setText(QString("%1号桌点餐").arg(currentTableId));
}



DishWidget::~DishWidget()
{
    delete ui;
}

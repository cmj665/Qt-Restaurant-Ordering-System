#include "tablewidget.h"
#include "ui_tablewidget.h"


#include <QPushButton>
#include <QGridLayout>

TableWidget::TableWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TableWidget)
{
    ui->setupUi(this);
    //标题样式
    //居中
    ui->titleLabel->setAlignment(Qt::AlignCenter);
    // ui->titleLabel->setMinimumHeight(80);

    ui->titleLabel->setStyleSheet(
        "font-size:36px;"
        "font-weight:bold;"
        "color:#222222;"
        );

    network = new NetworkManager(this);

    //原来的
    // layout = new QGridLayout(this);
    // layout->spacing();
    // layout->setAlignment(Qt::AlignCenter);

    //-------------------------------------------
    layout = new QGridLayout();
    layout->setSpacing(40);
    layout->setAlignment(Qt::AlignCenter);


    //桌台区域
    QWidget *tableArea = new QWidget(this);

    tableArea->setLayout(layout);


    //状态说明
    createLegend();


    //整体布局
    QVBoxLayout *mainLayout =
        new QVBoxLayout(this);


    mainLayout->addWidget(
        ui->titleLabel
        );


    mainLayout->addWidget(
        tableArea
        );


    mainLayout->addWidget(
        legendWidget
        );


    setLayout(mainLayout);

    //-------------------------------------


    connect(network,&NetworkManager::tableListReceived,this,&TableWidget::showTables);

    network->getTableList();

    //创建桌台按钮
    // createTables();
}


//用于测试桌台9个生成 3*3
// void TableWidget::createTables()
// {
//     QGridLayout *layout = new QGridLayout(ui->tableContainer);

//     //按钮之间距离
//     layout->setSpacing(40);

//     //整体居中
//     layout->setAlignment(Qt::AlignCenter);

//     int tableId=1;

//     for(int row=0;row<3;row++)
//     {
//         for(int col=0;col<3;col++)
//         {
//             QPushButton *button = new QPushButton(QString("%1号桌").arg(tableId));
//             //桌台大小
//             button->setFixedSize(200,120);

//             //按钮样式
//             button->setStyleSheet(

//                 "QPushButton{"
//                 "background:white;"
//                 "border:2px solid #3498db;"
//                 "border-radius:15px;"
//                 "font-size:28px;"
//                 "font-weight:bold;"
//                 "color:#222222;"
//                 "}"


//                 "QPushButton:hover{"
//                 "background:#dff0ff;"
//                 "}"

//                 );

//             //加入网络
//            layout->addWidget(button,row,col,Qt::AlignCenter);

//            //点击桌台
//             connect(button,&QPushButton::clicked,this,[this,tableId](){
//                 emit tableSelected(tableId);
//             });

//             tableId++;
//         }
//     }
//     setLayout(layout);

// }

void TableWidget::showTables(QList<DiningTable> tables)
{
    int row = 0;
    int col = 0;
    for(const DiningTable &table: tables)
    {
        QPushButton *button = new QPushButton(table.tableName);
        button->setFixedSize(200,120);
        QString color;
        switch(table.status)
        {
        case 0:
            color="#2ecc71";
            break;
        case 1:
            color="#e74c3c";
            break;
        case 2:
            color="#f1c40f";
            break;
        case 3:
            color="#3498bd";
            break;
        }

        button->setStyleSheet(
            QString(
                "QPushButton{"
                "background:%1;"
                "border-radius:15px;"
                "font-size:28px;"
                "font-weight:bold;"
                "color:white;"
                "}"
                )
                .arg(color)
            );


        layout->addWidget(button,row,col);

        connect(button,&QPushButton::clicked,this,[this,table](){
            emit tableSelected(table.id);
        });

        col++;
        if(col==3)
        {
            col=0;
            row++;
        }

    }

}

QLabel* TableWidget::createColorLabel(QString color, QString text)
{
    QLabel *label = new QLabel(text);
    label->setAlignment(Qt::AlignCenter);
    label->setFixedSize(75,50);
    label->setStyleSheet(
        QString(
            "QLabel{"
            "background:%1;"
            "border-radius:10px;"
            "font-size:18px;"
            "font-weight:bold;"
            "color:white;"
            "padding:5px;"
            "}"
            )
            .arg(color)
        );

    return label;
}

void TableWidget::createLegend(){
    legendWidget = new QWidget(this);
    QHBoxLayout *layout =new QHBoxLayout(legendWidget);

    QLabel *title =new QLabel("状态说明：");
    title->setStyleSheet(
        "font-size:20px;"
        "font-weight:bold;"
        );
    layout->addWidget(title);

    //绿色
    layout->addWidget(
        createColorLabel(
            "#2ecc71",
            "空闲"
            )
        );

    //红色
    layout->addWidget(
        createColorLabel(
            "#e74c3c",
            "用餐中"
            )
        );

    //黄色
    layout->addWidget(
        createColorLabel(
            "#f1c40f",
            "待结账"
            )
        );

    //蓝色
    layout->addWidget(
        createColorLabel(
            "#3498bd",
            "待清理"
            )
        );

    layout->addStretch();
}



TableWidget::~TableWidget()
{
    delete ui;

}
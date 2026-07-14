#include "admindishwidget.h"
#include "disheditdialog.h"
#include "../network/networkmanager.h"
#include <QColor>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGraphicsDropShadowEffect>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QUrl>
#include <QTableWidget>
#include <QVBoxLayout>

AdminDishWidget::AdminDishWidget(QWidget *parent)
    : QWidget(parent), network(new NetworkManager(this)), imageManager(new QNetworkAccessManager(this)),
      table(new QTableWidget(this)), statusLabel(new QLabel(this))
{
    setObjectName("adminDishPage");
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("QWidget#adminDishPage{background:#f7f3eb;}");

    auto *title=new QLabel("菜品管理",this);
    title->setStyleSheet("font-family:'Microsoft YaHei UI';font-size:20px;font-weight:800;color:#333333;background:transparent;");
    auto *addButton=new QPushButton("新增菜品",this);
    auto *refreshButton=new QPushButton("刷新",this);
    addButton->setMinimumSize(124,44); refreshButton->setMinimumSize(92,44);
    addButton->setCursor(Qt::PointingHandCursor); refreshButton->setCursor(Qt::PointingHandCursor);
    addButton->setFocusPolicy(Qt::NoFocus); refreshButton->setFocusPolicy(Qt::NoFocus);
    addButton->setStyleSheet(
        "QPushButton{outline:none;background:#ef963a;color:white;border:0;border-radius:21px;font-size:15px;font-weight:700;padding:0 20px;}"
        "QPushButton:hover{background:#e9882c;} QPushButton:pressed{background:#d87920;}"
    );
    refreshButton->setStyleSheet(
        "QPushButton{outline:none;background:#fffaf4;color:#d98231;border:1px solid #e9a15b;border-radius:21px;font-size:15px;font-weight:600;padding:0 18px;}"
        "QPushButton:hover{background:#fff0df;} QPushButton:pressed{background:#fbe2c7;}"
    );
    auto *top=new QHBoxLayout;
    top->setSpacing(12);
    top->addWidget(title); top->addStretch(); top->addWidget(addButton); top->addWidget(refreshButton);

    table->setColumnCount(10);
    table->setHorizontalHeaderLabels({"图片","ID","菜品名称","分类","价格","库存","销量","状态","描述","操作"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Fixed);
    table->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Fixed);
    table->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(8,QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(9,QHeaderView::Fixed);
    table->horizontalHeader()->resizeSection(0,96);
    table->horizontalHeader()->resizeSection(1,58);
    table->horizontalHeader()->resizeSection(9,255);
    table->horizontalHeader()->setFixedHeight(52);
    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(82);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);
    table->setFocusPolicy(Qt::NoFocus);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setMouseTracking(false);
    table->viewport()->setMouseTracking(false);
    table->viewport()->setAttribute(Qt::WA_Hover, false);
    table->setStyleSheet(
        "QTableWidget{background:#ffffff;alternate-background-color:#faf7f2;border:0;color:#414141;font-size:14px;}"
        "QTableWidget::item{border:0;padding:8px;}"
        "QTableWidget::item:hover{background:transparent;}"
        "QTableWidget::item:selected{background:transparent;}"
        "QHeaderView::section{background:#f7d8b4;color:#3f3b38;border:0;padding:10px;font-size:14px;font-weight:700;}"
        "QScrollBar:vertical{background:#f5efe8;width:10px;margin:2px;border-radius:5px;}"
        "QScrollBar::handle:vertical{background:#c9bdae;border-radius:5px;min-height:35px;}"
        "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}"
    );

    auto *tableCard=new QWidget(this);
    tableCard->setObjectName("dishTableCard");
    tableCard->setStyleSheet("QWidget#dishTableCard{background:#ffffff;border-radius:22px;}");
    auto *shadow=new QGraphicsDropShadowEffect(tableCard);
    shadow->setBlurRadius(32); shadow->setOffset(0,10); shadow->setColor(QColor(76,58,42,38));
    tableCard->setGraphicsEffect(shadow);
    auto *cardLayout=new QVBoxLayout(tableCard);
    cardLayout->setContentsMargins(16,16,16,16);
    cardLayout->addWidget(table);

    auto *layout=new QVBoxLayout(this);
    layout->setContentsMargins(36,13,36,28);
    layout->setSpacing(16);
    layout->addLayout(top); layout->addWidget(tableCard,1); layout->addWidget(statusLabel);
    connect(addButton,&QPushButton::clicked,this,&AdminDishWidget::addDish);
    connect(refreshButton,&QPushButton::clicked,this,&AdminDishWidget::refresh);
    connect(network,&NetworkManager::dishCategoriesReceived,this,[this](const QList<DishCategory>&items){categories=items;refresh();});

    connect(network,&NetworkManager::dishListReceived,this,[this](const QList<Dish>&dishes){
        table->setRowCount(dishes.size());
        int active=0, removed=0, low=0;
        QStringList lowNames;
        for(int row=0;row<dishes.size();++row){
            const Dish dish=dishes[row];
            const bool deleted=dish.isDeleted==1;
            if(deleted) ++removed;
            else { ++active; if(dish.stock<10){++low;lowNames<<QString("%1(%2)").arg(dish.name).arg(dish.stock);} }
            QString categoryName=QString::number(dish.catId);
            for(const auto &item:categories) if(item.id==dish.catId){categoryName=item.name;break;}
            auto *imageContainer=new QWidget(table);
            imageContainer->setAttribute(Qt::WA_TranslucentBackground,true);
            auto *imageLayout=new QHBoxLayout(imageContainer);
            imageLayout->setContentsMargins(6,7,6,7);
            auto *thumbnail=new QLabel("暂无图片",imageContainer);
            thumbnail->setAlignment(Qt::AlignCenter);
            thumbnail->setFixedSize(72,58);
            thumbnail->setStyleSheet("QLabel{background:#eee8df;color:#aaa097;border-radius:10px;font-size:11px;}");
            imageLayout->addStretch(); imageLayout->addWidget(thumbnail); imageLayout->addStretch();
            table->setCellWidget(row,0,imageContainer);

            if(!dish.picture.trimmed().isEmpty()){
                QString imagePath=dish.picture.trimmed();
                QUrl imageUrl;
                if(imagePath.startsWith("http://")||imagePath.startsWith("https://")) imageUrl=QUrl(imagePath);
                else if(imagePath.startsWith("/images/")) imageUrl=QUrl("http://localhost:8080"+imagePath);
                else imageUrl=QUrl("http://localhost:8080/images/"+imagePath);
                QPointer<QLabel> safeThumbnail(thumbnail);
                QNetworkReply *reply=imageManager->get(QNetworkRequest(imageUrl));
                connect(reply,&QNetworkReply::finished,this,[reply,safeThumbnail](){
                    QPixmap source;
                    if(safeThumbnail && reply->error()==QNetworkReply::NoError && source.loadFromData(reply->readAll())){
                        const QSize size=safeThumbnail->size();
                        const QPixmap scaled=source.scaled(size,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
                        QPixmap rounded(size); rounded.fill(Qt::transparent);
                        QPainter painter(&rounded); painter.setRenderHint(QPainter::Antialiasing,true);
                        QPainterPath clip; clip.addRoundedRect(QRectF(QPointF(0,0),QSizeF(size)),10,10);
                        painter.setClipPath(clip);
                        painter.drawPixmap(0,0,scaled,scaled.width()/2-size.width()/2,scaled.height()/2-size.height()/2,size.width(),size.height());
                        safeThumbnail->setText(QString()); safeThumbnail->setPixmap(rounded);
                    }
                    reply->deleteLater();
                });
            }

            const QList<QPair<int,QString>> values{
                {1,QString::number(dish.id)},{2,dish.name},{3,categoryName},
                {4,QString("¥ %1").arg(dish.price,0,'f',2)},{5,QString::number(dish.stock)},
                {6,QString::number(dish.soldCount)},{8,dish.description}
            };
            for(const auto &value:values){
                auto *cell=new QTableWidgetItem(value.second);
                cell->setTextAlignment(value.first==8 ? Qt::AlignLeft|Qt::AlignVCenter : Qt::AlignCenter);
                table->setItem(row,value.first,cell);
            }

            auto *stateContainer=new QWidget(table);
            stateContainer->setAttribute(Qt::WA_TranslucentBackground,true);
            auto *stateLayout=new QHBoxLayout(stateContainer);
            stateLayout->setContentsMargins(6,8,6,8);
            auto *stateTag=new QLabel(deleted?"下架":"在售",stateContainer);
            stateTag->setAlignment(Qt::AlignCenter); stateTag->setFixedSize(72,32);
            stateTag->setStyleSheet(deleted
                ? "QLabel{background:#dedede;color:#686868;border-radius:15px;font-size:14px;font-weight:700;}"
                : "QLabel{background:#d7f0d9;color:#27823b;border-radius:15px;font-size:14px;font-weight:700;}"
            );
            stateLayout->addStretch(); stateLayout->addWidget(stateTag); stateLayout->addStretch();
            table->setCellWidget(row,7,stateContainer);

            if(!deleted && dish.stock<10){
                table->item(row,5)->setForeground(QColor("#d32f2f"));
                table->item(row,5)->setText(dish.stock==0?"0（缺货）":QString::number(dish.stock)+"（预警）");
            }
            if(deleted) for(int col=1;col<=8;++col) if(table->item(row,col)) table->item(row,col)->setForeground(QColor("#8b8b8b"));

            auto *actions=new QWidget(table); actions->setAttribute(Qt::WA_TranslucentBackground,true);
            auto *box=new QHBoxLayout(actions); box->setContentsMargins(6,10,6,10); box->setSpacing(6);
            auto *edit=new QPushButton("修改",actions); auto *stockButton=new QPushButton("调库存",actions);
            auto *stateButton=new QPushButton(deleted?"上架":"下架",actions);
            edit->setEnabled(!deleted); stockButton->setEnabled(!deleted);
            for(auto *actionButton:{edit,stockButton,stateButton}){
                actionButton->setFocusPolicy(Qt::NoFocus);
                actionButton->setCursor(actionButton->isEnabled()?Qt::PointingHandCursor:Qt::ArrowCursor);
                actionButton->setFixedHeight(34);
            }
            edit->setMinimumWidth(58); stockButton->setMinimumWidth(72); stateButton->setMinimumWidth(58);
            edit->setStyleSheet(
                "QPushButton{outline:none;background:transparent;color:#dc7c2b;border:0;border-radius:16px;font-size:14px;font-weight:700;}"
                "QPushButton:hover{background:#fff0df;} QPushButton:disabled{color:#b9b9b9;}"
            );
            stockButton->setStyleSheet(
                "QPushButton{outline:none;background:#eeeeee;color:#666666;border:0;border-radius:16px;font-size:13px;}"
                "QPushButton:hover{background:#e3e3e3;} QPushButton:disabled{color:#b4b4b4;background:#f2f2f2;}"
            );
            stateButton->setStyleSheet(deleted
                ? "QPushButton{outline:none;background:#fff0df;color:#d97828;border:0;border-radius:16px;font-size:13px;font-weight:700;} QPushButton:hover{background:#fbe2c7;}"
                : "QPushButton{outline:none;background:#eeeeee;color:#666666;border:0;border-radius:16px;font-size:13px;} QPushButton:hover{background:#e2e2e2;}"
            );
            box->addWidget(edit); box->addWidget(stockButton); box->addWidget(stateButton);
            connect(edit,&QPushButton::clicked,this,[this,dish](){editDish(dish);});
            connect(stockButton,&QPushButton::clicked,this,[this,dish](){adjustStock(dish);});
            connect(stateButton,&QPushButton::clicked,this,[this,dish,deleted](){
                if(deleted){
                    if(QMessageBox::question(this,"确认上架","确认重新上架“"+dish.name+"”？")==QMessageBox::Yes) network->restoreDish(dish.id);
                }else if(QMessageBox::question(this,"确认下架","确认下架“"+dish.name+"”？\n历史订单不会被删除。")==QMessageBox::Yes) network->deleteDish(dish.id);
            });
            table->setCellWidget(row,9,actions);
        }
        const QString warning=low>0?QString("库存预警 %1 项：%2").arg(low).arg(lowNames.join("、")):"库存正常";
        statusLabel->setStyleSheet(low>0?"color:#d32f2f;font-weight:bold;":"color:#2e7d32;");
        statusLabel->setText(QString("在售 %1 · 已下架 %2 · %3 · 最近刷新 %4")
            .arg(active).arg(removed).arg(warning).arg(QDateTime::currentDateTime().toString("HH:mm:ss")));
    });
    connect(network,&NetworkManager::dishOperationFinished,this,[this](bool success,const QString&message){
        if(!success) QMessageBox::warning(this,"操作失败",message);
        refresh();
    });
    network->getDishCategories();
}

void AdminDishWidget::refresh(){statusLabel->setText("正在读取菜品列表...");network->getDishList();}
void AdminDishWidget::addDish(){DishEditDialog dialog(categories,nullptr,this);if(dialog.exec()==QDialog::Accepted)network->addDish(dialog.value());}
void AdminDishWidget::editDish(const Dish&dish){DishEditDialog dialog(categories,&dish,this);if(dialog.exec()==QDialog::Accepted)network->updateDish(dialog.value());}
void AdminDishWidget::adjustStock(const Dish&dish)
{
    QDialog dialog(this);
    dialog.setWindowTitle("库存调整");
    dialog.setObjectName("stockDialog");
    dialog.setAttribute(Qt::WA_StyledBackground,true);
    dialog.setFixedSize(460,330);
    dialog.setStyleSheet(
        "QDialog#stockDialog{background:#f7f3eb;}"
        "QLabel{background:transparent;}"
        "QSpinBox{background:#fffdfa;color:#3f3b38;border:1px solid #ded4c8;border-radius:12px;font-size:17px;padding:8px 14px;}"
        "QSpinBox:focus{border:2px solid #e99a4b;padding:7px 13px;}"
        "QSpinBox{padding-right:50px;}"
        "QSpinBox::up-button{subcontrol-origin:border;subcontrol-position:top right;width:44px;background:#f8ead9;border-left:1px solid #ded4c8;border-bottom:1px solid #e8dbcd;border-top-right-radius:11px;}"
        "QSpinBox::down-button{subcontrol-origin:border;subcontrol-position:bottom right;width:44px;background:#f8ead9;border-left:1px solid #ded4c8;border-bottom-right-radius:11px;}"
        "QSpinBox::up-arrow{image:url(:/controls/up-arrow.xpm);width:18px;height:18px;}"
        "QSpinBox::down-arrow{image:url(:/controls/down-arrow.xpm);width:18px;height:18px;}"
        "QSpinBox::up-button:hover,QSpinBox::down-button:hover{background:#f3dcc1;}"
    );

    auto *title=new QLabel("调整库存",&dialog);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size:26px;font-weight:800;color:#333333;");
    auto *dishName=new QLabel(dish.name,&dialog);
    dishName->setAlignment(Qt::AlignCenter);
    dishName->setStyleSheet("font-size:15px;color:#8a8179;");
    auto *hint=new QLabel("请输入新的库存数量",&dialog);
    hint->setStyleSheet("font-size:14px;font-weight:600;color:#514b46;");

    auto *stockInput=new QSpinBox(&dialog);
    stockInput->setRange(0,999999); stockInput->setValue(dish.stock);
    stockInput->setMinimumHeight(50); stockInput->selectAll();

    auto *buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,&dialog);
    auto *saveButton=buttons->button(QDialogButtonBox::Save);
    auto *cancelButton=buttons->button(QDialogButtonBox::Cancel);
    saveButton->setText("确认调整"); cancelButton->setText("取消");
    saveButton->setMinimumSize(116,42); cancelButton->setMinimumSize(92,42);
    saveButton->setFocusPolicy(Qt::NoFocus); cancelButton->setFocusPolicy(Qt::NoFocus);
    saveButton->setCursor(Qt::PointingHandCursor); cancelButton->setCursor(Qt::PointingHandCursor);
    saveButton->setStyleSheet("QPushButton{outline:none;background:#ed9338;color:white;border:0;border-radius:20px;font-size:15px;font-weight:700;padding:0 20px;} QPushButton:hover{background:#e4862c;} QPushButton:pressed{background:#d67722;}");
    cancelButton->setStyleSheet("QPushButton{outline:none;background:#eeeeee;color:#666666;border:0;border-radius:20px;font-size:15px;padding:0 20px;} QPushButton:hover{background:#e2e2e2;}");

    auto *layout=new QVBoxLayout(&dialog);
    layout->setContentsMargins(40,28,40,30); layout->setSpacing(12);
    layout->addWidget(title); layout->addWidget(dishName); layout->addSpacing(10);
    layout->addWidget(hint); layout->addWidget(stockInput); layout->addStretch(); layout->addWidget(buttons);
    connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);
    connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    if(dialog.exec()==QDialog::Accepted) network->adjustStock(dish.id,stockInput->value());
}

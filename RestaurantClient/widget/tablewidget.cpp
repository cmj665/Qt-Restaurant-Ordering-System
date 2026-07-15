#include "tablewidget.h"
#include "ui_tablewidget.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QScroller>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QPainter>
#include <QSet>
#include <QColor>
#include <algorithm>
#include <QGraphicsDropShadowEffect>
#include <QRadialGradient>
#include <QFont>

class TableStatusButton final : public QPushButton
{
public:
    explicit TableStatusButton(QWidget *parent=nullptr):QPushButton(parent)
    {
        setCursor(Qt::PointingHandCursor);
        setFlat(true);
        setAttribute(Qt::WA_TranslucentBackground, true);
    }
    void setTableData(const DiningTable &value){data=value;update();}
protected:
    bool hitButton(const QPoint &position) const override {
        // 视觉仍保持圆角，但整张桌台卡片都应响应点击，避免顶部圆角区域出现点击盲区。
        return rect().contains(position);
    }
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);painter.setRenderHint(QPainter::Antialiasing,true);
        QRectF card=rect().adjusted(5,5,-5,-5);
        if(isDown())card.translate(0,3);
        QColor first("#94a3b8"),second("#64748b");QString state="未知";
        switch(data.status){
        case 0:first=QColor("#6bcb77");second=QColor("#36a853");state="空闲";break;
        case 1:first=QColor("#4ea8de");second=QColor("#2563a8");state="用餐中";break;
        case 2:first=QColor("#ffd65a");second=QColor("#e7a916");state="待支付";break;
        case 3:first=QColor("#90a4ae");second=QColor("#546e7a");state="完成待清理";break;
        }
        if(underMouse()){first=first.lighter(112);second=second.lighter(108);}
        QRadialGradient gradient(card.center()-QPointF(48,35),card.width()*.72);
        gradient.setColorAt(0,first);gradient.setColorAt(1,second);
        painter.setBrush(gradient);
        painter.setPen(QPen(QColor(255,255,255,105),2));
        painter.drawRoundedRect(card, 28, 28);
        painter.setPen(Qt::white);
        QFont identifier("Arial Rounded MT Bold");identifier.setPixelSize(33);identifier.setWeight(QFont::Black);identifier.setLetterSpacing(QFont::AbsoluteSpacing,1.0);painter.setFont(identifier);
        painter.drawText(QRectF(0,21,width(),46),Qt::AlignCenter,data.tableName);
        QFont statusFont("Microsoft YaHei UI");statusFont.setPixelSize(17);statusFont.setWeight(QFont::DemiBold);painter.setFont(statusFont);
        painter.drawText(QRectF(0,69,width(),27),Qt::AlignCenter,state);
        QFont capacityFont("Microsoft YaHei UI");capacityFont.setPixelSize(14);painter.setFont(capacityFont);painter.setPen(QColor(255,255,255,205));
        painter.drawText(QRectF(0,97,width(),23),Qt::AlignCenter,QString("%1人桌").arg(data.capacity));
    }
private: DiningTable data{};
};

TableWidget::TableWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TableWidget)
    , network(new NetworkManager(this))
    , graphicsView(new QGraphicsView(this))
    , scene(new QGraphicsScene(this))
    , refreshButton(new QPushButton("立即刷新", this))
    , refreshStatusLabel(new QLabel("正在获取桌台状态...", this))
    , refreshTimer(new QTimer(this))
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground,true);
    setStyleSheet("TableWidget{background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #fffaf0,stop:0.55 #f8f0df,stop:1 #f3e7d2);}");
    ui->titleLabel->setText("桌台状态中心");
    ui->titleLabel->setAlignment(Qt::AlignCenter);
    ui->titleLabel->setStyleSheet("font-size:38px;font-weight:900;color:#5b3a22;padding:8px;background:transparent;");

    graphicsView->setScene(scene);
    graphicsView->setRenderHint(QPainter::Antialiasing, true);
    graphicsView->setFrameShape(QFrame::NoFrame);
    graphicsView->setBackgroundBrush(Qt::NoBrush);
    graphicsView->setStyleSheet("QGraphicsView{background:transparent;border:none;} QGraphicsView>QWidget{background:transparent;}");
    graphicsView->viewport()->setStyleSheet("background:transparent;");
    graphicsView->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    graphicsView->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
    QScroller::grabGesture(graphicsView->viewport(), QScroller::TouchGesture);

    refreshButton->setMinimumSize(118, 42);
    refreshButton->setCursor(Qt::PointingHandCursor);
    refreshButton->setStyleSheet(
        "QPushButton{background:#f2a14a;color:white;border:1px solid #e58a2d;border-radius:12px;font-size:16px;font-weight:700;padding:7px 15px;}"
        "QPushButton:hover{background:#eb9132;} QPushButton:pressed{background:#dc7d22;}"
    );
    refreshStatusLabel->setStyleSheet("font-size:14px;color:#7a5335;background:rgba(255,255,255,150);border:1px solid #ead8bd;border-radius:12px;padding:7px 14px;");

    QGridLayout *toolbar = new QGridLayout;
    toolbar->setContentsMargins(0, 0, 0, 0);
    toolbar->setColumnStretch(0, 1);
    toolbar->setColumnStretch(2, 1);
    toolbar->addWidget(refreshStatusLabel, 0, 1, Qt::AlignCenter);
    toolbar->addWidget(refreshButton, 0, 2, Qt::AlignRight | Qt::AlignVCenter);
    createLegend();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(48, 24, 48, 24);
    mainLayout->setSpacing(12);
    mainLayout->addWidget(ui->titleLabel);
    mainLayout->addLayout(toolbar);
    mainLayout->addWidget(graphicsView, 1);
    mainLayout->addWidget(legendWidget);
    setLayout(mainLayout);

    // .ui 中遗留的 tableContainer 已被上面的 QGraphicsView 取代。
    // 它采用绝对定位覆盖在左上角，会透明地截获第一张桌台卡片上半部分的鼠标事件。
    ui->tableContainer->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->tableContainer->hide();
    ui->widget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->widget->lower();
    ui->titleLabel->show();

    connect(refreshButton, &QPushButton::clicked, this, &TableWidget::refreshTables);
    connect(network, &NetworkManager::tableListReceived, this, &TableWidget::showTables);
    refreshTimer->setInterval(2000);
    connect(refreshTimer, &QTimer::timeout, this, &TableWidget::refreshTables);
    refreshTimer->start();
    refreshTables();
}

void TableWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    refreshTables();
}

void TableWidget::refreshTables()
{
    refreshButton->setEnabled(false);
    refreshStatusLabel->setText("正在刷新桌台状态...");
    network->getTableList();
}

void TableWidget::showTables(QList<DiningTable> tables)
{
    QSet<int> receivedIds;
    for(const DiningTable &table : tables)
    {
        receivedIds.insert(table.id);
        updateTableItem(table);
    }

    const QList<int> existingIds = tableItems.keys();
    for(int id : existingIds)
    {
        if(!receivedIds.contains(id))
        {
            scene->removeItem(tableItems.take(id));
            delete tableButtons.take(id);
        }
    }

    arrangeTableItems();
    refreshButton->setEnabled(true);
    refreshStatusLabel->setText(
        QString("触屏可滑动 · 自动刷新2秒 · %1").arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
    );
}

void TableWidget::updateTableItem(const DiningTable &table)
{
    QPushButton *button = tableButtons.value(table.id, nullptr);
    if(!button)
    {
        button = new TableStatusButton;
        button->setFixedSize(280, 144);
        button->setProperty("tableId", table.id);
        QGraphicsProxyWidget *proxy = scene->addWidget(button);
        button->show();
        proxy->setVisible(true);
        proxy->setOpacity(1.0);
        auto *shadow=new QGraphicsDropShadowEffect;
        shadow->setBlurRadius(28);shadow->setOffset(0,10);shadow->setColor(QColor(116,62,25,85));
        proxy->setGraphicsEffect(shadow);
        proxy->setCacheMode(QGraphicsItem::NoCache);
        tableButtons.insert(table.id, button);
        tableItems.insert(table.id, proxy);
    }

    static_cast<TableStatusButton *>(button)->setTableData(table);

    button->disconnect(this);
    connect(button, &QPushButton::clicked, this, [this, table](){
        if(table.status == 3)
        {
            QMessageBox::information(this, "等待清理",
                QString("%1已完成结账，正在等待管理员清理。\n清理完成后桌台会自动恢复为空闲状态。")
                    .arg(table.tableName));
            return;
        }
        emit tableSelected(table.id);
    });
}

void TableWidget::arrangeTableItems()
{
    QList<int> ids = tableItems.keys();
    std::sort(ids.begin(), ids.end());
    constexpr int columns = 3;
    constexpr qreal cardWidth = 280;
    constexpr qreal cardHeight = 144;
    constexpr qreal horizontalGap = 32;
    constexpr qreal verticalGap = 24;
    constexpr qreal margin = 32;

    for(int index = 0; index < ids.size(); ++index)
    {
        const int row = index / columns;
        const int col = index % columns;
        tableItems[ids[index]]->setPos(
            margin + col * (cardWidth + horizontalGap),
            margin + row * (cardHeight + verticalGap));
    }
    const int rows = (ids.size() + columns - 1) / columns;
    scene->setSceneRect(0, 0,
        margin * 2 + columns * cardWidth + (columns - 1) * horizontalGap,
        margin * 2 + qMax(1, rows) * cardHeight + qMax(0, rows - 1) * verticalGap);
}

QLabel *TableWidget::createColorLabel(const QString &color, const QString &text)
{
    QLabel *label = new QLabel(text, this);
    label->setAlignment(Qt::AlignCenter);
    label->setMinimumSize(125, 40);
    label->setStyleSheet(QString(
        "QLabel{background:%1;border:2px solid rgba(255,255,255,90);border-radius:18px;font-size:15px;font-weight:bold;color:white;padding:4px;}"
    ).arg(color));
    return label;
}

void TableWidget::createLegend()
{
    legendWidget = new QWidget(this);
    legendWidget->setStyleSheet("QWidget{background:rgba(255,255,255,150);border:1px solid #e6d2b4;border-radius:18px;} QLabel{border:none;}");
    QHBoxLayout *legend = new QHBoxLayout(legendWidget);
    QLabel *title = new QLabel("状态说明：", legendWidget);
    title->setStyleSheet("font-size:17px;font-weight:bold;color:#5b3a22;background:transparent;");
    legend->addWidget(title);
    legend->addWidget(createColorLabel("#36a853", "空闲"));
    legend->addWidget(createColorLabel("#2563a8", "用餐中"));
    legend->addWidget(createColorLabel("#e7a916", "待支付"));
    legend->addWidget(createColorLabel("#546e7a", "完成待清理"));
    legend->addStretch();
}

TableWidget::~TableWidget()
{
    delete ui;
}

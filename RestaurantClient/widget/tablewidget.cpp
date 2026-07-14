#include "tablewidget.h"
#include "ui_tablewidget.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QScroller>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QPainter>
#include <QSet>
#include <QColor>
#include <algorithm>

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
    ui->titleLabel->setText("桌台状态中心");
    ui->titleLabel->setAlignment(Qt::AlignCenter);
    ui->titleLabel->setStyleSheet("font-size:36px;font-weight:bold;color:#22313f;padding:8px;");

    graphicsView->setScene(scene);
    graphicsView->setRenderHint(QPainter::Antialiasing, true);
    graphicsView->setFrameShape(QFrame::NoFrame);
    graphicsView->setBackgroundBrush(QColor("#f5f7fa"));
    graphicsView->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    graphicsView->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
    QScroller::grabGesture(graphicsView->viewport(), QScroller::TouchGesture);

    refreshButton->setMinimumSize(118, 42);
    refreshButton->setCursor(Qt::PointingHandCursor);
    refreshButton->setStyleSheet(
        "QPushButton{background:#34495e;color:white;border:none;border-radius:9px;font-size:16px;padding:7px 15px;}"
        "QPushButton:hover{background:#2c3e50;} QPushButton:pressed{background:#20303d;}"
    );
    refreshStatusLabel->setStyleSheet("font-size:14px;color:#6b7b8c;");

    QHBoxLayout *toolbar = new QHBoxLayout;
    toolbar->addWidget(refreshStatusLabel);
    toolbar->addStretch();
    toolbar->addWidget(refreshButton);
    createLegend();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 20, 30, 22);
    mainLayout->setSpacing(12);
    mainLayout->addWidget(ui->titleLabel);
    mainLayout->addLayout(toolbar);
    mainLayout->addWidget(graphicsView, 1);
    mainLayout->addWidget(legendWidget);
    setLayout(mainLayout);

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
        button = new QPushButton;
        button->setFixedSize(245, 135);
        button->setCursor(Qt::PointingHandCursor);
        button->setProperty("tableId", table.id);
        QGraphicsProxyWidget *proxy = scene->addWidget(button);
        proxy->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        tableButtons.insert(table.id, button);
        tableItems.insert(table.id, proxy);
    }

    QString color = "#95a5a6";
    QString hover = "#7f8c8d";
    QString state = "未知";
    QString textColor = "white";
    switch(table.status)
    {
    case 0: color="#27ae60"; hover="#2ecc71"; state="空闲"; break;
    case 1: color="#e67e22"; hover="#f39c12"; state="用餐中"; break;
    case 2: color="#f1c40f"; hover="#f4d03f"; state="待结账"; textColor="#4a3b00"; break;
    case 3: color="#607d8b"; hover="#78909c"; state="完成待清理"; break;
    }

    button->setText(QString("%1\n%2\n%3人桌").arg(table.tableName, state).arg(table.capacity));
    button->setStyleSheet(QString(
        "QPushButton{background:%1;color:%2;border:3px solid rgba(255,255,255,150);"
        "border-radius:18px;font-size:21px;font-weight:bold;padding:10px;}"
        "QPushButton:hover{background:%3;border-color:#34495e;}"
        "QPushButton:pressed{border-width:5px;padding-top:14px;}"
    ).arg(color, textColor, hover));

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
    constexpr qreal cardWidth = 245;
    constexpr qreal cardHeight = 135;
    constexpr qreal horizontalGap = 38;
    constexpr qreal verticalGap = 30;
    constexpr qreal margin = 25;

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
    label->setMinimumSize(112, 40);
    label->setStyleSheet(QString(
        "QLabel{background:%1;border-radius:8px;font-size:15px;font-weight:bold;color:white;padding:4px;}"
    ).arg(color));
    return label;
}

void TableWidget::createLegend()
{
    legendWidget = new QWidget(this);
    QHBoxLayout *legend = new QHBoxLayout(legendWidget);
    QLabel *title = new QLabel("状态说明：", legendWidget);
    title->setStyleSheet("font-size:17px;font-weight:bold;");
    legend->addWidget(title);
    legend->addWidget(createColorLabel("#27ae60", "空闲"));
    legend->addWidget(createColorLabel("#e67e22", "用餐中"));
    legend->addWidget(createColorLabel("#f1c40f", "待结账"));
    legend->addWidget(createColorLabel("#607d8b", "完成待清理"));
    legend->addStretch();
}

TableWidget::~TableWidget()
{
    delete ui;
}

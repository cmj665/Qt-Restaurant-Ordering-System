#include "adminorderwidget.h"
#include "../network/networkmanager.h"

#include <QDateTime>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

AdminOrderWidget::AdminOrderWidget(QWidget *parent)
    : QWidget(parent), network(new NetworkManager(this)), table(new QTableWidget(this)),
      summary(new QLabel(this)), timer(new QTimer(this))
{
    setObjectName("adminOrderPage");
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("QWidget#adminOrderPage{background:#f7f3eb;}");

    auto *title = new QLabel("出餐管理", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-family:'Microsoft YaHei UI';font-size:34px;font-weight:800;color:#333333;background:transparent;");

    auto *refreshButton = new QPushButton("立即刷新", this);
    refreshButton->setMinimumSize(108, 42);
    refreshButton->setCursor(Qt::PointingHandCursor);
    refreshButton->setFocusPolicy(Qt::NoFocus);
    refreshButton->setStyleSheet(
        "QPushButton{outline:none;background:#fffaf4;color:#d98231;border:1px solid #e9a15b;border-radius:20px;font-size:14px;font-weight:600;padding:0 18px;}"
        "QPushButton:hover{background:#fff0df;} QPushButton:pressed{background:#fbe2c7;}"
    );

    auto *titleRow = new QGridLayout;
    titleRow->setContentsMargins(0, 0, 0, 0);
    titleRow->setColumnStretch(0, 1);
    titleRow->setColumnStretch(2, 1);
    titleRow->addWidget(title, 0, 1, Qt::AlignCenter);
    titleRow->addWidget(refreshButton, 0, 2, Qt::AlignRight | Qt::AlignVCenter);

    summary->setAlignment(Qt::AlignCenter);
    summary->setTextFormat(Qt::RichText);
    summary->setStyleSheet("font-size:17px;color:#7d7168;background:transparent;");

    table->setColumnCount(9);
    table->setHorizontalHeaderLabels({"桌台", "订单号", "菜品名称", "数量", "单价", "出餐状态", "下单时间", "订单项ID", "操作"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Fixed);
    table->horizontalHeader()->resizeSection(8, 220);
    table->horizontalHeader()->setFixedHeight(52);
    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(66);
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
        "QTableWidget::item:hover{background:transparent;} QTableWidget::item:selected{background:transparent;}"
        "QHeaderView::section{background:#f7d8b4;color:#3f3b38;border:0;padding:10px;font-size:14px;font-weight:700;}"
        "QScrollBar:vertical{background:#f5efe8;width:10px;margin:2px;border-radius:5px;}"
        "QScrollBar::handle:vertical{background:#c9bdae;border-radius:5px;min-height:35px;}"
        "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}"
    );

    auto *tableCard = new QWidget(this);
    tableCard->setObjectName("orderTableCard");
    tableCard->setStyleSheet("QWidget#orderTableCard{background:#ffffff;border-radius:22px;}");
    auto *shadow = new QGraphicsDropShadowEffect(tableCard);
    shadow->setBlurRadius(32);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(76, 58, 42, 38));
    tableCard->setGraphicsEffect(shadow);
    auto *cardLayout = new QVBoxLayout(tableCard);
    cardLayout->setContentsMargins(16, 16, 16, 16);
    cardLayout->addWidget(table);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(36, 13, 36, 28);
    layout->setSpacing(12);
    layout->addLayout(titleRow);
    layout->addWidget(summary);
    layout->addSpacing(4);
    layout->addWidget(tableCard, 1);

    connect(refreshButton, &QPushButton::clicked, this, &AdminOrderWidget::refresh);
    connect(network, &NetworkManager::adminOrderItemsReceived, this, [this](const QJsonArray &items) {
        table->setRowCount(items.size());
        int pending = 0;
        for (int row = 0; row < items.size(); ++row) {
            const QJsonObject item = items[row].toObject();
            const int status = item["itemStatus"].toInt();
            if (status == 0) ++pending;

            QString createTime = item["createTime"].toString();
            const QDateTime parsedTime = QDateTime::fromString(createTime, Qt::ISODate);
            if (parsedTime.isValid()) createTime = parsedTime.toString("yyyy-MM-dd HH:mm:ss");
            else createTime.replace('T', ' ');
            if (createTime.isEmpty()) createTime = "--";

            const QList<QPair<int, QString>> values{
                {0, item["tableName"].toString()},
                {1, QString::number(item["orderId"].toInt())},
                {2, item["dishName"].toString()},
                {3, QString::number(item["count"].toInt())},
                {4, QString("¥ %1").arg(item["price"].toDouble(), 0, 'f', 2)},
                {6, createTime},
                {7, QString::number(item["id"].toInt())}
            };
            for (const auto &value : values) {
                auto *cell = new QTableWidgetItem(value.second);
                cell->setTextAlignment(Qt::AlignCenter);
                table->setItem(row, value.first, cell);
            }

            auto *statusContainer = new QWidget(table);
            statusContainer->setAttribute(Qt::WA_TranslucentBackground, true);
            auto *statusLayout = new QHBoxLayout(statusContainer);
            statusLayout->setContentsMargins(6, 10, 6, 10);
            auto *statusTag = new QLabel(status == 0 ? "未出餐" : status == 1 ? "已出餐" : "已取消", statusContainer);
            statusTag->setAlignment(Qt::AlignCenter);
            statusTag->setFixedSize(82, 34);
            const QString statusStyle = status == 0
                ? "QLabel{background:#f7a23b;color:white;border-radius:16px;font-size:14px;font-weight:700;}"
                : status == 1
                    ? "QLabel{background:#d7f0d9;color:#27823b;border-radius:16px;font-size:14px;font-weight:700;}"
                    : "QLabel{background:#e1e1e1;color:#707070;border-radius:16px;font-size:14px;font-weight:700;}";
            statusTag->setStyleSheet(statusStyle);
            statusLayout->addStretch();
            statusLayout->addWidget(statusTag);
            statusLayout->addStretch();
            table->setCellWidget(row, 5, statusContainer);

            auto *actions = new QWidget(table);
            actions->setAttribute(Qt::WA_TranslucentBackground, true);
            auto *box = new QHBoxLayout(actions);
            box->setContentsMargins(6, 10, 6, 10);
            box->setSpacing(8);
            auto *serve = new QPushButton("确认出餐", actions);
            auto *cancel = new QPushButton("取消菜品", actions);
            for (auto *button : {serve, cancel}) {
                button->setFixedHeight(36);
                button->setMinimumWidth(88);
                button->setFocusPolicy(Qt::NoFocus);
                button->setEnabled(status == 0);
                button->setCursor(status == 0 ? Qt::PointingHandCursor : Qt::ArrowCursor);
            }
            serve->setStyleSheet(
                "QPushButton{outline:none;background:#ed9338;color:white;border:0;border-radius:17px;font-size:13px;font-weight:700;}"
                "QPushButton:hover{background:#e4862c;} QPushButton:pressed{background:#d67722;}"
                "QPushButton:disabled{background:#e6e1dc;color:#aaa39c;}"
            );
            cancel->setStyleSheet(
                "QPushButton{outline:none;background:#eeeeee;color:#666666;border:0;border-radius:17px;font-size:13px;}"
                "QPushButton:hover{background:#e2e2e2;} QPushButton:disabled{color:#b5b5b5;background:#f2f2f2;}"
            );
            box->addWidget(serve);
            box->addWidget(cancel);
            const int id = item["id"].toInt();
            connect(serve, &QPushButton::clicked, this, [this, id]() { network->updateOrderItemStatus(id, 1); });
            connect(cancel, &QPushButton::clicked, this, [this, id]() {
                if (QMessageBox::question(this, "确认取消", "取消后将退回库存并从订单金额中扣除，确定继续？") == QMessageBox::Yes)
                    network->updateOrderItemStatus(id, 2);
            });
            table->setCellWidget(row, 8, actions);
        }
        summary->setText(QString(
            "当前未支付订单项 <span style='color:#df862d;font-size:21px;font-weight:800;'>%1</span> 个"
            "　·　待出餐 <span style='color:#df862d;font-size:21px;font-weight:800;'>%2</span> 个"
            "　·　每 2 秒自动同步"
        ).arg(items.size()).arg(pending));
    });
    connect(network, &NetworkManager::orderItemOperationFinished, this, [this](bool ok, const QString &message) {
        if (!ok) QMessageBox::warning(this, "操作失败", message);
        refresh();
    });
    timer->setInterval(2000);
    connect(timer, &QTimer::timeout, this, &AdminOrderWidget::refresh);
    timer->start();
    refresh();
}

void AdminOrderWidget::refresh()
{
    network->getAdminOrderItems();
}

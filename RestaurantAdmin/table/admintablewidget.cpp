#include "admintablewidget.h"
#include "../network/networkmanager.h"

#include <QDateTime>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

AdminTableWidget::AdminTableWidget(const QString &username, QWidget *parent)
    : QWidget(parent), network(new NetworkManager(this)), table(new QTableWidget(this)),
      summaryLabel(new QLabel(this)), statusLabel(new QLabel(this)), timer(new QTimer(this))
{
    // 页面顶部：显示管理员信息，并提供立即刷新和退出登录操作。
    setObjectName("adminTablePage");
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("QWidget#adminTablePage{background:#f7f3eb;}");

    auto *title = new QLabel("桌台管理", this);
    title->setStyleSheet("font-family:'Microsoft YaHei UI';font-size:20px;font-weight:800;color:#333333;background:transparent;");
    auto *user = new QLabel("管理员：" + username, this);
    user->setStyleSheet("font-size:14px;color:#777777;background:transparent;margin-right:8px;");
    auto *refreshButton = new QPushButton("立即刷新", this);
    auto *logoutButton = new QPushButton("退出登录", this);
    refreshButton->setCursor(Qt::PointingHandCursor);
    logoutButton->setCursor(Qt::PointingHandCursor);
    refreshButton->setFocusPolicy(Qt::NoFocus);
    logoutButton->setFocusPolicy(Qt::NoFocus);
    refreshButton->setMinimumSize(112, 42);
    logoutButton->setMinimumSize(108, 42);
    refreshButton->setStyleSheet(
        "QPushButton{outline:none;background:#fffaf4;color:#d98231;border:1px solid #e9a15b;border-radius:20px;font-size:15px;font-weight:600;padding:0 18px;}"
        "QPushButton:hover{background:#fff0df;border-color:#df8737;} QPushButton:pressed{background:#fbe2c7;}"
    );
    logoutButton->setStyleSheet(
        "QPushButton{outline:none;background:#e9e9e9;color:#555555;border:0;border-radius:20px;font-size:15px;font-weight:600;padding:0 18px;}"
        "QPushButton:hover{background:#dddddd;} QPushButton:pressed{background:#d1d1d1;}"
    );
    auto *top = new QHBoxLayout;
    top->setSpacing(12);
    top->addWidget(title); top->addStretch(); top->addWidget(user, 0, Qt::AlignVCenter);
    top->addWidget(refreshButton); top->addWidget(logoutButton);

    // 桌台表格：展示桌台容量、当前状态、状态编号和清台操作。
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"桌台", "容纳人数", "当前状态", "状态编号", "操作"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);
    table->setFocusPolicy(Qt::NoFocus);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setMouseTracking(false);
    table->viewport()->setMouseTracking(false);
    table->viewport()->setAttribute(Qt::WA_Hover, false);
    table->horizontalHeader()->setFixedHeight(52);
    table->verticalHeader()->setDefaultSectionSize(58);
    table->setStyleSheet(
        "QTableWidget{background:#ffffff;alternate-background-color:#f6f6f6;border:0;font-size:15px;color:#404040;}"
        "QTableWidget::item{border:0;padding:8px;}"
        "QTableWidget::item:hover{background:transparent;}"
        "QTableWidget::item:selected{background:transparent;}"
        "QHeaderView::section{background:#3f4144;color:white;padding:10px;border:0;font-size:15px;font-weight:700;}"
        "QScrollBar:vertical{background:#f3f0eb;width:10px;margin:2px;border-radius:5px;}"
        "QScrollBar::handle:vertical{background:#c9c3bb;border-radius:5px;min-height:35px;}"
        "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}"
    );

    // 页面布局：把桌台列表和汇总信息组合成管理页面。
    auto *tableCard = new QWidget(this);
    tableCard->setObjectName("tableCard");
    tableCard->setStyleSheet("QWidget#tableCard{background:#ffffff;border-radius:22px;}");
    auto *cardShadow = new QGraphicsDropShadowEffect(tableCard);
    cardShadow->setBlurRadius(32);
    cardShadow->setOffset(0, 10);
    cardShadow->setColor(QColor(76, 58, 42, 38));
    tableCard->setGraphicsEffect(cardShadow);
    auto *cardLayout = new QVBoxLayout(tableCard);
    cardLayout->setContentsMargins(16, 16, 16, 16);
    cardLayout->addWidget(table);

    summaryLabel->setStyleSheet("font-size:16px;font-weight:600;color:#9b6232;background:transparent;");
    statusLabel->setStyleSheet("font-size:13px;color:#888078;background:transparent;");
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(36, 13, 36, 28);
    layout->setSpacing(16);
    layout->addLayout(top);
    layout->addWidget(summaryLabel);
    layout->addWidget(tableCard, 1);
    layout->addWidget(statusLabel);

    // 数据交互：连接刷新、退出、清台结果以及每三秒自动同步。
    connect(refreshButton, &QPushButton::clicked, this, &AdminTableWidget::refresh);
    connect(logoutButton, &QPushButton::clicked, this, &AdminTableWidget::logoutRequested);

    //接收并显示桌台
    connect(network, &NetworkManager::tableListReceived, this, &AdminTableWidget::showTables);
    connect(network, &NetworkManager::tableListFailed, this, [this](const QString &message) { statusLabel->setText("刷新失败：" + message); });
    connect(network, &NetworkManager::tableCleaned, this, [this](bool success, const QString &message) {
        if (!success) QMessageBox::warning(this, "清台失败", message);
        else QMessageBox::information(this, "清台成功", "桌台已恢复为空闲状态");
        refresh();
    });
    //自动刷新，每三秒向后端查询一次桌台状态。
    timer->setInterval(3000);
    connect(timer, &QTimer::timeout, this, &AdminTableWidget::refresh);
    timer->start();
    refresh();
}

//查询桌台
void AdminTableWidget::refresh()
{
    // 刷新桌台：向后端重新获取客户端正在使用的桌台状态。
    statusLabel->setText("正在同步 Client 使用的桌台状态...");
    network->getTableList();
}

//接收并显示桌台
void AdminTableWidget::showTables(const QList<DiningTable> &tables)
{
    // 列表渲染：为每个桌台生成状态标签，并统计等待清理的桌台数量。
    table->setRowCount(tables.size());
    int dirtyCount = 0;
    const QStringList states{"空闲", "用餐中", "待支付", "待清理"};
    const QStringList stateColors{"#38a857", "#3478b9", "#e1ad22", "#7c858c"};
    for (int row = 0; row < tables.size(); ++row) {
        const DiningTable &item = tables.at(row);
        if (item.status == 3) ++dirtyCount;
        auto setCenteredItem = [this, row](int column, const QString &text) {
            auto *cell = new QTableWidgetItem(text);
            cell->setTextAlignment(Qt::AlignCenter);
            table->setItem(row, column, cell);
        };
        setCenteredItem(0, item.tableName);
        setCenteredItem(1, QString::number(item.capacity));
        setCenteredItem(3, QString::number(item.status));

        // 状态列：把数字状态转换为空闲、用餐中、待支付或待清理标签。
        auto *statusContainer = new QWidget(table);
        statusContainer->setAttribute(Qt::WA_TranslucentBackground, true);
        auto *statusLayout = new QHBoxLayout(statusContainer);
        statusLayout->setContentsMargins(8, 7, 8, 7);
        auto *statusTag = new QLabel(
            item.status >= 0 && item.status < states.size() ? states[item.status] : "未知",
            statusContainer
        );
        statusTag->setAlignment(Qt::AlignCenter);
        statusTag->setFixedSize(92, 32);
        const QString tagColor = item.status >= 0 && item.status < stateColors.size()
            ? stateColors[item.status] : "#8c9297";
        statusTag->setStyleSheet(QString(
            "QLabel{background:%1;color:white;border-radius:15px;font-size:14px;font-weight:700;}"
        ).arg(tagColor));
        statusLayout->addStretch();
        statusLayout->addWidget(statusTag);
        statusLayout->addStretch();
        table->setCellWidget(row, 2, statusContainer);

        // 清台列：只有待清理桌台可以操作，确认后调用后端清台接口。
        auto *operationContainer = new QWidget(table);
        operationContainer->setAttribute(Qt::WA_TranslucentBackground, true);
        auto *operationLayout = new QHBoxLayout(operationContainer);
        operationLayout->setContentsMargins(8, 7, 8, 7);
        auto *button = new QPushButton(item.status == 3 ? "完成清理" : "无需清理", operationContainer);
        button->setFixedSize(112, 36);
        button->setFocusPolicy(Qt::NoFocus);
        button->setCursor(item.status == 3 ? Qt::PointingHandCursor : Qt::ArrowCursor);
        button->setEnabled(item.status == 3);
        button->setStyleSheet(item.status == 3
            ? "QPushButton{background:#df5546;color:white;border:0;border-radius:17px;font-size:14px;font-weight:700;} QPushButton:hover{background:#cf4437;} QPushButton:pressed{background:#b93930;}"
            : "QPushButton{background:#eeeeee;color:#9a9a9a;border:0;border-radius:17px;font-size:14px;}"
        );
        connect(button, &QPushButton::clicked, this, [this, item]() {
            if (QMessageBox::question(this, "确认清台", "确认已完成“" + item.tableName + "”的清理？\n清台后该桌台将变为空闲状态。") == QMessageBox::Yes)
                network->cleanTable(item.id);
        });
        operationLayout->addStretch();
        operationLayout->addWidget(button);
        operationLayout->addStretch();
        table->setCellWidget(row, 4, operationContainer);
    }
    summaryLabel->setText(QString("共 %1 个桌台，%2 个桌台等待清理").arg(tables.size()).arg(dirtyCount));
    statusLabel->setText("最近同步：" + QDateTime::currentDateTime().toString("HH:mm:ss") + "（每 3 秒自动刷新）");
}

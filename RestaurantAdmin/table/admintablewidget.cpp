#include "admintablewidget.h"
#include "../network/networkmanager.h"

#include <QDateTime>
#include <QHeaderView>
#include <QHBoxLayout>
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
    auto *title = new QLabel("桌台管理", this);
    title->setStyleSheet("font-size:30px;font-weight:700;color:#263238;");
    auto *user = new QLabel("管理员：" + username, this);
    auto *refreshButton = new QPushButton("立即刷新", this);
    auto *logoutButton = new QPushButton("退出登录", this);
    auto *top = new QHBoxLayout;
    top->addWidget(title); top->addStretch(); top->addWidget(user);
    top->addWidget(refreshButton); top->addWidget(logoutButton);

    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"桌台", "容纳人数", "当前状态", "状态编号", "操作"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->setStyleSheet("QTableWidget{background:white;alternate-background-color:#f5f8fa;border:1px solid #dbe3e8;font-size:16px;} QHeaderView::section{background:#37474f;color:white;padding:10px;border:0;}");
    summaryLabel->setStyleSheet("font-size:17px;font-weight:600;color:#d84315;");
    statusLabel->setStyleSheet("color:#607d8b;");
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->addLayout(top); layout->addWidget(summaryLabel); layout->addWidget(table, 1); layout->addWidget(statusLabel);

    connect(refreshButton, &QPushButton::clicked, this, &AdminTableWidget::refresh);
    connect(logoutButton, &QPushButton::clicked, this, &AdminTableWidget::logoutRequested);
    connect(network, &NetworkManager::tableListReceived, this, &AdminTableWidget::showTables);
    connect(network, &NetworkManager::tableListFailed, this, [this](const QString &message) { statusLabel->setText("刷新失败：" + message); });
    connect(network, &NetworkManager::tableCleaned, this, [this](bool success, const QString &message) {
        if (!success) QMessageBox::warning(this, "清台失败", message);
        else QMessageBox::information(this, "清台成功", "桌台已恢复为空闲状态");
        refresh();
    });
    timer->setInterval(3000);
    connect(timer, &QTimer::timeout, this, &AdminTableWidget::refresh);
    timer->start();
    refresh();
}

void AdminTableWidget::refresh()
{
    statusLabel->setText("正在同步 Client 使用的桌台状态...");
    network->getTableList();
}

void AdminTableWidget::showTables(const QList<DiningTable> &tables)
{
    table->setRowCount(tables.size());
    int dirtyCount = 0;
    const QStringList states{"空闲", "用餐中", "待结账", "未清理"};
    for (int row = 0; row < tables.size(); ++row) {
        const DiningTable &item = tables.at(row);
        if (item.status == 3) ++dirtyCount;
        table->setItem(row, 0, new QTableWidgetItem(item.tableName));
        table->setItem(row, 1, new QTableWidgetItem(QString::number(item.capacity)));
        table->setItem(row, 2, new QTableWidgetItem(item.status >= 0 && item.status < states.size() ? states[item.status] : "未知"));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(item.status)));
        auto *button = new QPushButton(item.status == 3 ? "完成清理" : "无需清理", table);
        button->setEnabled(item.status == 3);
        button->setStyleSheet(item.status == 3 ? "QPushButton{background:#e64a19;color:white;border:0;border-radius:5px;padding:8px;} QPushButton:hover{background:#d84315;}" : "color:#90a4ae;");
        connect(button, &QPushButton::clicked, this, [this, item]() {
            if (QMessageBox::question(this, "确认清台", "确认已完成“" + item.tableName + "”的清理？\n清台后该桌台将变为空闲状态。") == QMessageBox::Yes)
                network->cleanTable(item.id);
        });
        table->setCellWidget(row, 4, button);
    }
    summaryLabel->setText(QString("共 %1 个桌台，%2 个桌台等待清理").arg(tables.size()).arg(dirtyCount));
    statusLabel->setText("最近同步：" + QDateTime::currentDateTime().toString("HH:mm:ss") + "（每 3 秒自动刷新）");
}

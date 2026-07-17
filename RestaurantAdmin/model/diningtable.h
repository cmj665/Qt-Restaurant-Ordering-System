#ifndef DININGTABLE_H
#define DININGTABLE_H

#include <QString>

// 桌台数据模型：保存后端返回的桌台编号、名称、容量和状态。
struct DiningTable {
    int id = 0;
    QString tableName;
    int capacity = 0;
    int status = 0;
};

#endif

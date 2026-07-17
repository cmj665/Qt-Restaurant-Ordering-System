#include "tablewidget.h"
#include "ui_tablewidget.h"

#include <QDateTime>  // 刷新时打印当前时间
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QScroller> // 触屏滑动滚动
#include <QShowEvent>  // 窗口打开时自动刷新桌台
#include <QVBoxLayout>
#include <QPainter>  // 自定义桌台卡片绘图
#include <QSet>     //过滤后端已删除的桌台
#include <QColor>   // 状态渐变颜色
#include <algorithm>    // std::sort 桌台ID排序
#include <QGraphicsDropShadowEffect>   // 卡片阴影
#include <QRadialGradient>  //径向渐变背景
#include <QFont>


//绘制桌台卡片和不同状态颜色，完全自绘桌台卡片
class TableStatusButton final : public QPushButton
{
public:
    explicit TableStatusButton(QWidget *parent=nullptr):QPushButton(parent)
    {
        setCursor(Qt::PointingHandCursor);  // 鼠标悬浮小手
        setFlat(true);   //取消原生按钮边框
        setAttribute(Qt::WA_TranslucentBackground, true);  // 透明背景，自己绘制渐变
    }

    void setTableData(const DiningTable &value){data=value;update();}

protected:

    bool hitButton(const QPoint &position) const override {
        // 视觉仍保持圆角，但整张桌台卡片都应响应点击，避免顶部圆角区域出现点击盲区。
        return rect().contains(position);
    }

    // 核心绘图逻辑
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        //开启抗锯齿，圆角、渐变、文字边缘不会出现锯齿毛刺，画面更顺滑。
        painter.setRenderHint(QPainter::Antialiasing,true);

        //rect() 获取按钮完整矩形范围
        QRectF card=rect().adjusted(5,5,-5,-5);

        //鼠标按下按钮卡片整体向下偏移 3 像素，模拟按压下沉的视觉效果。
        if(isDown())card.translate(0,3);
        QColor first("#94a3b8"),second("#64748b");QString state="未知";

        //根据桌台状态切换渐变双色与文字
        switch(data.status){
        case 0:first=QColor("#6bcb77");second=QColor("#36a853");state="空闲";break;
        case 1:first=QColor("#4ea8de");second=QColor("#2563a8");state="用餐中";break;
        case 2:first=QColor("#ffd65a");second=QColor("#e7a916");state="待支付";break;
        case 3:first=QColor("#90a4ae");second=QColor("#546e7a");state="完成待清理";break;
        }

        // 鼠标悬停时颜色稍亮，提升交互反馈
        if(underMouse())
        {
            first=first.lighter(112);
            second=second.lighter(108);
        }

        //创建径向渐变（中心向外渐变）：
        //渐变中心点：卡片中心向左 48、向上 35 偏移；渐变半径：卡片宽度的 72%，控制渐变扩散范围。
        QRadialGradient gradient(card.center()-QPointF(48,35),card.width()*.72);
        //渐变规则：0（圆心）：使用浅色 first，1（外圈边缘）：使用深色 second
        gradient.setColorAt(0,first);gradient.setColorAt(1,second);
        //设置画笔填充样式为上面创建的径向渐变，后续图形会用渐变填充。
        painter.setBrush(gradient);
        //设置描边笔：白色、透明度 105、线宽 2，给卡片加一层淡白边框。
        painter.setPen(QPen(QColor(255,255,255,105),2));
        //绘制圆角矩形卡片，宽高圆角半径均为 28
        painter.drawRoundedRect(card, 28, 28);
        //后续文字画笔颜色纯白色。
        painter.setPen(Qt::white);
        //桌台名称字体配置：
        QFont identifier("Arial Rounded MT Bold");identifier.setPixelSize(33);identifier.setWeight(QFont::Black);identifier.setLetterSpacing(QFont::AbsoluteSpacing,1.0);painter.setFont(identifier);
        //绘制桌台名字（如 “1 号桌”）
        painter.drawText(QRectF(0,21,width(),46),Qt::AlignCenter,data.tableName);
        //状态文字字体
        QFont statusFont("Microsoft YaHei UI");statusFont.setPixelSize(17);statusFont.setWeight(QFont::DemiBold);painter.setFont(statusFont);
        painter.drawText(QRectF(0,69,width(),27),Qt::AlignCenter,state);
        //人数文字配置
        QFont capacityFont("Microsoft YaHei UI");capacityFont.setPixelSize(14);painter.setFont(capacityFont);painter.setPen(QColor(255,255,255,205));
        painter.drawText(QRectF(0,97,width(),23),Qt::AlignCenter,QString("%1人桌").arg(data.capacity));
    }
private: DiningTable data{};
};



TableWidget::TableWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TableWidget)
    , network(new NetworkManager(this))
    , graphicsView(new QGraphicsView(this))   // 画布视图（显示桌台卡片）
    , scene(new QGraphicsScene(this))       // 画布场景（承载所有桌台卡片代理控件）
    , refreshButton(new QPushButton("立即刷新", this))
    , refreshStatusLabel(new QLabel("正在获取桌台状态...", this))
    , refreshTimer(new QTimer(this))    // 2秒自动刷新定时器
{
    ui->setupUi(this);
    //允许窗口使用样式表设置背景渐变，否则自定义背景不生效。
    setAttribute(Qt::WA_StyledBackground,true);
    //给整个窗口设置对角线性渐变米色背景，从左上浅米过渡到右下深米。
    setStyleSheet("TableWidget{background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #fffaf0,stop:0.55 #f8f0df,stop:1 #f3e7d2);}");
    ui->titleLabel->setText("桌台状态中心");
    ui->titleLabel->setAlignment(Qt::AlignCenter);
    ui->titleLabel->setStyleSheet("font-size:38px;font-weight:900;color:#5b3a22;padding:8px;background:transparent;");

    //视图绑定场景，scene 里的桌台卡片才能显示出来
    graphicsView->setScene(scene);
    //开启抗锯齿，圆角、渐变、文字无锯齿，画面更顺滑
    graphicsView->setRenderHint(QPainter::Antialiasing, true);
    //去掉视图默认边框
    graphicsView->setFrameShape(QFrame::NoFrame);
    //视图背景透明，不遮挡窗口渐变底色
    graphicsView->setBackgroundBrush(Qt::NoBrush);
    //样式强制视图及内部子控件全部透明
    graphicsView->setStyleSheet("QGraphicsView{background:transparent;border:none;} QGraphicsView>QWidget{background:transparent;}");
    graphicsView->viewport()->setStyleSheet("background:transparent;");

    graphicsView->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    //永久隐藏横向滚动条
    graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //纵向滚动条仅内容超出可视区域时才显示
    graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    //鼠标按住画布可拖拽滑动，鼠标变成小手
    graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    //视口接收触屏触摸事件
    graphicsView->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
    //开启触屏惯性滑动，手机 / 触摸屏可以像网页一样滑画布
    QScroller::grabGesture(graphicsView->viewport(), QScroller::TouchGesture);

    //刷新按钮样式配置
    refreshButton->setMinimumSize(118, 42);
    refreshButton->setCursor(Qt::PointingHandCursor);
    refreshButton->setStyleSheet(
        "QPushButton{background:#f2a14a;color:white;border:1px solid #e58a2d;border-radius:12px;font-size:16px;font-weight:700;padding:7px 15px;}"
        "QPushButton:hover{background:#eb9132;} QPushButton:pressed{background:#dc7d22;}"
    );
    refreshStatusLabel->setStyleSheet("font-size:14px;color:#7a5335;background:rgba(255,255,255,150);border:1px solid #ead8bd;border-radius:12px;padding:7px 14px;");

    //顶部工具栏布局
    QGridLayout *toolbar = new QGridLayout;
    toolbar->setContentsMargins(0, 0, 0, 0);
    //第 0 列、第 2 列自动拉伸占空白，中间第 1 列放文字，右侧第 2 列放按钮。
    toolbar->setColumnStretch(0, 1);
    toolbar->setColumnStretch(2, 1);
    toolbar->addWidget(refreshStatusLabel, 0, 1, Qt::AlignCenter);
    toolbar->addWidget(refreshButton, 0, 2, Qt::AlignRight | Qt::AlignVCenter);
    // 生成四个状态色块
    createLegend();

    //主垂直布局（整个窗口排版）
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(48, 24, 48, 24);
    //各个组件之间间距 12 像素
    mainLayout->setSpacing(12);
    mainLayout->addWidget(ui->titleLabel);
    mainLayout->addLayout(toolbar);
    //绘图画布，拉伸系数 1，占用页面绝大部分空间。
    mainLayout->addWidget(graphicsView, 1);
    //底部状态颜色图例。
    mainLayout->addWidget(legendWidget);
    setLayout(mainLayout);

    // .ui 中遗留的 tableContainer 已被上面的 QGraphicsView 取代。
    // 它采用绝对定位覆盖在左上角，会透明地截获第一张桌台卡片上半部分的鼠标事件。
    ui->tableContainer->setAttribute(Qt::WA_TransparentForMouseEvents, true);  //旧容器允许鼠标穿透，不会挡住画布点击。
    ui->tableContainer->hide();  //隐藏旧废弃容器
    ui->widget->setAttribute(Qt::WA_TransparentForMouseEvents, true);   //ui 内其他空白容器也穿透鼠标。
    ui->widget->lower();  //把旧容器放到底层，避免遮挡画布
    ui->titleLabel->show();  //保证标题正常显示。


    //每2秒自动刷新
    //手动刷新
    connect(refreshButton, &QPushButton::clicked, this, &TableWidget::refreshTables);
    // refreshTables() → 网络请求 → 数据回调 showTables()
    connect(network, &NetworkManager::tableListReceived, this, &TableWidget::showTables);
    //定时器间隔 2000 毫秒 = 2 秒
    refreshTimer->setInterval(2000);
    //每 2 秒超时自动触发刷新。
    connect(refreshTimer, &QTimer::timeout, this, &TableWidget::refreshTables);
    //启动定时器，自动刷新开始运行。
    refreshTimer->start();
    //窗口打开立刻执行一次刷新，加载初始桌台数据。
    refreshTables();
}


//窗口显示事件，每次打开 / 切换到桌台页面，立刻刷新最新桌台数据，保证状态实时
void TableWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    refreshTables();
}


//刷新  refreshTables() → 网络请求 → 数据回调 showTables()
void TableWidget::refreshTables()
{
    refreshButton->setEnabled(false);                       //刷新按钮置灰不可重复点击
    refreshStatusLabel->setText("正在刷新桌台状态...");
    network->getTableList();
}


//更新画布上所有桌台卡片
void TableWidget::showTables(QList<DiningTable> tables)
{
    //创建集合
    QSet<int> receivedIds;
    for(const DiningTable &table : tables)
    {
        receivedIds.insert(table.id);
        //调用方法：有该桌台卡片就更新状态颜色，没有就新建桌台卡片。
        updateTableItem(table);
    }

    //取出当前画布上已经存在的所有桌台 ID
    const QList<int> existingIds = tableItems.keys();
    for(int id : existingIds)
    {
        //本次后端返回数据里没有这个 ID → 代表后端已删除该桌台，界面需要删掉卡片
        if(!receivedIds.contains(id))
        {
            scene->removeItem(tableItems.take(id));
            delete tableButtons.take(id);
        }
    }


    //调用排版函数，把所有存活的桌台卡片按 3 列自动整齐排列，重新计算坐标
    arrangeTableItems();
    //刷新完成，解锁「立即刷新」按钮，允许用户再次点击手动刷新。
    refreshButton->setEnabled(true);
    refreshStatusLabel->setText(
        QString("触屏可滑动 · 自动刷新2秒 · %1").arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
    );
}

//根据后端传来的单条桌台数据，创建 / 更新桌台卡片按钮
//不存在卡片：新建TableStatusButton、添加画布、存入缓存
//存在卡片：更新桌台数据，刷新卡片颜色、文字
//重新绑定按钮点击事件
void TableWidget::updateTableItem(const DiningTable &table)
{
    QPushButton *button = tableButtons.value(table.id, nullptr);
    //存在该桌台卡片(按钮) → 新建自定义桌台按钮、放到画布、加阴影、存入缓存
    if(!button)
    {
        button = new TableStatusButton;
        button->setFixedSize(280, 144);
        button->setProperty("tableId", table.id);
        //QGraphicsScene 不能直接放普通 QWidget，必须用代理包装；scene->addWidget()：把按钮包装成画布代理控件，添加到绘图场景。
        QGraphicsProxyWidget *proxy = scene->addWidget(button);
        button->show();
        proxy->setVisible(true);
        proxy->setOpacity(1.0);   //不透明，完全显示卡片。
        auto *shadow=new QGraphicsDropShadowEffect;
        //阴影参数，模糊半径 28（柔和大阴影）；偏移向下 10 像素，模拟悬浮；棕褐色半透明阴影。
        shadow->setBlurRadius(28);shadow->setOffset(0,10);shadow->setColor(QColor(116,62,25,85));
        proxy->setGraphicsEffect(shadow);           //给卡片代理绑定阴影效果。
        proxy->setCacheMode(QGraphicsItem::NoCache); //关闭缓存，每次状态变更实时重绘渐变、颜色，避免刷新卡顿 / 颜色不更新。
        tableButtons.insert(table.id, button);   //存入按钮缓存，key = 桌台 ID，方便下次刷新复用。
        tableItems.insert(table.id, proxy); //存入画布代理缓存，key = 桌台 ID，后续排版 / 删除需要用到。
    }

    //普通 QPushButton 强制转为自定义 TableStatusButton，调用 setTableData() 把最新桌台数据传入按钮；
    static_cast<TableStatusButton *>(button)->setTableData(table);
    //断开当前 this 与该按钮所有旧的点击信号槽；
    //防止多次刷新重复绑定 click，造成点击一次触发多次逻辑
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

//桌台自动排版
void TableWidget::arrangeTableItems()
{
    QList<int> ids = tableItems.keys();
    std::sort(ids.begin(), ids.end());
    constexpr int columns = 3;
    //设置单张卡片的宽和高
    constexpr qreal cardWidth = 280;
    constexpr qreal cardHeight = 144;
    //设置卡片之间的水平和垂直距离
    constexpr qreal horizontalGap = 32;
    constexpr qreal verticalGap = 24;
    constexpr qreal margin = 32;

    for(int index = 0; index < ids.size(); ++index)
    {
        const int row = index / columns;
        const int col = index % columns;
        //给当前桌台代理控件设置场景坐标
        tableItems[ids[index]]->setPos(
            margin + col * (cardWidth + horizontalGap),
            margin + row * (cardHeight + verticalGap));
    }
    const int rows = (ids.size() + columns - 1) / columns;
    //设置画布 Scene 的可视范围：
    scene->setSceneRect(0, 0,
        margin * 2 + columns * cardWidth + (columns - 1) * horizontalGap,
        margin * 2 + qMax(1, rows) * cardHeight + qMax(0, rows - 1) * verticalGap);
}

// 生成状态图例色块标签
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

//创建底部状态图例面版 ，什么颜色表示什么状态
void TableWidget::createLegend()
{
    legendWidget = new QWidget(this);
    legendWidget->setStyleSheet("QWidget{background:rgba(255,255,255,150);border:1px solid #e6d2b4;border-radius:18px;} QLabel{border:none;}");
    QHBoxLayout *legend = new QHBoxLayout(legendWidget);  //横向排列
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

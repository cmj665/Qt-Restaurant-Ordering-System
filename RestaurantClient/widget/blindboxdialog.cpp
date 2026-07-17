#include "blindboxdialog.h"
#include <QLabel>
#include <QPainter>   //绘图类画盲盒
#include <QPushButton>
#include <QVariantAnimation>    //动画类
#include <QVBoxLayout>
#include <QEasingCurve>     //缓动曲线
#include <QTimer>
#include <QtMath>

//负责画盲盒动画（纯绘图）
class BlindBoxCanvas:public QWidget {

public:
    explicit BlindBoxCanvas(QWidget*p=nullptr):QWidget(p){
        setMinimumSize(420,330);
    }

    //更新动画进度
    void setProgress(qreal value){
        progress=value;
        update();
    }

protected:
    //动画核心
    void paintEvent(QPaintEvent*)override{
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);  //开启抗锯齿，边缘更平滑。
        p.fillRect(rect(),QColor("#111827"));   //把整个背景刷成深蓝黑色
        //计算盒子中心点，放在窗口中心向下45像素
        const QPointF c(width()/2.0,height()/2.0+45);
        //计算左右摇晃距离                    ，*(1-progress)越到后面：摇得越轻
        const qreal shake=progress<.45?qSin(progress*75)*12*(1-progress):0;
        //整个坐标系左右移动，所以盒子一起摇。
        p.translate(shake,0);
        //准备喷彩带
        if(progress>.55)
        {
            p.setPen(Qt::NoPen);
            //画22个粒子
            for(int i=0;i<22;++i){
                //计算角度
                qreal a=i*6.283/22;
                //粒子越来越远
                qreal r=(progress-.55)*260;
                //彩带金色粉色交替
                QColor color=i%2?QColor("#fbbf24"):QColor("#f472b6");
                //透明度越来越高
                color.setAlphaF(qMin(1.0,(progress-.5)*2));
                p.setBrush(color);
                //画小圆，就是彩带粒子
                p.drawEllipse(c+QPointF(qCos(a)*r,qSin(a)*r*.65),4,4);
            }
        }

        //盒子主体
        //创造渐变
        QLinearGradient front(c.x()-100,c.y()-70,c.x()+100,c.y()+100);
        front.setColorAt(0,QColor("#ec4899"));
        front.setColorAt(1,QColor("#9d174d"));
        //填充渐变
        p.setBrush(front);
        //边框颜色
        p.setPen(QPen(QColor("#f9a8d4"),2));
        //画盒子正面六边形
        p.drawPolygon(QPolygonF{c+QPointF(-105,-55),c+QPointF(70,-55),c+QPointF(105,-25),c+QPointF(105,85),c+QPointF(-70,85),c+QPointF(-105,55)});

        //画右侧面
        p.setBrush(QColor("#701a75"));
        p.drawPolygon(QPolygonF{c+QPointF(70,-55),c+QPointF(105,-25),c+QPointF(105,85),c+QPointF(70,55)});
        //画金色丝带
        p.setBrush(QColor("#f59e0b"));
        p.setPen(Qt::NoPen);
        p.drawRect(QRectF(c.x()-18,c.y()-55,38,140));

        //盒盖动画，盒子向上升
        qreal lift=progress>.42?(progress-.42)*190:0;
        //旋转角度
        qreal tilt=progress>.42?(progress-.42)*24:0;
        p.save();  //保存画笔状态
        p.translate(c.x(),c.y()-68-lift);  //移到盖子中心
        p.rotate(-tilt);  //盒盖旋转
        QLinearGradient lid(-115,-25,90,35);  //盒子渐变
        lid.setColorAt(0,QColor("#f472b6"));   //画盖子，渐变起点，渐变终点
        lid.setColorAt(1,QColor("#be185d"));
        p.setBrush(lid);
        p.setPen(QPen(QColor("#fbcfe8"),2));  //描边
        //绘制外轮廓多边形（礼盒盖子整体外框）
        p.drawPolygon(QPolygonF{QPointF(-118,0),QPointF(-78,-34),QPointF(86,-34),QPointF(120,0),QPointF(78,28),QPointF(-80,28)});p.setBrush(QColor("#fbbf24"));p.drawPolygon(QPolygonF{QPointF(-18,-34),QPointF(18,-34),QPointF(28,28),QPointF(-28,28)});
        p.restore();
        //画星星
        if(progress>.72)
        {
            p.setPen(QColor("#fff7ed"));
            QFont f=p.font();
            f.setPixelSize(42);  //42像素。
            f.setBold(true);
            p.setFont(f);
            //画星星，文字绘制矩形区域 x=0，y=35：纵向从 35 像素位置开始，宽度等于当前控件完整宽度，文字区域高度 70 像素
            p.drawText(QRectF(0,35,width(),70),Qt::AlignCenter,"✨");}
    }

private:qreal progress=0;
};

//负责弹窗逻辑
BlindBoxDialog::BlindBoxDialog(int chances,
                               std::function<void()> action,
                               QWidget*parent):QDialog(parent),
                                canvas(new BlindBoxCanvas(this)), //创建动画区域
                                title(new QLabel(this)),
                                result(new QLabel(this)),
                                openButton(new QPushButton("开启盲盒",this)),
                                remainingChances(chances),
                                drawAction(std::move(action)), //保存外部传入的：真正抽奖函数
                                animation(new QVariantAnimation(this)){
    setWindowTitle("满额盲盒");
    setMinimumSize(500,520);
    setStyleSheet("QDialog{background:#111827;}"
                  " QLabel{color:white;} "
                  "QPushButton{background:#f59e0b;color:#111827;"
                  "border:0;border-radius:12px;padding:14px;"
                  "font-size:20px;font-weight:bold;} "
                  "QPushButton:disabled{background:#6b7280;}");

    title->setText(QString("每满200元抽一次 · 剩余 %1 次").arg(chances));
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size:22px;font-weight:bold;");

    result->setAlignment(Qt::AlignCenter);
    result->setWordWrap(true);
    result->setStyleSheet("font-size:25px;color:#fde68a;font-weight:bold;min-height:55px;");

    //布局
    auto*layout=new QVBoxLayout(this);
    layout->setContentsMargins(25,20,25,25);layout->addWidget(title);
    layout->addWidget(canvas,1);
    layout->addWidget(result);
    layout->addWidget(openButton);

    //动画
    animation->setDuration(1700);  //时长
    animation->setStartValue(0.0); //起点
    animation->setEndValue(1.0);   //终点
    animation->setEasingCurve(QEasingCurve::OutBack); //设置动画缓动曲线，动画结束阶段有回弹效果

    //动画每更新一次就刷新画面
    connect(animation,&QVariantAnimation::valueChanged,
            this,
            [this](const QVariant&v){
                canvas->setProgress(v.toReal());
    });

    connect(animation,&QVariantAnimation::finished,
            this,[this](){
        finishOpeningIfReady();
    });
    connect(openButton,&QPushButton::clicked,this,[this](){startOpening();});
}


void BlindBoxDialog::startOpening(){
    if(opening||remainingChances<=0)  //防止重复点击/没次数
        return;
    opening=true;
    rewardReady=false;   //奖励还没回来
    pendingReward.clear();  //清空奖励
    animation->stop();      //停止旧动画
    openButton->setEnabled(false);result->setText("准备开启...");
    canvas->setProgress(0);
    //90ms后执行
    QTimer::singleShot(90,this,[this](){
        if(!opening)return;
        result->setText("盲盒开启中...");
        animation->setCurrentTime(0);
        animation->start();
        if(drawAction)drawAction();
    });
}


void BlindBoxDialog::showReward(const QString&name,int remaining){
    pendingReward=name;
    pendingRemaining=remaining;
    rewardReady=true;   //标记奖励到了
    if(animation->state()!=QAbstractAnimation::Running)  //动画不在运行状态
        finishOpeningIfReady();
}


void BlindBoxDialog::showError(const QString&message){
    animation->stop();
    opening=false;
    rewardReady=false;
    canvas->setProgress(0);
    result->setText("开启失败："+message);
    openButton->setText("重新开启");
    openButton->setEnabled(remainingChances>0);
}

void BlindBoxDialog::finishOpeningIfReady(){
    if(!opening)
        return;
    //如果服务器还没返回，等待
    if(!rewardReady){
        result->setText("正在揭晓奖励...");
        return;
    }
    //动画结束，剩余次数=待更新的剩余抽奖次数
    remainingChances=pendingRemaining;
    result->setText("🎉 恭喜获得："+pendingReward);
    title->setText(QString("剩余 %1 次机会").arg(remainingChances));
    openButton->setText(remainingChances>0?"再开一个":"机会已用完");
    opening=false;
    openButton->setEnabled(remainingChances>0);
}

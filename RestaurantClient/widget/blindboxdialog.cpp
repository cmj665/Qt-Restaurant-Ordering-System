#include "blindboxdialog.h"
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QRandomGenerator>
#include <QVariantAnimation>
#include <QVBoxLayout>
#include <QEasingCurve>
#include <QTimer>
#include <QtMath>

class BlindBoxCanvas:public QWidget {

public:
    explicit BlindBoxCanvas(QWidget*p=nullptr):QWidget(p){
        setMinimumSize(420,330);
    }

    void setProgress(qreal value){
        progress=value;update();
    }

protected:
    void paintEvent(QPaintEvent*)override{
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(),QColor("#111827"));

        const QPointF c(width()/2.0,height()/2.0+45);
        const qreal shake=progress<.45?qSin(progress*75)*12*(1-progress):0;
        p.translate(shake,0);
        if(progress>.55)
        {
            p.setPen(Qt::NoPen);
            for(int i=0;i<22;++i){
                qreal a=i*6.283/22;
                qreal r=(progress-.55)*260;
                QColor color=i%2?QColor("#fbbf24"):QColor("#f472b6");
                color.setAlphaF(qMin(1.0,(progress-.5)*2));
                p.setBrush(color);
                p.drawEllipse(c+QPointF(qCos(a)*r,qSin(a)*r*.65),4,4);
            }
        }

        QLinearGradient front(c.x()-100,c.y()-70,c.x()+100,c.y()+100);
        front.setColorAt(0,QColor("#ec4899"));
        front.setColorAt(1,QColor("#9d174d"));
        p.setBrush(front);
        p.setPen(QPen(QColor("#f9a8d4"),2));
        p.drawPolygon(QPolygonF{c+QPointF(-105,-55),c+QPointF(70,-55),c+QPointF(105,-25),c+QPointF(105,85),c+QPointF(-70,85),c+QPointF(-105,55)});

        p.setBrush(QColor("#701a75"));
        p.drawPolygon(QPolygonF{c+QPointF(70,-55),c+QPointF(105,-25),c+QPointF(105,85),c+QPointF(70,55)});
        p.setBrush(QColor("#f59e0b"));
        p.setPen(Qt::NoPen);
        p.drawRect(QRectF(c.x()-18,c.y()-55,38,140));
        qreal lift=progress>.42?(progress-.42)*190:0;
        qreal tilt=progress>.42?(progress-.42)*24:0;
        p.save();p.translate(c.x(),c.y()-68-lift);
        p.rotate(-tilt);QLinearGradient lid(-115,-25,90,35);
        lid.setColorAt(0,QColor("#f472b6"));
        lid.setColorAt(1,QColor("#be185d"));
        p.setBrush(lid);p.setPen(QPen(QColor("#fbcfe8"),2));
        p.drawPolygon(QPolygonF{QPointF(-118,0),QPointF(-78,-34),QPointF(86,-34),QPointF(120,0),QPointF(78,28),QPointF(-80,28)});p.setBrush(QColor("#fbbf24"));p.drawPolygon(QPolygonF{QPointF(-18,-34),QPointF(18,-34),QPointF(28,28),QPointF(-28,28)});
        p.restore();
        if(progress>.72)
        {
            p.setPen(QColor("#fff7ed"));
            QFont f=p.font();
            f.setPixelSize(42);
            f.setBold(true);
            p.setFont(f);
            p.drawText(QRectF(0,35,width(),70),Qt::AlignCenter,"✨");}
    }

private:qreal progress=0;
};

BlindBoxDialog::BlindBoxDialog(int chances,std::function<void()> action,QWidget*parent):QDialog(parent),canvas(new BlindBoxCanvas(this)),title(new QLabel(this)),result(new QLabel(this)),openButton(new QPushButton("开启盲盒",this)),remainingChances(chances),drawAction(std::move(action)),animation(new QVariantAnimation(this)){
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

    auto*layout=new QVBoxLayout(this);
    layout->setContentsMargins(25,20,25,25);layout->addWidget(title);
    layout->addWidget(canvas,1);
    layout->addWidget(result);
    layout->addWidget(openButton);

    animation->setDuration(1700);animation->setStartValue(0.0);animation->setEndValue(1.0);animation->setEasingCurve(QEasingCurve::OutBack);connect(animation,&QVariantAnimation::valueChanged,this,[this](const QVariant&v){canvas->setProgress(v.toReal());});
    connect(animation,&QVariantAnimation::finished,this,[this](){finishOpeningIfReady();});
    connect(openButton,&QPushButton::clicked,this,[this](){startOpening();});
}
void BlindBoxDialog::startOpening(){
    if(opening||remainingChances<=0)return;
    opening=true;
    rewardReady=false;
    pendingReward.clear();
    animation->stop();
    openButton->setEnabled(false);result->setText("准备开启...");
    canvas->setProgress(0);
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
    rewardReady=true;
    if(animation->state()!=QAbstractAnimation::Running)finishOpeningIfReady();
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
    if(!opening)return;
    if(!rewardReady){
        result->setText("正在揭晓奖励...");
        return;
    }
    remainingChances=pendingRemaining;
    result->setText("🎉 恭喜获得："+pendingReward);
    title->setText(QString("剩余 %1 次机会").arg(remainingChances));
    openButton->setText(remainingChances>0?"再开一个":"机会已用完");
    opening=false;
    openButton->setEnabled(remainingChances>0);
}

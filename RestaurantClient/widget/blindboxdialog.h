#ifndef BLINDBOXDIALOG_H
#define BLINDBOXDIALOG_H
#include <QDialog>
#include <functional>

class QLabel;
class QPushButton;
class QVariantAnimation;
class BlindBoxCanvas;
/**
 * @brief 支付后奖励抽取对话框。
 *
 * 动画结束和服务端抽奖结果是两个独立的异步事件，只有二者均完成后才展示
 * 最终奖励，避免动画期间提前泄露结果或重复触发抽奖请求。
 */
class BlindBoxDialog:public QDialog {
public:
    BlindBoxDialog(int chances,std::function<void()> drawAction,QWidget *parent=nullptr);

    void showReward(const QString &name,int remaining);
    void showError(const QString &message);

private:
    BlindBoxCanvas *canvas;
    QLabel *title;
    QLabel *result; QPushButton *openButton;
    int remainingChances;
    std::function<void()> drawAction;
    QVariantAnimation *animation;
    bool opening = false;
    bool rewardReady = false;
    QString pendingReward;
    int pendingRemaining = 0;

private:
    void startOpening();
    void finishOpeningIfReady();
};
#endif

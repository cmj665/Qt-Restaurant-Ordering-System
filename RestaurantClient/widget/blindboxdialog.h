#ifndef BLINDBOXDIALOG_H
#define BLINDBOXDIALOG_H
#include <QDialog>
#include <functional>

class QLabel;
class QPushButton;
class QVariantAnimation;
class BlindBoxCanvas;
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

private:
    void startOpening();
};
#endif

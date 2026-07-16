#ifndef COVERWIDGET_H
#define COVERWIDGET_H

#include <QWidget>

class QLabel;
class QPushButton;

class CoverWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit CoverWidget(QWidget *parent = nullptr);

signals:
    void startOrdering();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QLabel *makeImage(const QString &resourcePath);
    void placeImage(QLabel *label, const QString &resourcePath, const QRect &bounds, bool outlined = false);
    void updateLayout();

    QLabel *brandIcon;
    QLabel *mascot;
    QLabel *title;
    QLabel *subtitle;
    QLabel *fishPot;
    QLabel *chef;
    QLabel *receipt;
    QLabel *cart;
    QLabel *payment;
    QPushButton *startButton;
};

#endif

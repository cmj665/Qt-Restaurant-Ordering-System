#include "coverwidget.h"

#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QResizeEvent>

class PillButton final : public QPushButton
{
public:
    explicit PillButton(const QString &text, QWidget *parent = nullptr)
        : QPushButton(text, parent)
    {
        setCursor(Qt::PointingHandCursor);
        setFlat(true);
        setMouseTracking(true);
        setAttribute(Qt::WA_TranslucentBackground, true);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        QRectF buttonRect = rect().adjusted(3, 3, -3, -3);
        if (isDown())
            buttonRect.translate(0, 2);

        QLinearGradient gradient(buttonRect.topLeft(), buttonRect.topRight());
        if (isDown()) {
            gradient.setColorAt(0, QColor("#f18428"));
            gradient.setColorAt(1, QColor("#dc281f"));
        } else if (underMouse()) {
            gradient.setColorAt(0, QColor("#ffb34f"));
            gradient.setColorAt(1, QColor("#ff4538"));
        } else {
            gradient.setColorAt(0, QColor("#ff9f32"));
            gradient.setColorAt(1, QColor("#f22f25"));
        }

        painter.setBrush(gradient);
        painter.setPen(QPen(QColor(255, 255, 255, 145), 2));
        const qreal radius = buttonRect.height() / 2.0;
        painter.drawRoundedRect(buttonRect, radius, radius);

        QFont buttonFont("Microsoft YaHei UI");
        buttonFont.setPixelSize(qMax(28, qRound(height() * 0.39)));
        buttonFont.setWeight(QFont::Black);
        painter.setFont(buttonFont);
        painter.setPen(Qt::white);
        painter.drawText(buttonRect, Qt::AlignCenter, text());
    }
};

CoverWidget::CoverWidget(QWidget *parent)
    : QWidget(parent)
    , brandIcon(makeImage(":/cover/brand.png"))
    , mascot(makeImage(":/cover/mascot.png"))
    , title(new QLabel(QStringLiteral("大丰收"), this))
    , subtitle(new QLabel(QStringLiteral("平板点餐系统"), this))
    , fishPot(makeImage(":/cover/fish-pot.png"))
    , chef(makeImage(":/cover/chef.png"))
    , receipt(makeImage(":/cover/receipt.png"))
    , cart(makeImage(":/cover/cart.png"))
    , payment(makeImage(":/cover/payment.png"))
    , startButton(new PillButton(QStringLiteral("点击开始点餐"), this))
{
    setMinimumSize(1000, 680);
    title->setAlignment(Qt::AlignCenter);
    subtitle->setAlignment(Qt::AlignCenter);

    connect(startButton, &QPushButton::clicked, this, &CoverWidget::startOrdering);
    updateLayout();
}

QLabel *CoverWidget::makeImage(const QString &resourcePath)
{
    auto *label = new QLabel(this);
    label->setPixmap(QPixmap(resourcePath));
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("background:transparent;");
    label->setAttribute(Qt::WA_TransparentForMouseEvents);
    return label;
}

void CoverWidget::placeImage(QLabel *label, const QString &resourcePath, const QRect &bounds, bool outlined)
{
    label->setGeometry(bounds);
    const int outline = outlined ? qMax(4, qRound(qMin(width() / 1600.0, height() / 1000.0) * 7)) : 0;
    const QSize target(qMax(1, bounds.width() - outline * 2), qMax(1, bounds.height() - outline * 2));
    const QPixmap source = QPixmap(resourcePath).scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (!outlined) {
        label->setPixmap(source);
        return;
    }

    QPixmap silhouette(source.size());
    silhouette.fill(Qt::transparent);
    {
        QPainter maskPainter(&silhouette);
        maskPainter.drawPixmap(0, 0, source);
        maskPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        maskPainter.fillRect(silhouette.rect(), Qt::white);
    }
    QPixmap result(source.width() + outline * 2, source.height() + outline * 2);
    result.fill(Qt::transparent);
    QPainter outlinePainter(&result);
    outlinePainter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    for (int x = -outline; x <= outline; ++x) {
        for (int y = -outline; y <= outline; ++y) {
            if (x * x + y * y <= outline * outline)
                outlinePainter.drawPixmap(outline + x, outline + y, silhouette);
        }
    }
    outlinePainter.drawPixmap(outline, outline, source);
    label->setPixmap(result);
}

void CoverWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateLayout();
}

void CoverWidget::updateLayout()
{
    const qreal sx = width() / 1600.0;
    const qreal sy = height() / 1000.0;
    auto rect = [sx, sy](int x, int y, int w, int h) {
        return QRect(qRound(x * sx), qRound(y * sy), qRound(w * sx), qRound(h * sy));
    };

    placeImage(brandIcon, ":/cover/brand.png", rect(52, 45, 125, 115));
    title->setGeometry(rect(315, 73, 970, 175));
    subtitle->setGeometry(rect(500, 232, 600, 68));
    placeImage(fishPot, ":/cover/fish-pot.png", rect(270, 225, 1020, 610));
    placeImage(chef, ":/cover/chef.png", rect(185, 305, 190, 245));
    placeImage(payment, ":/cover/payment.png", rect(285, 620, 175, 160));

    // 吉祥物贴近鱼锅右下沿，形成正在品尝鱼锅的画面。
    placeImage(mascot, ":/cover/mascot.png", rect(1025, 610, 300, 255));
    // 右侧图标拆分排列：购物车在上，单据位于它的左下方。
    placeImage(cart, ":/cover/cart.png", rect(1135, 285, 155, 135));
    placeImage(receipt, ":/cover/receipt.png", rect(1305, 420, 150, 180));
    startButton->setGeometry(rect(620, 790, 360, 94));

    title->setStyleSheet(QString("color:#fff9e8;background:transparent;font-family:'Microsoft YaHei';font-size:%1px;font-weight:900;letter-spacing:%2px;")
        .arg(qMax(82, qRound(142 * qMin(sx, sy)))).arg(qMax(4, qRound(11 * sx))));
    subtitle->setStyleSheet(QString("color:#fff8e8;background:transparent;font-family:'Microsoft YaHei UI';font-size:%1px;font-weight:700;letter-spacing:%2px;")
        .arg(qMax(28, qRound(42 * qMin(sx, sy)))).arg(qMax(3, qRound(7 * sx))));
}

void CoverWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QLinearGradient background(0, 0, 0, height());
    background.setColorAt(0.0, QColor("#f23822"));
    background.setColorAt(0.34, QColor("#ff7840"));
    background.setColorAt(0.67, QColor("#ffd39a"));
    background.setColorAt(1.0, QColor("#fff9de"));
    painter.fillRect(rect(), background);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 241, 187, 55));
    painter.drawEllipse(QPointF(width() * .28, height() * .48), width() * .14, width() * .14);
    painter.drawEllipse(QPointF(width() * .78, height() * .43), width() * .13, width() * .13);
    painter.setBrush(QColor(255, 255, 233, 185));
    QPainterPath glow;
    glow.moveTo(0, height() * .62);
    glow.cubicTo(width() * .18, height() * .79, width() * .36, height() * .61, width() * .53, height() * .72);
    glow.cubicTo(width() * .72, height() * .84, width() * .9, height() * .65, width(), height() * .76);
    glow.lineTo(width(), height()); glow.lineTo(0, height()); glow.closeSubpath();
    painter.drawPath(glow);
}

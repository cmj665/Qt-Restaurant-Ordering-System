#ifndef CARTWIDGET_H
#define CARTWIDGET_H

#include <QList>
#include <QMap>
#include <QWidget>
#include <QHash>
#include <QPixmap>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include "../model/cartitem.h"
#include "../model/dish.h"

class QCloseEvent;
class QResizeEvent;
class QPaintEvent;
class QShowEvent;
class QListWidget;
class QLabel;
class QTabWidget;
class NetworkManager;

QT_BEGIN_NAMESPACE
namespace Ui { class CartWidget; }
QT_END_NAMESPACE

class CartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CartWidget(int tableId, QWidget *parent = nullptr);
    ~CartWidget();

    void addDish(const Dish &dish);
    void decreaseDish(int dishId);
    int quantityForDish(int dishId) const;
    QList<CartItem> cartItems() const;
    double totalPrice() const;
    void clearCart();
    void setSubmitting(bool submitting);
    void refreshOrderedOrder();

signals:
    void submitOrderRequested(const QList<CartItem> &items);
    void checkoutRequested();
    void detailRequested();
    void quantityChanged(int dishId, int quantity);
    void closeRequested();
    void totalChanged(double total, int itemCount);

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    Ui::CartWidget *ui;
    QMap<int, CartItem> m_cart;
    bool submitting = false;
    QNetworkAccessManager *imageManager;
    QHash<int, QPixmap> imageCache;
    int tableId;
    NetworkManager *orderNetwork;
    QTabWidget *tabWidget;
    QListWidget *orderedListWidget;
    QLabel *orderedSummaryLabel;

    void refreshCart();
    void updateButtons();
    void removeCurrentDish();
    void changeQuantity(int dishId, int delta);
    void renderOrderedOrder(bool success, const QJsonObject &data);
};

#endif // CARTWIDGET_H

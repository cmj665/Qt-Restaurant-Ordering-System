#ifndef DISHEDITDIALOG_H
#define DISHEDITDIALOG_H
#include <QDialog>
#include "../model/dish.h"
class QComboBox; class QDoubleSpinBox; class QLineEdit; class QSpinBox; class QTextEdit;
class DishEditDialog : public QDialog {
public: DishEditDialog(const QList<DishCategory> &categories, const Dish *dish=nullptr, QWidget *parent=nullptr); Dish value() const;
private: int dishId=0; QComboBox *category; QLineEdit *name; QDoubleSpinBox *price; QSpinBox *stock; QLineEdit *picture; QTextEdit *description; };
#endif

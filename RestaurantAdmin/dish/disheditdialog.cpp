#include "disheditdialog.h"
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
DishEditDialog::DishEditDialog(const QList<DishCategory> &categories,const Dish *dish,QWidget *parent)
    :QDialog(parent),category(new QComboBox(this)),name(new QLineEdit(this)),price(new QDoubleSpinBox(this)),stock(new QSpinBox(this)),picture(new QLineEdit(this)),description(new QTextEdit(this)) {
    setWindowTitle(dish?"修改菜品":"新增菜品"); setMinimumWidth(480);
    for(const auto &item:categories) category->addItem(item.name,item.id);
    price->setRange(0.01,999999.99); price->setDecimals(2); price->setSuffix(" 元"); stock->setRange(0,999999);
    picture->setPlaceholderText("例如 /images/example.png"); description->setMaximumHeight(100);
    auto *form=new QFormLayout; form->addRow("分类：",category); form->addRow("菜品名称：",name); form->addRow("价格：",price); form->addRow("库存：",stock); form->addRow("图片地址：",picture); form->addRow("描述：",description);
    auto *buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,this); buttons->button(QDialogButtonBox::Save)->setText("保存"); buttons->button(QDialogButtonBox::Cancel)->setText("取消");
    auto *layout=new QVBoxLayout(this); layout->addLayout(form); layout->addWidget(buttons);
    if(dish){dishId=dish->id; category->setCurrentIndex(category->findData(dish->catId)); name->setText(dish->name); price->setValue(dish->price); stock->setValue(dish->stock); picture->setText(dish->picture); description->setPlainText(dish->description);}
    connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject); connect(buttons,&QDialogButtonBox::accepted,this,[this](){if(category->currentIndex()<0||name->text().trimmed().isEmpty()){QMessageBox::warning(this,"信息不完整","请选择分类并填写菜品名称");return;}accept();});
}
Dish DishEditDialog::value() const {return {dishId,category->currentData().toInt(),name->text().trimmed(),price->value(),stock->value(),picture->text().trimmed(),description->toPlainText().trimmed(),0,0};}

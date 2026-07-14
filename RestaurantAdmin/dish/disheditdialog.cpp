#include "disheditdialog.h"
#include <QColor>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
DishEditDialog::DishEditDialog(const QList<DishCategory> &categories,const Dish *dish,QWidget *parent)
    :QDialog(parent),category(new QComboBox(this)),name(new QLineEdit(this)),price(new QDoubleSpinBox(this)),stock(new QSpinBox(this)),picture(new QLineEdit(this)),description(new QTextEdit(this)) {
    const bool editing=dish!=nullptr;
    setWindowTitle(editing?"修改菜品":"新增菜品");
    setObjectName("dishEditDialog");
    setAttribute(Qt::WA_StyledBackground,true);
    setMinimumSize(590,650);
    setStyleSheet(
        "QDialog#dishEditDialog{background:#f7f3eb;}"
        "QLabel{background:transparent;color:#4b4642;font-size:15px;font-weight:600;}"
        "QLineEdit,QComboBox,QDoubleSpinBox,QSpinBox,QTextEdit{background:#fffdfa;color:#3f3b38;border:1px solid #ded4c8;border-radius:12px;font-size:15px;padding:8px 12px;}"
        "QLineEdit:focus,QComboBox:focus,QDoubleSpinBox:focus,QSpinBox:focus,QTextEdit:focus{border:2px solid #e99a4b;padding:7px 11px;}"
        "QComboBox{padding-right:48px;}"
        "QComboBox::drop-down{subcontrol-origin:border;subcontrol-position:top right;width:42px;background:#f8ead9;border-left:1px solid #ded4c8;border-top-right-radius:11px;border-bottom-right-radius:11px;}"
        "QComboBox::down-arrow{image:url(:/controls/down-arrow.xpm);width:18px;height:18px;}"
        "QDoubleSpinBox,QSpinBox{padding-right:48px;}"
        "QDoubleSpinBox::up-button,QSpinBox::up-button{subcontrol-origin:border;subcontrol-position:top right;width:42px;background:#f8ead9;border-left:1px solid #ded4c8;border-bottom:1px solid #e8dbcd;border-top-right-radius:11px;}"
        "QDoubleSpinBox::down-button,QSpinBox::down-button{subcontrol-origin:border;subcontrol-position:bottom right;width:42px;background:#f8ead9;border-left:1px solid #ded4c8;border-bottom-right-radius:11px;}"
        "QDoubleSpinBox::up-arrow,QSpinBox::up-arrow{image:url(:/controls/up-arrow.xpm);width:18px;height:18px;}"
        "QDoubleSpinBox::down-arrow,QSpinBox::down-arrow{image:url(:/controls/down-arrow.xpm);width:18px;height:18px;}"
        "QComboBox::drop-down:hover,QDoubleSpinBox::up-button:hover,QDoubleSpinBox::down-button:hover,QSpinBox::up-button:hover,QSpinBox::down-button:hover{background:#f3dcc1;}"
        "QComboBox QAbstractItemView{background:white;color:#3f3b38;border:1px solid #ead8c5;selection-background-color:#f7d8b4;selection-color:#3f3b38;}"
    );

    for(const auto &item:categories) category->addItem(item.name,item.id);
    price->setRange(0.01,999999.99); price->setDecimals(2); price->setSuffix(" 元"); stock->setRange(0,999999);
    name->setPlaceholderText("请输入菜品名称");
    picture->setPlaceholderText("例如 example.png 或 /images/example.png");
    description->setPlaceholderText("请输入菜品介绍（可选）");
    description->setMaximumHeight(105);
    const QList<QWidget*> inputs{category,name,price,stock,picture};
    for(QWidget *input:inputs) input->setMinimumHeight(46);

    auto *heading=new QLabel(editing?"修改菜品":"新增菜品",this);
    heading->setAlignment(Qt::AlignCenter);
    heading->setStyleSheet("font-size:27px;font-weight:800;color:#333333;background:transparent;");
    auto *subtitle=new QLabel(editing?"更新菜品价格、库存及展示信息":"填写菜品基本信息并保存到数据库",this);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("font-size:13px;font-weight:400;color:#8c8178;background:transparent;");

    auto *card=new QWidget(this);
    card->setObjectName("dishFormCard");
    card->setStyleSheet("QWidget#dishFormCard{background:#ffffff;border-radius:20px;}");
    auto *shadow=new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(28); shadow->setOffset(0,8); shadow->setColor(QColor(85,58,35,35));
    card->setGraphicsEffect(shadow);

    auto *form=new QFormLayout;
    form->setLabelAlignment(Qt::AlignRight|Qt::AlignVCenter);
    form->setHorizontalSpacing(18); form->setVerticalSpacing(14);
    form->addRow("分类",category); form->addRow("菜品名称",name); form->addRow("价格",price); form->addRow("库存",stock); form->addRow("图片地址",picture); form->addRow("菜品描述",description);
    auto *buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,this); buttons->button(QDialogButtonBox::Save)->setText("保存"); buttons->button(QDialogButtonBox::Cancel)->setText("取消");
    auto *saveButton=buttons->button(QDialogButtonBox::Save);
    auto *cancelButton=buttons->button(QDialogButtonBox::Cancel);
    saveButton->setDefault(true); saveButton->setMinimumSize(112,42); cancelButton->setMinimumSize(96,42);
    saveButton->setCursor(Qt::PointingHandCursor); cancelButton->setCursor(Qt::PointingHandCursor);
    saveButton->setFocusPolicy(Qt::NoFocus); cancelButton->setFocusPolicy(Qt::NoFocus);
    saveButton->setStyleSheet("QPushButton{outline:none;background:#ed9338;color:white;border:0;border-radius:20px;font-size:15px;font-weight:700;padding:0 22px;} QPushButton:hover{background:#e4862c;} QPushButton:pressed{background:#d67722;}");
    cancelButton->setStyleSheet("QPushButton{outline:none;background:#eeeeee;color:#666666;border:0;border-radius:20px;font-size:15px;padding:0 20px;} QPushButton:hover{background:#e2e2e2;} QPushButton:pressed{background:#d6d6d6;}");

    auto *cardLayout=new QVBoxLayout(card);
    cardLayout->setContentsMargins(30,28,30,26);
    cardLayout->setSpacing(20);
    cardLayout->addLayout(form);
    cardLayout->addWidget(buttons);

    auto *layout=new QVBoxLayout(this);
    layout->setContentsMargins(38,28,38,34); layout->setSpacing(12);
    layout->addWidget(heading); layout->addWidget(subtitle); layout->addSpacing(6); layout->addWidget(card,1);
    if(dish){dishId=dish->id; category->setCurrentIndex(category->findData(dish->catId)); name->setText(dish->name); price->setValue(dish->price); stock->setValue(dish->stock); picture->setText(dish->picture); description->setPlainText(dish->description);}
    connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject); connect(buttons,&QDialogButtonBox::accepted,this,[this](){if(category->currentIndex()<0||name->text().trimmed().isEmpty()){QMessageBox::warning(this,"信息不完整","请选择分类并填写菜品名称");return;}accept();});
}
Dish DishEditDialog::value() const {return {dishId,category->currentData().toInt(),name->text().trimmed(),price->value(),stock->value(),picture->text().trimmed(),description->toPlainText().trimmed(),0,0};}

#include "adminmainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

// 程序入口：初始化 Qt、加载系统语言翻译、显示管理端主窗口并进入事件循环。
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "RestaurantAdmin_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    AdminMainWindow w;
    w.show();
    return QApplication::exec();
}

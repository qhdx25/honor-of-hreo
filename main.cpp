//启动，创建窗口并跑起来
#include "mainwindow.h"
//引入 Qt 的应用程序类
#include <QApplication>
//系统主函数，所有 C++/Qt 程序的唯一入口，程序从这里开始执行。
int main(int argc, char *argv[])
{
    //创建 Qt 应用程序对象 a
    QApplication a(argc, argv);
    //创建主窗口对象 w，跳转到mainwindow.cpp
    MainWindow w;
    //窗口显示
    w.show();
    return a.exec();
}

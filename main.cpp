//负责启动，创建窗口并跑起来
//引入你自己写的主窗口类头文件，告诉编译器 “我要用到 MainWindow 这个类”。
#include "mainwindow.h"
//引入 Qt 的应用程序类，这是 Qt 程序的 “大管家”
#include <QApplication>
//系统主函数，所有 C++/Qt 程序的唯一入口，程序从这里开始执行。
int main(int argc, char *argv[])
{
    //1创建 Qt 应用程序对象 a
    QApplication a(argc, argv);
    //2创建主窗口对象 w，跳转到mainwindow.cpp
    MainWindow w;
    //6将窗口显示到屏幕上（之前只是创建好了，还没显示）
    w.show();
    //启动 Qt 的事件循环，程序会一直运行，直到你关闭窗口才退出
    return a.exec();
}

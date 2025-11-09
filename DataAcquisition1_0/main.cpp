#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

#include <QSqlDatabase>
#include <QDebug>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用程序信息
    QApplication::setApplicationName("数据采集与分析系统");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("DataAcquisition");

    // 设置样式表（可选）
    QFile styleFile(":/styles/dark.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream styleStream(&styleFile);
        a.setStyleSheet(styleStream.readAll());
        styleFile.close();
    }

    // 创建并显示主窗口
    MainWindow w;
    w.show();

    qDebug() << "Available drivers:";
    const QStringList drivers = QSqlDatabase::drivers();
    for (const QString &driver : drivers) {
        qDebug() << driver;
    }

    return a.exec();
}

#include "mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <log.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    // Get the primary screen geometry
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // Set the main window size to match the screen resolution
    w.setGeometry(0, 0, screenWidth, screenHeight);

    if(QFile::exists("dark_mode_style_sheet.qss")){
        LOG_TO_FILE("width:%d Height:%d",screenWidth ,screenHeight);
        QFile file("dark_mode_style_sheet.qss");
        if (file.open(QFile::ReadOnly)) {
            QString styleSheet = QLatin1String(file.readAll());
            w.setStyleSheet(styleSheet);
        }
    }else{

        LOG_TO_FILE("Else width:%d Height:%d",screenWidth ,screenHeight);
        QFile file("../../dark_mode_style_sheet.qss");
        if (file.open(QFile::ReadOnly)) {
            QString styleSheet = QLatin1String(file.readAll());
            w.setStyleSheet(styleSheet);
        }
    }
    w.showMaximized();
    w.show();
    return a.exec();
}

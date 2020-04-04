#include "mainwindow.h"
#include <QApplication>
#include <QSharedMemory>

int main(int argc, char *argv[])
{
    QSharedMemory sharedMemory;
    sharedMemory.setKey(QString("mchecker"));
    sharedMemory.attach();

    if (!sharedMemory.create(1)) {
        return 0; // Exit already a process running
    }
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon("images/tray1.png"));
    a.setQuitOnLastWindowClosed(false);
    MainWindow w;
    w.setWindowIcon(QIcon("images/tray1.png"));

    if(w.sm->GetDebugLevel()>1) w.show();
    else w.hide();
    
    return a.exec();
}

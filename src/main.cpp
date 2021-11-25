#include <unistd.h>
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{

    if(getuid() != 0){
        cout << "Permission denied" << endl;
        exit(1);
    }
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

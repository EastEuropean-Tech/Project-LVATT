#include "ProjectLVATT.h"
#include <QtWidgets/QApplication>

#pragma comment(lib, "Qt6Multimedia.lib")
#pragma comment(lib, "Qt6MultimediaWidgets.lib")

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LVATTMainWindow w;
    w.show();
    return a.exec();
}

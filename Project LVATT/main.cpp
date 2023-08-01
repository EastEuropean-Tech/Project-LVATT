#include "ProjectLVATT.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LVATTMainWindow w;
    w.show();
    return a.exec();
}

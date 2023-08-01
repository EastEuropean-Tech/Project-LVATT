#include "ProjectLVATT.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ProjectLVATT w;
    w.show();
    return a.exec();
}

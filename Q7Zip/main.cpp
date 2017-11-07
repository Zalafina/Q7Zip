#include "q7zip.h"
#include <QApplication>

#ifdef VLD_ENABLED
#include "vld.h"
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    Q7Zip w;

    // Remove "?" Button from QDialog
    Qt::WindowFlags flags = Qt::Dialog;
    flags |= Qt::WindowCloseButtonHint;
    flags |= Qt::WindowMinimizeButtonHint;
    w.setWindowFlags(flags);

    w.show();

    return a.exec();
}

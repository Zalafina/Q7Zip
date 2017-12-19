#include "q7zip_window.h"
#include <QApplication>

#ifdef VLD_ENABLED
#include "vld.h"
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    Q7Zip_Window w;

    int init_result = w.init();

    if (0 == init_result){
        // Remove "?" Button from QDialog
        Qt::WindowFlags flags = Qt::Dialog;
        flags |= Qt::WindowCloseButtonHint;
        flags |= Qt::WindowMinimizeButtonHint;
        w.setWindowFlags(flags);

        w.show();

        return a.exec();
    }
    else{
        return -1;
    }
}

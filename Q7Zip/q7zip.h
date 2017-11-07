#ifndef Q7ZIP_H
#define Q7ZIP_H

#include <QStyleFactory>
#include <QDialog>

namespace Ui {
class Q7Zip;
}

class Q7Zip : public QDialog
{
    Q_OBJECT

public:
    explicit Q7Zip(QWidget *parent = 0);
    ~Q7Zip();

private:
    Ui::Q7Zip *ui;
};

#endif // Q7ZIP_H

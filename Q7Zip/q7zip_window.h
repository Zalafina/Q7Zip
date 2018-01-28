#ifndef Q7ZIP_WINDOW_H
#define Q7ZIP_WINDOW_H

#include <QStyleFactory>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QThread>
#include "q7zip.h"
#include "progress_window.h"

namespace Ui {
class Q7Zip_Window;
}

class Q7Zip_Window : public QDialog
{
    Q_OBJECT

public:
    explicit Q7Zip_Window(QWidget *parent = 0);
    ~Q7Zip_Window();

public slots:
    void WorkerThread_Started(int result);
    void Operation_Result(Q7Zip::Operation operation, const QString archive_filename, int result);

private slots:
    void on_Make7zButton_clicked();

    void on_Extract7zButton_clicked();

private:
    Ui::Q7Zip_Window *ui;
    Q7Zip *m_7Zip;
    QThread *workerThread;
    Progress_Window *progress_win;
};

#endif // Q7ZIP_WINDOW_H

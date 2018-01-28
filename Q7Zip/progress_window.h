#ifndef PROGRESS_WINDOW_H
#define PROGRESS_WINDOW_H

#include <QDialog>

namespace Ui {
class Progress_Window;
}

class Progress_Window : public QDialog
{
    Q_OBJECT

public:
    explicit Progress_Window(QWidget *parent = 0);
    ~Progress_Window();

public slots:
    void Compressing_FileName(const QString filename);
    void Compress_Range(const quint64 filesize);

    void Extracting_FileName(const QString filename);
    void Extract_Range(const quint64 filesize);

    void processbar_SetValue(quint64 progressValue);

public:
    Ui::Progress_Window *progress_ui;
};

#endif // PROGRESS_WINDOW_H

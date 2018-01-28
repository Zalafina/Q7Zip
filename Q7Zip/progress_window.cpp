#include "progress_window.h"
#include "ui_progress_window.h"

Progress_Window::Progress_Window(QWidget *parent) :
    QDialog(parent),
    progress_ui(new Ui::Progress_Window)
{
    progress_ui->setupUi(this);
}

Progress_Window::~Progress_Window()
{
    delete progress_ui;
}

void Progress_Window::Compressing_FileName(const QString filename)
{
    progress_ui->filenameLabel->setText(filename);
}

void Progress_Window::Compress_Range(const quint64 filesize)
{
    if (filesize > 0){
        progress_ui->progressBar->setRange(0, filesize);
        progress_ui->progressBar->setValue(0);
    }
}

void Progress_Window::Extracting_FileName(const QString filename)
{
    progress_ui->filenameLabel->setText(filename);
}

void Progress_Window::Extract_Range(const quint64 filesize)
{
    if (filesize > 0){
        progress_ui->progressBar->setRange(0, filesize);
        progress_ui->progressBar->setValue(0);
    }
}

void Progress_Window::processbar_SetValue(quint64 progressValue)
{
    progress_ui->progressBar->setValue(progressValue);
}

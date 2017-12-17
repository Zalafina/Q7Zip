#include "q7zip_window.h"
#include "ui_q7zip_window.h"

Q7Zip_Window::Q7Zip_Window(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Q7Zip_Window),
    m_7Zip()
{
    ui->setupUi(this);
}

Q7Zip_Window::~Q7Zip_Window()
{
    delete ui;
}

void Q7Zip_Window::on_Make7zButton_clicked()
{
    QString archinve_filename = "test.7z";
    QString temp_filename = "QKeyMapper.exe";
    QStringList compress_filelist;
    compress_filelist.append(temp_filename);
    m_7Zip.compress(archinve_filename, compress_filelist);
}

void Q7Zip_Window::on_Extract7zButton_clicked()
{

}

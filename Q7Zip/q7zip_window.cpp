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
    QString filename = QFileDialog::getOpenFileName(this,
                                                    "Open File to be Compressed",
                                                    NULL,
                                                    "All Files (*.*)");

    if (filename.length() != 0){
        QFileInfo fileinfo(filename);
        QString archive_filename = fileinfo.absolutePath() + "/" + fileinfo.baseName() + ".7z";

        QStringList compress_filelist;
        compress_filelist.append(filename);

        bool compress_result;
        compress_result = m_7Zip.compress(archive_filename, compress_filelist);

        if (0 == compress_result){
            qDebug() << archive_filename << "compress complete.";
            QString message = "<html><head/><body><p align=\"center\">" + fileinfo.baseName() + ".7z" "</p><p align=\"center\"> Compress Complete.</p></body></html>";
            QMessageBox::information(this, "Q7Zip", message);
        }
    }
    else{
    }
}

void Q7Zip_Window::on_Extract7zButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    "Open File to be Extracted",
                                                    NULL,
                                                    "7z Files (*.7z)");

    if (true == QFileInfo::exists(filename)){
        QFileInfo fileinfo(filename);
        QString archive_filename = filename;
        QString output_path;
        if (true == QFileInfo::exists(fileinfo.absolutePath())){
            output_path = fileinfo.absolutePath();
        }
        bool extrace_result;
        extrace_result = m_7Zip.extract(archive_filename, output_path);

        if (0 == extrace_result){
            qDebug() << archive_filename << "extract complete.";
            QString message = "<html><head/><body><p align=\"center\">" + fileinfo.fileName() + "</p><p align=\"center\"> Extract Complete.</p></body></html>";
            QMessageBox::information(this, "Q7Zip", message);
        }
    }
}

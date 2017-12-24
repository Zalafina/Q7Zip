#include "q7zip_window.h"
#include "mfiledialog.h"
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

int Q7Zip_Window::init(void)
{
    int init_result;
    init_result = m_7Zip.init();

    if (init_result != 0){
        QString message = "<html><head/><body><p align=\"center\">" kDllName "</p><p align=\"center\"> Load Failure!!!</p></body></html>";
        //QMessageBox::critical(this, "Q7Zip", message);
        /* Set parent pointer to NULL for display messagebox at the center of the screen */
        QMessageBox::critical(NULL, "Q7Zip", message);
    }
    else{
        ui->LZMA_SDK_Label->setText(m_7Zip.lzma_sdk_version());
    }

    return init_result;
}

void Q7Zip_Window::on_Make7zButton_clicked()
{
#if 0
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
#endif

    MFileDialog fileDialog;
    if (fileDialog.exec() == QDialog::Accepted)
    {
        QStringList compress_filelist;
        QStringList selected_files = fileDialog.selectedFiles();
        selected_files.sort(Qt::CaseInsensitive);
        for(const QString &filename : selected_files){
            compress_filelist.append(filename);

            QFileInfo fileinfo(filename);
            if (true == fileinfo.isDir()){
                QFileInfoList list = MFileDialog::GetFileList(filename);

                for (const QFileInfo &finfo : list){
                    compress_filelist.append(finfo.absoluteFilePath());
                }
            }
        }

#ifdef DEBUG_LOGOUT_ON
        qDebug().noquote() << "";
        qDebug().noquote() << "Compress Filelist Start >>>";
        for(const QString &filename : compress_filelist){
            qDebug().noquote() << filename;
        }
        qDebug().noquote() << "Compress Filelist End   <<<";
        qDebug().noquote() << "";
#endif

        if (compress_filelist.size() > 0){
            QString archive_filename;
            QString woring_path;
            QString first_filename = selected_files.at(0);
            QFileInfo fileinfo(first_filename);
            QFileInfo path_fileinfo(fileinfo.absolutePath());

            if (1 == selected_files.size()){
                archive_filename = fileinfo.absolutePath() + "/" + fileinfo.completeBaseName() + ".7z";
            }
            else{
                archive_filename = fileinfo.absolutePath() + "/" + path_fileinfo.completeBaseName() + ".7z";
            }
            woring_path = fileinfo.absolutePath() + "/";

            bool compress_result;
            compress_result = m_7Zip.compress(archive_filename, compress_filelist, woring_path);

            if (0 == compress_result){
                QFileInfo archive_fileinfo(archive_filename);
                qDebug() << archive_filename << "compress complete.";
                QString message = "<html><head/><body><p align=\"center\">" + archive_fileinfo.fileName() + "</p><p align=\"center\"> Compress Complete.</p></body></html>";
                QMessageBox::information(this, "Q7Zip", message);
            }
        }
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
            output_path = fileinfo.absolutePath() + "/" + fileinfo.baseName();
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

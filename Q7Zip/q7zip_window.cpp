#include "q7zip_window.h"
#include "mfiledialog.h"
#include "ui_q7zip_window.h"

Q7Zip_Window::Q7Zip_Window(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Q7Zip_Window),
    m_7Zip(NULL),
    workerThread(NULL)
{
    ui->setupUi(this);

    m_7Zip = new Q7Zip();
    // Move Checksumer to a sub thread
    workerThread = new QThread();
    m_7Zip->moveToThread(workerThread);
    workerThread->setObjectName("Q7Zip");
    static_cast<void>(QObject::connect(workerThread, SIGNAL(started()), m_7Zip, SLOT(threadStarted())));
    static_cast<void>(QObject::connect(m_7Zip, SIGNAL(threadStarted_signal(int)), this, SLOT(WorkerThread_Started(int)), Qt::QueuedConnection));
    static_cast<void>(QObject::connect(m_7Zip, SIGNAL(operation_result_signal(Q7Zip::Operation, const QString, int)), this, SLOT(Operation_Result(Q7Zip::Operation, const QString, int)), Qt::QueuedConnection));
    workerThread->start();
}

Q7Zip_Window::~Q7Zip_Window()
{
    delete ui;
}

void Q7Zip_Window::WorkerThread_Started(int result)
{
    if (result != 0){
        QString message = "<html><head/><body><p align=\"center\">" kDllName "</p><p align=\"center\"> Load Failure!!!</p></body></html>";
        //QMessageBox::critical(this, "Q7Zip", message);
        /* Set parent pointer to NULL for display messagebox at the center of the screen */
        QMessageBox::critical(NULL, "Q7Zip", message);

        qApp->exit(-1);
    }
    else{
        ui->LZMA_SDK_Label->setText(m_7Zip->lzma_sdk_version());

        show();
    }
}

void Q7Zip_Window::Operation_Result(Q7Zip::Operation operation, const QString archive_filename, int result)
{
#ifdef DEBUG_LOGOUT_ON
    qDebug() << "Operation_Result:" << operation << "," << result;
#endif

    QFileInfo archive_fileinfo(archive_filename);

    if (Q7Zip::Q7ZIP_COMPRESS == operation){
        if(0 == result){
            qDebug() << archive_filename << "compress complete.";
            QString message = "<html><head/><body><p align=\"center\">" + archive_fileinfo.fileName() + "</p><p align=\"center\"> Compress Complete.</p></body></html>";
            QMessageBox::information(this, "Q7Zip", message);
        }
        else{
            qDebug() << archive_filename << "compress failure.";
            QString message = "<html><head/><body><p align=\"center\">" + archive_fileinfo.fileName() + "</p><p align=\"center\"> Compress Failure.</p></body></html>";
            QMessageBox::warning(this, "Q7Zip", message);
        }
    }
    else if (Q7Zip::Q7ZIP_EXTRACT == operation){
        if(0 == result){
            qDebug() << archive_filename << "extract complete.";
            QString message = "<html><head/><body><p align=\"center\">" + archive_fileinfo.fileName() + "</p><p align=\"center\"> Extract Complete.</p></body></html>";
            QMessageBox::information(this, "Q7Zip", message);
        }
        else{
            qDebug() << archive_filename << "extract failure.";
            QString message = "<html><head/><body><p align=\"center\">" + archive_fileinfo.fileName() + "</p><p align=\"center\"> Extract Failure.</p></body></html>";
            QMessageBox::warning(this, "Q7Zip", message);
        }
    }
}

void Q7Zip_Window::on_Make7zButton_clicked()
{
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

            emit m_7Zip->operation_signal_compress(archive_filename, compress_filelist, woring_path);
        }
    }
}

void Q7Zip_Window::on_Extract7zButton_clicked()
{
    QString archive_filename = QFileDialog::getOpenFileName(this,
                                                    "Open File to be Extracted",
                                                    NULL,
                                                    "7z Files (*.7z)");

    if (true == QFileInfo::exists(archive_filename)){
        QFileInfo fileinfo(archive_filename);
        QString output_path;
        if (true == QFileInfo::exists(fileinfo.absolutePath())){
            output_path = fileinfo.absolutePath() + "/" + fileinfo.baseName();
        }

        emit m_7Zip->operation_signal_extract(archive_filename, output_path);
    }
}

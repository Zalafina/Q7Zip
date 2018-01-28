#ifndef Q7ZIP_H
#define Q7ZIP_H

#include <QObject>
#include <QLibrary>
#include <QFileInfo>
#include <QDebug>
#include <QThread>

#define kDllName "7z.dll"

class Q7Zip : public QObject
{
    Q_OBJECT
public:
    explicit Q7Zip(QObject *parent = 0);

    enum Operation
    {
        Q7ZIP_COMPRESS = 0U,
        Q7ZIP_EXTRACT,
        Q7ZIP_SHOWLIST
    };
    Q_ENUM(Operation)

    static Q7Zip * getInstance(void);

    int init(void);
    QString lzma_sdk_version(void);

    int compress(const QString &archive_name, const QStringList &compress_filelist, const QString &working_path);
    int extract(const QString &archive_name, const QString &output_path);
    int showfilelist(const QString &archive_name);

signals:
    void threadStarted_signal(int result);
    void operation_signal_compress(const QString archive_name, const QStringList compress_filelist, const QString working_path);
    void operation_signal_extract(const QString archive_name, const QString output_path);
    void operation_result_signal(Q7Zip::Operation operation, const QString archive_filename, int result);

    void compress_filesize_signal(const quint64 filesize);
    void compress_completeValue_signal(const quint64 completeValue);
    void compressing_filename_signal(const QString filename);

    void extract_filesize_signal(const quint64 filesize);
    void extract_completeValue_signal(const quint64 completeValue);
    void extracting_filename_signal(const QString filename);

public slots:
    void threadStarted(void);
    void operation_slot_compress(const QString archive_name, const QStringList compress_filelist, const QString working_path);
    void operation_slot_extract(const QString archive_name, const QString output_path);

private:
    static Q7Zip * m_q7zip;
    QLibrary m_7zLib;
};

#endif // Q7ZIP_H

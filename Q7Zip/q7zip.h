#ifndef Q7ZIP_H
#define Q7ZIP_H

#include <QObject>
#include <QLibrary>
#include <QFileInfo>
#include <QDebug>

#define kDllName "7z.dll"

class Q7Zip : public QObject
{
    Q_OBJECT
public:
    explicit Q7Zip(QObject *parent = 0);

    int init(void);

    int compress(const QString &archive_name, const QStringList &compress_filelist, const QString &working_path);
    int extract(const QString &archive_name, const QString &output_path);
    int showfilelist(const QString &archive_name);

signals:

public slots:

private:
    QLibrary m_7zLib;
};

#endif // Q7ZIP_H

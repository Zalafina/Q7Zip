#ifndef Q7ZIP_H
#define Q7ZIP_H

#include <QObject>
#include <QLibrary>
#include <QDebug>

class Q7Zip : public QObject
{
    Q_OBJECT
public:
    explicit Q7Zip(QObject *parent = 0);

    int compress(QString &archive_name, QStringList &compress_filelist);
    int extract(QString &archive_name);
    int showfilelist(QString &archive_name);

signals:

public slots:

private:
    QLibrary m_7zLib;
};

#endif // Q7ZIP_H

#include <QListView>
#include <QTreeView>
#include <QDialogButtonBox>
#include <QDebug>
#include "mfiledialog.h"

MFileDialog::MFileDialog(QWidget *parent) :
    QFileDialog(parent)
{
    Qt::WindowFlags flags = Qt::Dialog;
    flags |= Qt::WindowCloseButtonHint;
    this->setWindowFlags(flags);

    this->setOption(QFileDialog::DontUseNativeDialog,true);

    // Support multiple choice
    QListView *pListView = this->findChild<QListView*>("listView");
    if (pListView){
        pListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }
    QTreeView *pTreeView = this->findChild<QTreeView*>();
    if (pTreeView){
        pTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }
    QDialogButtonBox *pButton = this->findChild<QDialogButtonBox *>("buttonBox");

    // Change accepted signal to trigger custom slot.
    disconnect(pButton, SIGNAL(accepted()), this, SLOT(accept()));
    connect(pButton, SIGNAL(accepted()), this, SLOT(onChiose()));
}

MFileDialog::~MFileDialog()
{

}

QFileInfoList MFileDialog::GetFileList(const QString &path)
{
    QFileInfoList final_list;
    QDir dir(path);
    dir.setSorting(QDir::IgnoreCase | QDir::DirsFirst | QDir::Name);
    QFileInfoList fileinfolist = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot| QDir::CaseSensitive | QDir::Files | QDir::Hidden | QDir::System);

    for (const QFileInfo &fileinfo : fileinfolist){
        final_list.append(fileinfo);

        if (true == fileinfo.isDir()){
            QFileInfoList child_file_list = GetFileList(fileinfo.absoluteFilePath());
            final_list.append(child_file_list);
        }
    }

    return final_list;
}

void MFileDialog::onChiose()
{
    QDialog::accept();
}

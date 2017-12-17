#include "q7zip.h"


#include "FileStreams.h"
#include "IArchive.h"

#define kDllName "7z.dll"

DEFINE_GUID(CLSID_CFormat7z,
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatXz,
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x0C, 0x00, 0x00);

#define CLSID_Format CLSID_CFormat7z
// #define CLSID_Format CLSID_CFormatXz

using namespace NWindows;
using namespace NFile;
//using namespace NDir;

Q7Zip::Q7Zip(QObject *parent) :
    QObject(parent),
    m_7zLib(kDllName)
{
    bool loadResult;
    loadResult = m_7zLib.load();
    Q_UNUSED(loadResult);

#ifdef DEBUG_LOGOUT_ON
    if (true == loadResult){
        qDebug() << "Q7Zip" << kDllName << "load success.";
    }
    else{
        qDebug() << "Q7Zip" << kDllName << "load failure!!!";
    }
#endif
}

int Q7Zip::compress(QString &archive_name, QStringList &compress_filelist)
{
    Q_UNUSED(compress_filelist);
    Func_CreateObject createObjectFunc = (Func_CreateObject)m_7zLib.resolve("CreateObject");
    if (!createObjectFunc)
    {
        qDebug("Can not get CreateObject");
        return 1;
    }

#if 0
    CObjectVector<CDirItem> dirItems;
    {
      int i;
      for (i = 3; i < numArgs; i++)
      {
        CDirItem di;
        FString name = CmdStringToFString(args[i]);

        NFind::CFileInfo fi;
        if (!fi.Find(name))
        {
          PrintError("Can't find file", name);
          return 1;
        }

        di.Attrib = fi.Attrib;
        di.Size = fi.Size;
        di.CTime = fi.CTime;
        di.ATime = fi.ATime;
        di.MTime = fi.MTime;
        di.Name = fs2us(name);
        di.FullPath = name;
        dirItems.Add(di);
      }
    }
#endif

    COutFileStream *outFileStreamSpec = new COutFileStream;
    CMyComPtr<IOutStream> outFileStream = outFileStreamSpec;
    if (!outFileStreamSpec->Create(archive_name.toStdWString().c_str(), true))
    {
        qDebug("can't create archive file");
        return 1;
    }

    CMyComPtr<IOutArchive> outArchive;
    if (createObjectFunc(&CLSID_Format, &IID_IOutArchive, (void **)&outArchive) != S_OK)
    {
        qDebug("Can not get class object");
        return 1;
    }

//    CArchiveUpdateCallback *updateCallbackSpec = new CArchiveUpdateCallback;
//    CMyComPtr<IArchiveUpdateCallback2> updateCallback(updateCallbackSpec);
//    updateCallbackSpec->Init(&dirItems);

#if 0
    NDLL::CLibrary lib;
    if (!lib.Load(NDLL::GetModuleDirPrefix() + FTEXT(kDllName)))
    {
      PrintError("Can not load 7-zip library");
      return 1;
    }

    Func_CreateObject createObjectFunc = (Func_CreateObject)lib.GetProc("CreateObject");
    if (!createObjectFunc)
    {
      PrintError("Can not get CreateObject");
      return 1;
    }

    char c;
    {
      AString command (args[1]);
      if (command.Len() != 1)
      {
        PrintError(kIncorrectCommand);
        return 1;
      }
      c = (char)MyCharLower_Ascii(command[0]);
    }

    FString archiveName = CmdStringToFString(args[2]);

    if (c == 'a')
    {
      // create archive command
      if (numArgs < 4)
      {
        PrintError(kIncorrectCommand);
        return 1;
      }
      CObjectVector<CDirItem> dirItems;
      {
        int i;
        for (i = 3; i < numArgs; i++)
        {
          CDirItem di;
          FString name = CmdStringToFString(args[i]);

          NFind::CFileInfo fi;
          if (!fi.Find(name))
          {
            PrintError("Can't find file", name);
            return 1;
          }

          di.Attrib = fi.Attrib;
          di.Size = fi.Size;
          di.CTime = fi.CTime;
          di.ATime = fi.ATime;
          di.MTime = fi.MTime;
          di.Name = fs2us(name);
          di.FullPath = name;
          dirItems.Add(di);
        }
      }

      COutFileStream *outFileStreamSpec = new COutFileStream;
      CMyComPtr<IOutStream> outFileStream = outFileStreamSpec;
      if (!outFileStreamSpec->Create(archiveName, true))
      {
        PrintError("can't create archive file");
        return 1;
      }

      CMyComPtr<IOutArchive> outArchive;
      if (createObjectFunc(&CLSID_Format, &IID_IOutArchive, (void **)&outArchive) != S_OK)
      {
        PrintError("Can not get class object");
        return 1;
      }

      CArchiveUpdateCallback *updateCallbackSpec = new CArchiveUpdateCallback;
      CMyComPtr<IArchiveUpdateCallback2> updateCallback(updateCallbackSpec);
      updateCallbackSpec->Init(&dirItems);
      // updateCallbackSpec->PasswordIsDefined = true;
      // updateCallbackSpec->Password = L"1";

      /*
      {
        const wchar_t *names[] =
        {
          L"s",
          L"x"
        };
        const unsigned kNumProps = ARRAY_SIZE(names);
        NCOM::CPropVariant values[kNumProps] =
        {
          false,    // solid mode OFF
          (UInt32)9 // compression level = 9 - ultra
        };
        CMyComPtr<ISetProperties> setProperties;
        outArchive->QueryInterface(IID_ISetProperties, (void **)&setProperties);
        if (!setProperties)
        {
          PrintError("ISetProperties unsupported");
          return 1;
        }
        RINOK(setProperties->SetProperties(names, values, kNumProps));
      }
      */

      HRESULT result = outArchive->UpdateItems(outFileStream, dirItems.Size(), updateCallback);

      updateCallbackSpec->Finilize();

      if (result != S_OK)
      {
        PrintError("Update Error");
        return 1;
      }

      FOR_VECTOR (i, updateCallbackSpec->FailedFiles)
      {
        PrintNewLine();
        PrintError("Error for file", updateCallbackSpec->FailedFiles[i]);
      }

      if (updateCallbackSpec->FailedFiles.Size() != 0)
        return 1;
    }
#endif

    return 0;
}

int Q7Zip::extract(QString archive_name)
{
    return 0;
}

int Q7Zip::showfilelist(QString archive_name)
{
    return 0;
}

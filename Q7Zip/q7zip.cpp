#include "q7zip.h"

#include "Windows/PropVariant.h"
#include "Windows/FileFind.h"

#include "7Zip/Common/FileStreams.h"
#include "7Zip/Archive/IArchive.h"
#include "7Zip/IPassword.h"

#include "C/7zVersion.h"

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

#if 1
//////////////////////////////////////////////////////////////
// Archive Creating callback class

struct CDirItem
{
    UInt64 Size;
    FILETIME CTime;
    FILETIME ATime;
    FILETIME MTime;
    UString Name;
    FString FullPath;
    UInt32 Attrib;

    bool isDir() const { return (Attrib & FILE_ATTRIBUTE_DIRECTORY) != 0 ; }
};

class CArchiveUpdateCallback:
        public IArchiveUpdateCallback2,
        public ICryptoGetTextPassword2,
        public CMyUnknownImp
{
public:
    MY_UNKNOWN_IMP2(IArchiveUpdateCallback2, ICryptoGetTextPassword2)

    // IProgress
    STDMETHOD(SetTotal)(UInt64 size);
    STDMETHOD(SetCompleted)(const UInt64 *completeValue);

    // IUpdateCallback2
    STDMETHOD(GetUpdateItemInfo)(UInt32 index,
                                 Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive);
    STDMETHOD(GetProperty)(UInt32 index, PROPID propID, PROPVARIANT *value);
    STDMETHOD(GetStream)(UInt32 index, ISequentialInStream **inStream);
    STDMETHOD(SetOperationResult)(Int32 operationResult);
    STDMETHOD(GetVolumeSize)(UInt32 index, UInt64 *size);
    STDMETHOD(GetVolumeStream)(UInt32 index, ISequentialOutStream **volumeStream);

    STDMETHOD(CryptoGetTextPassword2)(Int32 *passwordIsDefined, BSTR *password);

public:
    CRecordVector<UInt64> VolumesSizes;
    UString VolName;
    UString VolExt;

    FString DirPrefix;
    const CObjectVector<CDirItem> *DirItems;

    bool PasswordIsDefined;
    UString Password;
    bool AskPassword;

    bool m_NeedBeClosed;

    FStringVector FailedFiles;
    CRecordVector<HRESULT> FailedCodes;
    UInt64 FileSize;

    CArchiveUpdateCallback(): PasswordIsDefined(false), AskPassword(false), DirItems(0) {};

    ~CArchiveUpdateCallback() { Finilize(); }
    HRESULT Finilize();

    void Init(const CObjectVector<CDirItem> *dirItems)
    {
        DirItems = dirItems;
        m_NeedBeClosed = false;
        FailedFiles.Clear();
        FailedCodes.Clear();
    }
};

STDMETHODIMP CArchiveUpdateCallback::SetTotal(UInt64 size)
{
    FileSize = size;
    return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::SetCompleted(const UInt64 * completeValue)
{
    printf("Compress %.2f%%\n", static_cast<float>(*completeValue) / FileSize * 100.0f );
    return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::GetUpdateItemInfo(UInt32 /* index */,
                                                       Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive)
{
    if (newData)
        *newData = BoolToInt(true);
    if (newProperties)
        *newProperties = BoolToInt(true);
    if (indexInArchive)
        *indexInArchive = (UInt32)(Int32)-1;
    return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value)
{
    NCOM::CPropVariant prop;

    if (propID == kpidIsAnti)
    {
        prop = false;
        prop.Detach(value);
        return S_OK;
    }

    {
        const CDirItem &dirItem = (*DirItems)[index];
        switch (propID)
        {
        case kpidPath:  prop = dirItem.Name; break;
        case kpidIsDir:  prop = dirItem.isDir(); break;
        case kpidSize:  prop = dirItem.Size; break;
        case kpidAttrib:  prop = dirItem.Attrib; break;
        case kpidCTime:  prop = dirItem.CTime; break;
        case kpidATime:  prop = dirItem.ATime; break;
        case kpidMTime:  prop = dirItem.MTime; break;
        }
    }
    prop.Detach(value);
    return S_OK;
}

HRESULT CArchiveUpdateCallback::Finilize()
{
    if (m_NeedBeClosed)
    {
        SetCompleted(&FileSize);
        qDebug() << "";
        m_NeedBeClosed = false;
    }
    return S_OK;
}
#endif

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

    CObjectVector<CDirItem> dirItems;
    {
        for (const QString &name : compress_filelist)
        {
            CDirItem di;
            NFind::CFileInfo fi;
            if (!fi.Find(name.toStdWString().c_str()))
            {
                qDebug() << "Can't find file:" << name;
                return 1;
            }

            di.Attrib = fi.Attrib;
            di.Size = fi.Size;
            di.CTime = fi.CTime;
            di.ATime = fi.ATime;
            di.MTime = fi.MTime;
            di.Name = fs2us(name.toStdWString().c_str());
            di.FullPath = name.toStdWString().c_str();
            dirItems.Add(di);

        }
    }

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

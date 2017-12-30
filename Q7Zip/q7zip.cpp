#include "q7zip.h"

#include "Common/IntToString.h"
#include "Common/StringConvert.h"

#include "Windows/FileDir.h"
#include "Windows/FileFind.h"
#include "Windows/FileName.h"
#include "Windows/PropVariant.h"
#include "Windows/PropVariantConv.h"

#include "7Zip/Common/FileStreams.h"
#include "7Zip/Archive/IArchive.h"
#include "7Zip/IPassword.h"

#include "C/7zVersion.h"

DEFINE_GUID(CLSID_CFormat7z,
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatXz,
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x0C, 0x00, 0x00);

#define CLSID_Format CLSID_CFormat7z
// #define CLSID_Format CLSID_CFormatXz

using namespace NWindows;
using namespace NFile;
using namespace NDir;

#if 1
static void Convert_UString_to_AString(const UString &s, AString &temp)
{
    int codePage = CP_OEMCP;
    /*
  int g_CodePage = -1;
  int codePage = g_CodePage;
  if (codePage == -1)
    codePage = CP_OEMCP;
  if (codePage == CP_UTF8)
    ConvertUnicodeToUTF8(s, temp);
  else
  */
    UnicodeStringToMultiByte2(temp, s, (UINT)codePage);
}

static FString CmdStringToFString(const char *s)
{
    return us2fs(GetUnicodeString(s));
}

static void Print(const char *s)
{
    //fputs(s, stdout);
    qDebug("%s", s);
}

static void Print(const AString &s)
{
    Print(s.Ptr());
}

static void Print(const UString &s)
{
    AString as;
    Convert_UString_to_AString(s, as);
    Print(as);
}

static void Print(const wchar_t *s)
{
    Print(UString(s));
}

static void PrintNewLine()
{
    //Print("\n");
    Print("");
}

static void PrintStringLn(const char *s)
{
    Print(s);
    PrintNewLine();
}

static void PrintError(const char *message)
{
    Print("Error: ");
    PrintNewLine();
    Print(message);
    PrintNewLine();
}

static void PrintError(const char *message, const FString &name)
{
    PrintError(message);
    Print(name);
}


static HRESULT IsArchiveItemProp(IInArchive *archive, UInt32 index, PROPID propID, bool &result)
{
    NCOM::CPropVariant prop;
    RINOK(archive->GetProperty(index, propID, &prop));
    if (prop.vt == VT_BOOL)
        result = VARIANT_BOOLToBool(prop.boolVal);
    else if (prop.vt == VT_EMPTY)
        result = false;
    else
        return E_FAIL;
    return S_OK;
}

static HRESULT IsArchiveItemFolder(IInArchive *archive, UInt32 index, bool &result)
{
    return IsArchiveItemProp(archive, index, kpidIsDir, result);
}


static const wchar_t * const kEmptyFileAlias = L"[Content]";


//////////////////////////////////////////////////////////////
// Archive Open callback class


class CArchiveOpenCallback:
        public IArchiveOpenCallback,
        public ICryptoGetTextPassword,
        public CMyUnknownImp
{
public:
    MY_UNKNOWN_IMP1(ICryptoGetTextPassword)

    STDMETHOD(SetTotal)(const UInt64 *files, const UInt64 *bytes);
    STDMETHOD(SetCompleted)(const UInt64 *files, const UInt64 *bytes);

    STDMETHOD(CryptoGetTextPassword)(BSTR *password);

    bool PasswordIsDefined;
    UString Password;

    CArchiveOpenCallback() : PasswordIsDefined(false) {}
};

STDMETHODIMP CArchiveOpenCallback::SetTotal(const UInt64 * /* files */, const UInt64 * /* bytes */)
{
    return S_OK;
}

STDMETHODIMP CArchiveOpenCallback::SetCompleted(const UInt64 * /* files */, const UInt64 * /* bytes */)
{
    return S_OK;
}

STDMETHODIMP CArchiveOpenCallback::CryptoGetTextPassword(BSTR *password)
{
    if (!PasswordIsDefined)
    {
        // You can ask real password here from user
        // Password = GetPassword(OutStream);
        // PasswordIsDefined = true;
        PrintError("Password is not defined");
        return E_ABORT;
    }
    return StringToBstr(Password, password);
}



static const char * const kIncorrectCommand = "incorrect command";

//////////////////////////////////////////////////////////////
// Archive Extracting callback class

static const char * const kTestingString    =  "Testing     ";
static const char * const kExtractingString =  "Extracting  ";
static const char * const kSkippingString   =  "Skipping    ";

static const char * const kUnsupportedMethod = "Unsupported Method";
static const char * const kCRCFailed = "CRC Failed";
static const char * const kDataError = "Data Error";
static const char * const kUnavailableData = "Unavailable data";
static const char * const kUnexpectedEnd = "Unexpected end of data";
static const char * const kDataAfterEnd = "There are some data after the end of the payload data";
static const char * const kIsNotArc = "Is not archive";
static const char * const kHeadersError = "Headers Error";


class CArchiveExtractCallback:
        public IArchiveExtractCallback,
        public ICryptoGetTextPassword,
        public CMyUnknownImp
{
public:
    MY_UNKNOWN_IMP1(ICryptoGetTextPassword)

    // IProgress
    STDMETHOD(SetTotal)(UInt64 size);
    STDMETHOD(SetCompleted)(const UInt64 *completeValue);

    // IArchiveExtractCallback
    STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode);
    STDMETHOD(PrepareOperation)(Int32 askExtractMode);
    STDMETHOD(SetOperationResult)(Int32 resultEOperationResult);

    // ICryptoGetTextPassword
    STDMETHOD(CryptoGetTextPassword)(BSTR *aPassword);

private:
    CMyComPtr<IInArchive> _archiveHandler;
    FString _directoryPath;  // Output directory
    UString _filePath;       // name inside arcvhive
    FString _diskFilePath;   // full path to file on disk
    bool _extractMode;
    struct CProcessedFileInfo
    {
        FILETIME MTime;
        UInt32 Attrib;
        bool isDir;
        bool AttribDefined;
        bool MTimeDefined;
    } _processedFileInfo;

    COutFileStream *_outFileStreamSpec;
    CMyComPtr<ISequentialOutStream> _outFileStream;

public:
    void Init(IInArchive *archiveHandler, const FString &directoryPath);

    UInt64 NumErrors;
    bool PasswordIsDefined;
    UString Password;
    UInt64 FileSize;

    CArchiveExtractCallback() : PasswordIsDefined(false) {}
};

void CArchiveExtractCallback::Init(IInArchive *archiveHandler, const FString &directoryPath)
{
    NumErrors = 0;
    _archiveHandler = archiveHandler;
    _directoryPath = directoryPath;
    NName::NormalizeDirPathPrefix(_directoryPath);
}

STDMETHODIMP CArchiveExtractCallback::SetTotal(UInt64 size)
{
    FileSize = size;
    return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::SetCompleted(const UInt64 * completeValue)
{
    Q_UNUSED(completeValue);
#ifdef DEBUG_LOGOUT_ON
    qDebug("Extract %.2f%%", static_cast<float>(*completeValue) / FileSize * 100.0f );
#endif
    return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::GetStream(UInt32 index,
                                                ISequentialOutStream **outStream, Int32 askExtractMode)
{
    *outStream = 0;
    _outFileStream.Release();

    {
        // Get Name
        NCOM::CPropVariant prop;
        RINOK(_archiveHandler->GetProperty(index, kpidPath, &prop));

        UString fullPath;
        if (prop.vt == VT_EMPTY)
            fullPath = kEmptyFileAlias;
        else
        {
            if (prop.vt != VT_BSTR)
                return E_FAIL;
            fullPath = prop.bstrVal;
        }
        _filePath = fullPath;
    }

    if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
        return S_OK;

    {
        // Get Attrib
        NCOM::CPropVariant prop;
        RINOK(_archiveHandler->GetProperty(index, kpidAttrib, &prop));
        if (prop.vt == VT_EMPTY)
        {
            _processedFileInfo.Attrib = 0;
            _processedFileInfo.AttribDefined = false;
        }
        else
        {
            if (prop.vt != VT_UI4)
                return E_FAIL;
            _processedFileInfo.Attrib = prop.ulVal;
            _processedFileInfo.AttribDefined = true;
        }
    }

    RINOK(IsArchiveItemFolder(_archiveHandler, index, _processedFileInfo.isDir));

    {
        // Get Modified Time
        NCOM::CPropVariant prop;
        RINOK(_archiveHandler->GetProperty(index, kpidMTime, &prop));
        _processedFileInfo.MTimeDefined = false;
        switch (prop.vt)
        {
        case VT_EMPTY:
            // _processedFileInfo.MTime = _utcMTimeDefault;
            break;
        case VT_FILETIME:
            _processedFileInfo.MTime = prop.filetime;
            _processedFileInfo.MTimeDefined = true;
            break;
        default:
            return E_FAIL;
        }

    }
    {
        // Get Size
        NCOM::CPropVariant prop;
        RINOK(_archiveHandler->GetProperty(index, kpidSize, &prop));
        UInt64 newFileSize;
        /* bool newFileSizeDefined = */ ConvertPropVariantToUInt64(prop, newFileSize);
    }


    {
        // Create folders for file
        int slashPos = _filePath.ReverseFind_PathSepar();
        if (slashPos >= 0)
            CreateComplexDir(_directoryPath + us2fs(_filePath.Left(slashPos)));
    }

    FString fullProcessedPath = _directoryPath + us2fs(_filePath);
    _diskFilePath = fullProcessedPath;

    if (_processedFileInfo.isDir)
    {
        CreateComplexDir(fullProcessedPath);
    }
    else
    {
        NFind::CFileInfo fi;
        if (fi.Find(fullProcessedPath))
        {
            if (!DeleteFileAlways(fullProcessedPath))
            {
                PrintError("Can not delete output file", fullProcessedPath);
                return E_ABORT;
            }
        }

        _outFileStreamSpec = new COutFileStream;
        CMyComPtr<ISequentialOutStream> outStreamLoc(_outFileStreamSpec);
        if (!_outFileStreamSpec->Open(fullProcessedPath, CREATE_ALWAYS))
        {
            PrintError("Can not open output file", fullProcessedPath);
            return E_ABORT;
        }
        _outFileStream = outStreamLoc;
        *outStream = outStreamLoc.Detach();
    }
    return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::PrepareOperation(Int32 askExtractMode)
{
    _extractMode = false;
    switch (askExtractMode)
    {
    case NArchive::NExtract::NAskMode::kExtract:  _extractMode = true; break;
    };
    QString filepath = QString::fromWCharArray(_filePath.Ptr());
    filepath.replace(QString("\\"), QChar('/'));
    switch (askExtractMode)
    {
    case NArchive::NExtract::NAskMode::kExtract:
        //Print(kExtractingString);
        qDebug() << kExtractingString << filepath;
        break;
    case NArchive::NExtract::NAskMode::kTest:
        //Print(kTestingString);
        qDebug() << kTestingString << filepath;
        break;
    case NArchive::NExtract::NAskMode::kSkip:
        //Print(kSkippingString);
        qDebug() << kSkippingString << filepath;
        break;
    };
    //Print(_filePath);
    return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::SetOperationResult(Int32 operationResult)
{
    switch (operationResult)
    {
    case NArchive::NExtract::NOperationResult::kOK:
        break;
    default:
    {
        NumErrors++;
        Print("  :  ");
        const char *s = NULL;
        switch (operationResult)
        {
        case NArchive::NExtract::NOperationResult::kUnsupportedMethod:
            s = kUnsupportedMethod;
            break;
        case NArchive::NExtract::NOperationResult::kCRCError:
            s = kCRCFailed;
            break;
        case NArchive::NExtract::NOperationResult::kDataError:
            s = kDataError;
            break;
        case NArchive::NExtract::NOperationResult::kUnavailable:
            s = kUnavailableData;
            break;
        case NArchive::NExtract::NOperationResult::kUnexpectedEnd:
            s = kUnexpectedEnd;
            break;
        case NArchive::NExtract::NOperationResult::kDataAfterEnd:
            s = kDataAfterEnd;
            break;
        case NArchive::NExtract::NOperationResult::kIsNotArc:
            s = kIsNotArc;
            break;
        case NArchive::NExtract::NOperationResult::kHeadersError:
            s = kHeadersError;
            break;
        }
        if (s)
        {
            Print("Error : ");
            Print(s);
        }
        else
        {
            char temp[16];
            ConvertUInt32ToString(operationResult, temp);
            Print("Error #");
            Print(temp);
        }
    }
    }

    if (_outFileStream)
    {
        if (_processedFileInfo.MTimeDefined)
            _outFileStreamSpec->SetMTime(&_processedFileInfo.MTime);
        RINOK(_outFileStreamSpec->Close());
    }
    _outFileStream.Release();
    if (_extractMode && _processedFileInfo.AttribDefined)
        SetFileAttrib_PosixHighDetect(_diskFilePath, _processedFileInfo.Attrib);
    PrintNewLine();
    return S_OK;
}


STDMETHODIMP CArchiveExtractCallback::CryptoGetTextPassword(BSTR *password)
{
    if (!PasswordIsDefined)
    {
        // You can ask real password here from user
        // Password = GetPassword(OutStream);
        // PasswordIsDefined = true;
        PrintError("Password is not defined");
        return E_ABORT;
    }
    return StringToBstr(Password, password);
}



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
    Q_UNUSED(completeValue);
#ifdef DEBUG_LOGOUT_ON
    qDebug("Compress %.2f%%", static_cast<float>(*completeValue) / FileSize * 100.0f );
#endif
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
        PrintNewLine();
        m_NeedBeClosed = false;
    }
    return S_OK;
}

static void GetStream2(const wchar_t *name)
{
    if (name[0] == 0)
        name = kEmptyFileAlias;
//    Print("Compressing  ");
//    Print(name);
#ifdef DEBUG_LOGOUT_ON
    qDebug() << "Compressing " << QString::fromWCharArray(name);
#endif
}

STDMETHODIMP CArchiveUpdateCallback::GetStream(UInt32 index, ISequentialInStream **inStream)
{
    RINOK(Finilize());

    const CDirItem &dirItem = (*DirItems)[index];
    GetStream2(dirItem.Name);

    if (dirItem.isDir())
        return S_OK;

    {
        CInFileStream *inStreamSpec = new CInFileStream;
        CMyComPtr<ISequentialInStream> inStreamLoc(inStreamSpec);
        FString path = DirPrefix + dirItem.FullPath;
        if (!inStreamSpec->Open(path))
        {
            DWORD sysError = ::GetLastError();
            FailedCodes.Add(sysError);
            FailedFiles.Add(path);
            // if (systemError == ERROR_SHARING_VIOLATION)
            {
                PrintNewLine();
                PrintError("WARNING: can't open file");
                // Print(NError::MyFormatMessageW(systemError));
                return S_FALSE;
            }
            // return sysError;
        }
        *inStream = inStreamLoc.Detach();
    }
    return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::SetOperationResult(Int32 /* operationResult */)
{
    m_NeedBeClosed = true;
    return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::GetVolumeSize(UInt32 index, UInt64 *size)
{
    if (VolumesSizes.Size() == 0)
        return S_FALSE;
    if (index >= (UInt32)VolumesSizes.Size())
        index = VolumesSizes.Size() - 1;
    *size = VolumesSizes[index];
    return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::GetVolumeStream(UInt32 index, ISequentialOutStream **volumeStream)
{
    wchar_t temp[16];
    ConvertUInt32ToString(index + 1, temp);
    UString res = temp;
    while (res.Len() < 2)
        res.InsertAtFront(L'0');
    UString fileName = VolName;
    fileName += '.';
    fileName += res;
    fileName += VolExt;
    COutFileStream *streamSpec = new COutFileStream;
    CMyComPtr<ISequentialOutStream> streamLoc(streamSpec);
    if (!streamSpec->Create(us2fs(fileName), false))
        return ::GetLastError();
    *volumeStream = streamLoc.Detach();
    return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::CryptoGetTextPassword2(Int32 *passwordIsDefined, BSTR *password)
{
    if (!PasswordIsDefined)
    {
        if (AskPassword)
        {
            // You can ask real password here from user
            // Password = GetPassword(OutStream);
            // PasswordIsDefined = true;
            PrintError("Password is not defined");
            return E_ABORT;
        }
    }
    *passwordIsDefined = BoolToInt(PasswordIsDefined);
    return StringToBstr(Password, password);
}
#endif

Q7Zip::Q7Zip(QObject *parent) :
    QObject(parent),
    m_7zLib(kDllName)
{
    qRegisterMetaType<Q7Zip::Operation>();
    static_cast<void>(QObject::connect(this, SIGNAL(operation_signal_compress(const QString, const QStringList, const QString)), this, SLOT(operation_slot_compress(const QString, const QStringList, const QString)), Qt::QueuedConnection));
    static_cast<void>(QObject::connect(this, SIGNAL(operation_signal_extract(const QString, const QString)), this, SLOT(operation_slot_extract(const QString, const QString)), Qt::QueuedConnection));
}

int Q7Zip::init(void)
{
    int init_result = 1;
    bool loadResult = m_7zLib.load();

    if (true == loadResult){
#ifdef DEBUG_LOGOUT_ON
        qDebug() << "Q7Zip" << kDllName << "load success.";
#endif
        init_result = 0;
    }
    else{
        qDebug() << "Q7Zip" << kDllName << "load failure!!!";
        init_result = 1;
    }

    return init_result;
}

QString Q7Zip::lzma_sdk_version(void)
{
    return QString(MY_VERSION " ("  MY_DATE  ")");
}

int Q7Zip::compress(const QString &archive_name, const QStringList &compress_filelist, const QString &working_path)
{
    Func_CreateObject createObjectFunc = (Func_CreateObject)m_7zLib.resolve("CreateObject");
    if (!createObjectFunc)
    {
        PrintError("Can not get CreateObject");
        return 1;
    }

    CObjectVector<CDirItem> dirItems;
    {
        for (const QString &name : compress_filelist)
        {
            QString compress_name;
            CDirItem di;
            NFind::CFileInfo fi;
            if (!fi.Find(name.toStdWString().c_str()))
            {
                qDebug() << "Can't find file" << name;
                return 1;
            }

            if (true == name.startsWith(working_path)){
                QString temp_filename = name;
                compress_name = temp_filename.remove(working_path);
            }

            di.Attrib = fi.Attrib;
            di.Size = fi.Size;
            di.CTime = fi.CTime;
            di.ATime = fi.ATime;
            di.MTime = fi.MTime;
            di.Name = fs2us(compress_name.toStdWString().c_str());
            di.FullPath = name.toStdWString().c_str();
            dirItems.Add(di);
        }
    }

    COutFileStream *outFileStreamSpec = new COutFileStream;
    CMyComPtr<IOutStream> outFileStream = outFileStreamSpec;
    if (!outFileStreamSpec->Create(archive_name.toStdWString().c_str(), true))
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

    if (updateCallbackSpec->FailedFiles.Size() != 0){
        return 1;
    }

    return 0;
}

int Q7Zip::extract(const QString &archive_name, const QString &output_path)
{
    Func_CreateObject createObjectFunc = (Func_CreateObject)m_7zLib.resolve("CreateObject");
    if (!createObjectFunc)
    {
        PrintError("Can not get CreateObject");
        return 1;
    }

    CMyComPtr<IInArchive> archive;
    if (createObjectFunc(&CLSID_Format, &IID_IInArchive, (void **)&archive) != S_OK)
    {
        PrintError("Can not get class object");
        return 1;
    }

    CInFileStream *fileSpec = new CInFileStream;
    CMyComPtr<IInStream> file = fileSpec;

    if (!fileSpec->Open(archive_name.toStdWString().c_str()))
    {
        PrintError("Can not open archive file", archive_name.toStdWString().c_str());
        return 1;
    }

    {
        CArchiveOpenCallback *openCallbackSpec = new CArchiveOpenCallback;
        CMyComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);
        openCallbackSpec->PasswordIsDefined = false;
        // openCallbackSpec->PasswordIsDefined = true;
        // openCallbackSpec->Password = L"1";

        const UInt64 scanSize = 1 << 23;
        if (archive->Open(file, &scanSize, openCallback) != S_OK)
        {
            PrintError("Can not open file as archive", archive_name.toStdWString().c_str());
            return 1;
        }
    }

    // Extract command
    CArchiveExtractCallback *extractCallbackSpec = new CArchiveExtractCallback;
    CMyComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
    extractCallbackSpec->Init(archive, FString(output_path.toStdWString().c_str())); // second parameter is output folder path
    extractCallbackSpec->PasswordIsDefined = false;
    // extractCallbackSpec->PasswordIsDefined = true;
    // extractCallbackSpec->Password = "1";

    /*
      const wchar_t *names[] =
      {
        L"mt",
        L"mtf"
      };
      const unsigned kNumProps = sizeof(names) / sizeof(names[0]);
      NCOM::CPropVariant values[kNumProps] =
      {
        (UInt32)1,
        false
      };
      CMyComPtr<ISetProperties> setProperties;
      archive->QueryInterface(IID_ISetProperties, (void **)&setProperties);
      if (setProperties)
        setProperties->SetProperties(names, values, kNumProps);
      */

    HRESULT result = archive->Extract(NULL, (UInt32)(Int32)(-1), false, extractCallback);

    if (result != S_OK)
    {
        PrintError("Extract Error");
        return 1;
    }

    return 0;
}

int Q7Zip::showfilelist(const QString &archive_name)
{
    Q_UNUSED(archive_name);
    return 0;
}

void Q7Zip::threadStarted(void)
{
    int init_result = init();
    emit threadStarted_signal(init_result);
}

void Q7Zip::operation_slot_compress(const QString archive_name, const QStringList compress_filelist, const QString working_path)
{
    int operate_result = 0;

    operate_result = compress(archive_name, compress_filelist, working_path);

    emit operation_result_signal(Q7Zip::Q7ZIP_COMPRESS, archive_name, operate_result);
}

void Q7Zip::operation_slot_extract(const QString archive_name, const QString output_path)
{
    int operate_result = 0;

    operate_result = extract(archive_name, output_path);

    emit operation_result_signal(Q7Zip::Q7ZIP_EXTRACT, archive_name, operate_result);
}

#include "pch.h"
#include "CMyPe.h"

CMyPe::CMyPe()
{
  Init();
}

CMyPe::CMyPe(void* pFileBuff)
{
  Init();
  InitPeFormat(pFileBuff);
  m_bIsMemInit = 1;
}

CMyPe::CMyPe(const char* strFilePath)
{
  Init();
  InitPeFormat(strFilePath);
  m_bIsMemInit = 0;
}

CMyPe::~CMyPe()
{
  if (m_lpFileBuff != NULL && m_bIsMemInit != 1)
  {
    ::UnmapViewOfFile(m_lpFileBuff);
    m_lpFileBuff = NULL;
  }

  if (m_hFileMap != NULL)
  {
    ::CloseHandle(m_hFileMap);
    m_hFileMap = NULL;
  }

  if (m_hFile != INVALID_HANDLE_VALUE)
  {
    ::CloseHandle(m_hFile);
    m_hFile = INVALID_HANDLE_VALUE;
  }
}

int CMyPe::IsPeFile(void* pFileBuff)
{
  // �ж��Ƿ�PE�ļ�
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileBuff;
  if (pDosHeader->e_magic != 'ZM')
  {
    return FILE_NOTPE;
  }
  PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((char*)pFileBuff + pDosHeader->e_lfanew);
  if (pNtHeader->Signature != 'EP')
  {
    return FILE_NOTPE;
  }

  return FILE_ISPE;
}

int CMyPe::IsPeFile(const char* strFilePath)
{
  // ���ļ�
  HANDLE hFile = ::CreateFile(strFilePath,            // �ļ�·��
                              GENERIC_READ | GENERIC_WRITE,  // �ļ��Ĵ򿪷�ʽ
                              FILE_SHARE_READ,        // ����ģʽ�������ļ��ɶ�
                              NULL,                   // ��ȫ���ԣ�����ȷ�����صľ���Ƿ���Ա��ӽ��̼̳�
                              OPEN_EXISTING,          // �򿪷�ʽ
                              FILE_ATTRIBUTE_NORMAL,  // �ļ�����
                              NULL);
  if (hFile == INVALID_HANDLE_VALUE)
  {
    return FIlE_OPENFAILD;
  }

  // ��ȡMZ��־
  WORD wMzMagic = 0;
  DWORD dwNumberOfBytesRead = 0;
  int nRet = ::ReadFile(hFile, &wMzMagic, sizeof(WORD), &dwNumberOfBytesRead, NULL);
  if (nRet == 0) { goto OPENFAILD; }
  if (wMzMagic != 'ZM') { goto NOTPE; }

  // ��ȡPE��־
  DWORD dwOffset = 0;
  WORD wPeMagic = 0;
  DWORD dwPtr = ::SetFilePointer(hFile, 0x3c, NULL, FILE_BEGIN);
  if (dwPtr == INVALID_SET_FILE_POINTER) { goto OPENFAILD; }

  nRet = ::ReadFile(hFile, &dwOffset, sizeof(DWORD), &dwNumberOfBytesRead, NULL);
  if (nRet == 0){ goto OPENFAILD; }

  dwPtr = ::SetFilePointer(hFile, dwOffset, NULL, FILE_BEGIN);
  if (dwPtr == INVALID_SET_FILE_POINTER){ goto OPENFAILD; }

  nRet = ::ReadFile(hFile, &wPeMagic, sizeof(wPeMagic), &dwNumberOfBytesRead, NULL);
  if (nRet == 0){ goto OPENFAILD; }
  if (wPeMagic != 'EP'){ goto NOTPE; }

  ::CloseHandle(hFile);
  return FILE_ISPE;

OPENFAILD:
  ::CloseHandle(hFile);
  return FIlE_OPENFAILD;

NOTPE:
  ::CloseHandle(hFile);
  return FILE_NOTPE;
}

void CMyPe::Init()
{
  m_lpFileBuff = NULL;
  m_pDosHeader = NULL;
  m_pNtHeader = NULL;
  m_pFileHeader = NULL;
  m_pOptionHeader = NULL;
  m_pSectionHeader = NULL;
  m_pExportDirectory = NULL;
  m_pImportDirectory = NULL;
  m_pResourceDirectory = NULL;
  m_pRelocDirectory = NULL;
  m_pTlsDirectory = NULL;
  m_dwExportSize = 0;
  m_dwImportSize = 0;
  m_dwRelocSize = 0;

  m_wNumberOfSections = 0;
  m_dwAddressOfEntryPoint = 0;
  m_dwImageBase = 0;
  m_dwSectionAlignment = 0;
  m_dwFileAlignment = 0;
  m_dwSizeOfImage = 0;
  m_dwSizeOfHeaders = 0;
  m_dwNumberOfRvaAndSizes = 0;

  m_hFile = INVALID_HANDLE_VALUE;
  m_dwFileSize = 0;
  m_hFileMap = NULL;
  m_bIsMemInit = 0;
}

void CMyPe::InitPeFormat(void* pFileBuff)
{
  if (IsPeFile(pFileBuff) != FILE_ISPE)
  {
    return;
  }
  m_lpFileBuff = pFileBuff;
  m_pDosHeader = (PIMAGE_DOS_HEADER)pFileBuff;
  m_pNtHeader = (PIMAGE_NT_HEADERS)((char*)pFileBuff + m_pDosHeader->e_lfanew);
  m_pFileHeader = (PIMAGE_FILE_HEADER)(&m_pNtHeader->FileHeader);
  m_pOptionHeader = (PIMAGE_OPTIONAL_HEADER)(&m_pNtHeader->OptionalHeader);
  m_pSectionHeader = (PIMAGE_SECTION_HEADER)((char*)m_pOptionHeader + m_pFileHeader->SizeOfOptionalHeader);

  m_wNumberOfSections = m_pFileHeader->NumberOfSections;
  m_dwAddressOfEntryPoint = m_pOptionHeader->AddressOfEntryPoint;
  m_dwImageBase = m_pOptionHeader->ImageBase;
  m_dwSectionAlignment = m_pOptionHeader->SectionAlignment;
  m_dwFileAlignment = m_pOptionHeader->FileAlignment;
  m_dwSizeOfImage = m_pOptionHeader->SizeOfImage;
  m_dwSizeOfHeaders = m_pOptionHeader->SizeOfHeaders;
  m_dwNumberOfRvaAndSizes = m_pOptionHeader->NumberOfRvaAndSizes;

  // ������
  DWORD dwExportRva = m_pOptionHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
  m_dwExportSize = m_pOptionHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
  m_pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)(Rva2Fa(dwExportRva) + (char*)m_lpFileBuff);

  // �����
  DWORD dwImportRva = m_pOptionHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
  m_dwImportSize = m_pOptionHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
  m_pImportDirectory = (PIMAGE_IMPORT_DESCRIPTOR)(Rva2Fa(dwImportRva) + (char*)m_lpFileBuff);

  // ��Դ��
  DWORD dwResourceRva = m_pOptionHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
  m_pResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)(Rva2Fa(dwResourceRva) + (char*)m_lpFileBuff);

  // �ض�λ��
  DWORD dwRelocRva = m_pOptionHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
  m_dwRelocSize = m_pOptionHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
  m_pRelocDirectory = (PIMAGE_BASE_RELOCATION)(Rva2Fa(dwRelocRva) + (char*)m_lpFileBuff);

  // TLS��
  DWORD dwTlsRva = m_pOptionHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress;
  m_pTlsDirectory = (PIMAGE_TLS_DIRECTORY)(Rva2Fa(dwTlsRva) + (char*)m_lpFileBuff);
}

void CMyPe::InitPeFormat(const char* strFilePath)
{
  if (IsPeFile(strFilePath) != FILE_ISPE)
  {
    return;
  }
  // ���ļ�
  m_hFile = ::CreateFile(strFilePath,           // �ļ�·��
                        GENERIC_READ | GENERIC_WRITE,  // �ļ��Ĵ򿪷�ʽ
                        FILE_SHARE_READ,        // ����ģʽ�������ļ��ɶ�
                        NULL,                   // ��ȫ���ԣ�����ȷ�����صľ���Ƿ���Ա��ӽ��̼̳�
                        OPEN_EXISTING,          // �򿪷�ʽ
                        FILE_ATTRIBUTE_NORMAL,  // �ļ�����
                        NULL);
  if (m_hFile == INVALID_HANDLE_VALUE)
  {
    return;
  }

  // ��ȡ�ļ���С
  m_dwFileSize = ::GetFileSize(m_hFile, NULL);

  // �����ļ�ӳ�����
  m_hFileMap = ::CreateFileMapping(m_hFile,  // �ļ����
                                    NULL,    // ��ȫ���ԣ�����ȷ�����صľ���Ƿ���Ա��ӽ��̼̳�
                                    PAGE_READWRITE, // ӳ����ڴ�Ҳ���ڴ�����
                                    NULL,    // ����4Gʱ����
                                    m_dwFileSize, // ӳ���С
                                    NULL);   // �ļ�ӳ���������ƣ����ú�����ڽ��̼�ͨ��
  if (m_hFileMap == NULL)
  {
    return;
  }

  // ���ļ�ӳ�䵽�ڴ�
  m_lpFileBuff = ::MapViewOfFile(m_hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  if (m_lpFileBuff == NULL) {
    return;
  }

  InitPeFormat(m_lpFileBuff);
}

DWORD CMyPe::Rva2Fa(DWORD dwRva, LPVOID lpImageBase)
{
  // �ж�RVA�Ƿ���Ч,RVA�������ģ���ַ��
  DWORD dwImageBase = m_dwImageBase;
  if (lpImageBase != NULL)
  {
    dwImageBase = (DWORD)lpImageBase;
  }

  DWORD dwVa = dwImageBase + dwRva;
  if (dwVa < dwImageBase || dwVa >= dwImageBase + m_dwSizeOfImage)
  {
    return -1;
  }
  
  // �����ڱ���ȡFA
  PIMAGE_SECTION_HEADER pSectionHeader = m_pSectionHeader;
  for (int i = 0; i < m_wNumberOfSections; ++i)
  {
    DWORD dwVirtualAddress = pSectionHeader->VirtualAddress;  // ӳ�䵽�ڴ�ĵ�ַ��RVA
    DWORD dwVirtualSize = pSectionHeader->Misc.VirtualSize;   // ӳ�䵽�ڴ�����ݴ�С��OS�Ὣ��ֵ����������ڴ�
    DWORD dwPointerToRawData = pSectionHeader->PointerToRawData; // �ļ������ݵ�ƫ��
    DWORD dwSizeOfRawData = pSectionHeader->SizeOfRawData;    // �ļ������ݶ�����С

    if (dwRva >= dwVirtualAddress && dwRva < dwVirtualAddress + dwSizeOfRawData)
    {
      return dwRva - dwVirtualAddress + dwPointerToRawData;
    }
    pSectionHeader++;
  } 

  return -1;
}

LPVOID CMyPe::GetDosHeaderPointer()
{
  return m_pDosHeader;
}

LPVOID CMyPe::GetNtHeaderPointer()
{
  return m_pNtHeader;
}

LPVOID CMyPe::GetFileHeaderPointer()
{
  return m_pFileHeader;
}

LPVOID CMyPe::GetOptionHeaderPointer()
{
  return m_pOptionHeader;
}

LPVOID CMyPe::GetSectionHeaderPointer()
{
  return m_pSectionHeader;
}

LPVOID CMyPe::GetExportDirectoryPointer()
{
  return m_pExportDirectory;
}

DWORD CMyPe::GetExportDirectorySize()
{
  return m_dwExportSize;
}

LPVOID CMyPe::GetImportDirectoryPointer()
{
  return m_pImportDirectory;
}

LPVOID CMyPe::GetResourceDirectoryPointer()
{
  return m_pResourceDirectory;
}

LPVOID CMyPe::GetRelocDirectoryPointer()
{
  return m_pRelocDirectory;
}

DWORD CMyPe::GetRelocDirectorySize()
{
  return m_dwRelocSize;
}

LPVOID CMyPe::GetTlsDirectoryPointer()
{
  return m_pTlsDirectory;
}

WORD CMyPe::GetNumberOfSections()
{
  return m_wNumberOfSections;
}

DWORD CMyPe::GetAddressOfEntryPoint()
{
  return m_dwAddressOfEntryPoint;
}

DWORD CMyPe::GetImageBase()
{
  return m_dwImageBase;
}

DWORD CMyPe::GetSectionAlignment()
{
  return m_dwSectionAlignment;
}

DWORD CMyPe::GetFileAlignment()
{
  return m_dwFileAlignment;
}

DWORD CMyPe::GetSizeOfImage()
{
  return m_dwSizeOfImage;
}

DWORD CMyPe::GetSizeOfHeaders()
{
  return m_dwSizeOfHeaders;
}

DWORD CMyPe::GetNumberOfRvaAndSizes()
{
  return m_dwNumberOfRvaAndSizes;
}

#pragma once
#include <Windows.h>

class CMyPe
{
public:
  CMyPe();
  CMyPe(void* pFileBuff);
  CMyPe(const char* strFilePath);
  ~CMyPe();

public:
  enum {
    FIlE_OPENFAILD,
    FILE_NOTPE,
    FILE_ISPE
  };
  static int IsPeFile(void* pFileBuff);
  static int IsPeFile(const char* strFilePath);

public:
  DWORD Rva2Fa(DWORD dwRva, LPVOID lpImageBase = NULL);

  // �������ʹ��
  static LPVOID MyGetProcAddress(HMODULE hInst, LPCSTR lpProcName); // ��ʵ�ֵ�GetProcAddress
  static LPVOID MyGetProcFunName(LPVOID pfnAddr); // ͨ��������ַ��ȡ��������/���
  
  
private:
  void Init();
  void InitPeFormat(void* pFileBuff);
  void InitPeFormat(const char* strFilePath);
  LPVOID GetExportName(DWORD dwOrdinal); // ͨ���������ȡ����������

private:
  HANDLE m_hFile;
  HANDLE m_hFileMap;
  DWORD  m_dwFileSize;
  LPVOID m_lpFileBuff;
  BOOL   m_bIsMemInit; // �������ʱ��������ڴ��ַ������Ҫȡ���ļ�ӳ��

public:
  LPVOID GetDosHeaderPointer();
  LPVOID GetNtHeaderPointer();
  LPVOID GetFileHeaderPointer();
  LPVOID GetOptionHeaderPointer();
  LPVOID GetSectionHeaderPointer();

  LPVOID GetExportDirectoryPointer();
  LPVOID GetImportDirectoryPointer();
  LPVOID GetResourceDirectoryPointer();
  LPVOID GetRelocDirectoryPointer();
  LPVOID GetTlsDirectoryPointer();
  DWORD  GetExportDirectorySize();
  DWORD  GetRelocDirectorySize();

private:
  PIMAGE_DOS_HEADER      m_pDosHeader;
  PIMAGE_NT_HEADERS      m_pNtHeader;
  PIMAGE_FILE_HEADER     m_pFileHeader;
  PIMAGE_OPTIONAL_HEADER m_pOptionHeader;
  PIMAGE_SECTION_HEADER  m_pSectionHeader;

  PIMAGE_EXPORT_DIRECTORY   m_pExportDirectory;   // ������λ������Ŀ¼��0��
  PIMAGE_IMPORT_DESCRIPTOR  m_pImportDirectory;   // �����λ������Ŀ¼��1��
  PIMAGE_RESOURCE_DIRECTORY m_pResourceDirectory; // ��Դ��λ������Ŀ¼��2��
  PIMAGE_BASE_RELOCATION    m_pRelocDirectory;    // �ض�λ��λ������Ŀ¼��5��
  PIMAGE_TLS_DIRECTORY      m_pTlsDirectory;      // TLSλ������Ŀ¼��9��

  DWORD m_dwExportSize; // �������size�����õ�
  DWORD m_dwImportSize; // importʹ��ȫ0�ṹ��β
  DWORD m_dwRelocSize;  // �ض�λ���size�����õģ�����ʱ������

public:
  // PE�ṹ������Ҫ�ֶ�
  WORD  GetNumberOfSections();
  DWORD GetAddressOfEntryPoint();
  DWORD GetImageBase();
  DWORD GetSectionAlignment();
  DWORD GetFileAlignment();
  DWORD GetSizeOfImage();
  DWORD GetSizeOfHeaders();
  DWORD GetNumberOfRvaAndSizes();

private:
  WORD  m_wNumberOfSections;      // �ڱ�ĸ���
  DWORD m_dwAddressOfEntryPoint;  // ������ڵ㣬RVA
  DWORD m_dwImageBase;            // ����Ľ���װ�ص�ַ
  DWORD m_dwSectionAlignment;     // ���ڴ��еĶ��� 
  DWORD m_dwFileAlignment;        // ���ļ��еĶ���
  DWORD m_dwSizeOfImage;          // �ļ������ڴ沢�����Ĵ�С
  DWORD m_dwSizeOfHeaders;        // PEͷ�Ĵ�С
  DWORD m_dwNumberOfRvaAndSizes;  // ����Ŀ¼�ĸ���
};


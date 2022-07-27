# Pe File Parser
- 熟悉Pe文件结构，程序只支持32位
- PE格式中重要的数据目录：
  - 导出表：熟悉导出表，能自实现GetProcAddress()
  ```
  /*
	typedef struct _IMAGE_EXPORT_DIRECTORY {
	  DWORD   Characteristics;            
	  DWORD   TimeDateStamp;          
	  WORD    MajorVersion;               
	  WORD    MinorVersion;           
	  DWORD   Name;                   // dll名称
	  DWORD   Base;                   // 序号查询时会用上，数组的坐标平移
	  DWORD   NumberOfFunctions;      // 有多少个被导出的项    
	  DWORD   NumberOfNames;          // 有多少个被名称导出的项
	  DWORD   AddressOfFunctions;     // 导出地址表，rva
	  DWORD   AddressOfNames;         // 导出名称表，rva
	  DWORD   AddressOfNameOrdinals;  // 导出序号表，rva
	} IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;
	*/

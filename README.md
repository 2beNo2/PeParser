# Pe File Parser
- 熟悉Pe文件结构，程序只支持32位
- PE格式中重要的数据目录：
  - **导出表**：了解导出表，可以自己实现GetProcAddress()、GetProcFunName()等方法
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
  ```
  
  - **导入表**：了解导入表，加壳/脱壳都需要对导入表进行操作，可利用导入表进行注入
  ```
  typedef struct _IMAGE_IMPORT_DESCRIPTOR {
	    union {
	        DWORD   Characteristics;            
	        DWORD   OriginalFirstThunk;         // 导入名称表(INT)的RVA，GetProAddress
	    } DUMMYUNIONNAME;
	    DWORD   TimeDateStamp;                  // 忽略
	    DWORD   ForwarderChain;                 // 忽略
	    DWORD   Name;                           // dll名称的地址，LoadLibrary
	    DWORD   FirstThunk;                     // 指向导入地址表(IAT)的RVA，pfn填在此处
	} IMAGE_IMPORT_DESCRIPTOR;
	typedef IMAGE_IMPORT_DESCRIPTOR UNALIGNED *PIMAGE_IMPORT_DESCRIPTOR;
  ```
  	`INT表`和`IAT表`都是`IMAGE_THUNK_DATA`结构的
  ```
  typedef struct _IMAGE_THUNK_DATA32 {
	  union {
	    PBYTE  ForwarderString;                 //转发字符串的RVA；
	    PDWORD Function;                        //导入函数的地址；
	    DWORD Ordinal;                          //导入函数的序号；
	    PIMAGE_IMPORT_BY_NAME  AddressOfData;   //指向IMAGE_IMPORT_BY_NAME；
	  } u1;
	} IMAGE_THUNK_DATA32;

	// IMAGE_THUNK_DATA32 在不同的状态下有不同的解释方式：
	// 在文件状态下解释为 PIMAGE_IMPORT_BY_NAME
	// 进程状态下是函数地址
	// 如果是序号导入的函数，最高位应该为一，取LWORD作为序号

	typedef struct _IMAGE_IMPORT_BY_NAME {
	  WORD Hint;     // 编译器添加的当前电脑中对应函数的序号，无实际意义
	  BYTE Name[1];  // 字符串
	} IMAGE_IMPORT_BY_NAME, *PIMAGE_IMPORT_BY_NAME;
  ```
  
  - **重定位表**：LoadPe和加壳器，需要对重定位表中的数据做重定位处理
  ```
  typedef struct _IMAGE_BASE_RELOCATION {
	    DWORD   VirtualAddress;  // 页起始地址RVA，通知系统该分页上有数据需要重定位
	    DWORD   SizeOfBlock;     // 整个数据块的大小，包含SizeOfBlock
	//  WORD    TypeOffset[1];   // 柔性数组，保存了要修正的数据相对于页的偏移
				     // 低12位表偏移
			             // 数组成员的高4位，决定了修复的方式，是修正4个字节还是8个字节
				     // 高4位为0，表示无效，用来对齐
			             // 高4位为3，表示修4字节
			             // 高4位为0xA，表示修8字节
	} IMAGE_BASE_RELOCATION;
	typedef IMAGE_BASE_RELOCATION UNALIGNED * PIMAGE_BASE_RELOCATION;
  ```
  
  - **TLS表**：TLS的回调函数可用于反调试，装载PE时可拷贝一份TLS节表到装载器中
  ```
  typedef struct _IMAGE_TLS_DIRECTORY32 {
	  DWORD StartAddressOfRawData; // VA，TLS数据块的起始，VA - imagebase = RVA
	  DWORD EndAddressOfRawData;   // VA，TLS数据块的结束
	  PDWORD AddressOfIndex;       // VA，该TLS数据块在ThreadLocalStoragePointer 指针数组的下标
	  PIMAGE_TLS_CALLBACK *AddressOfCallBacks; // VA，以0结尾的函数指针数组，手动填写函数指针时，注意也是VA
						// 函数指针数组保存了回调函数的指针，用于初始化TLS
						// 和dllMain的格式一样
	  DWORD SizeOfZeroFill;
	  DWORD Characteristics;
	} IMAGE_TLS_DIRECTORY32;
  ```
  
  - **资源表**：`FindResource`、`LoadResource`、`LockResource`等获取资源的API实际是对PE格式解析，获取对应资源的地址
  	- `资源目录：`目录都解释为该结构体：
  	```
	typedef struct _IMAGE_RESOURCE_DIRECTORY {
	    DWORD   Characteristics;
	    DWORD   TimeDateStamp;
	    WORD    MajorVersion;
	    WORD    MinorVersion;
	    WORD    NumberOfNamedEntries;  // 以名称命名的资源数量
	    WORD    NumberOfIdEntries;     // 以id命名的资源数量
	    // NumberOfNamedEntries + NumberOfIdEntries 来确定资源目录项的个数
	} IMAGE_RESOURCE_DIRECTORY, *PIMAGE_RESOURCE_DIRECTORY;
	```
  	- `资源目录项：`是资源目录中的元素:
  	```
	typedef struct _IMAGE_RESOURCE_DIRECTORY_ENTRY {
	    union {
		struct {
		    DWORD NameOffset:31;  //偏移量是按照资源节的起始来计算的
		    DWORD NameIsString:1; //如果最高位为1，则剩下31位填写 名称字符串的偏移量
					//资源的字符串都是pascal字符串格式，
					//第一个字节表字符个数，后面每2个字节保存一个字符
					//如果最高位为0，则LDWORD保存id号        
					//ID号有宏定义：RT_ICON = 3
		};
		DWORD   Name;　　　　　　　　　　　　　　　　　
		WORD    Id;
	    };
	    union {
		DWORD   OffsetToData;    //描述是数据还是目录
		struct {
		    DWORD   OffsetToDirectory:31; //偏移量是按照资源节的起始来计算的
		    DWORD   DataIsDirectory:1;    //如果最高位为1，则为下一个目录的偏移量
						  //如果最高位为0，则为下一个数据的偏移量
		};
	    };
	} IMAGE_RESOURCE_DIRECTORY_ENTRY, *PIMAGE_RESOURCE_DIRECTORY_ENTRY;
	```
	- `数据：`数据都解释为该结构体:
	```
	typedef struct _IMAGE_RESOURCE_DATA_ENTRY {
	    DWORD   OffsetToData;  // 数据的偏移，rva
	    DWORD   Size;　　　　　　// 数据的大小　　　　　　　　　
	    DWORD   CodePage;
	    DWORD   Reserved;
	} IMAGE_RESOURCE_DATA_ENTRY, *PIMAGE_RESOURCE_DATA_ENTRY;
	```

.386
.model flat, stdcall  ;32 bit memory model
option casemap :none  ;case sensitive

include LoadDll.inc

.const
	g_szFileName db 'TestDll.dll', 0
	g_szFunName db 'ShowMsg', 0

.code

MyMemCmp proc uses edi esi ecx lpDstAddress:LPVOID, lpSrcAddress:LPVOID, dwSize:DWORD
	cld
	mov edi, lpDstAddress
	mov esi, lpSrcAddress
	mov ecx, dwSize
	repz cmpsb  ;相等时循环
	jnz NOT_EQUAL
	mov eax, 0
	ret
NOT_EQUAL:
	sub edi, esi
	mov eax, edi
	ret
MyMemCmp endp


MyMemCopy proc uses edi esi ecx edx lpDstAddress:LPVOID, lpSrcAddress:LPVOID, dwSize:DWORD
	cld
	mov edi, lpDstAddress
	mov esi, lpSrcAddress
	mov eax, dwSize
	xor edx, edx
	mov ecx, 4
	div ecx
	mov ecx, eax
	rep movsd
	mov ecx, edx
	rep movsb
	ret
MyMemCopy endp


RepairReloc proc uses edi esi ebx ecx edx lpDstAddress:LPVOID, lpFileBuff:LPVOID
	LOCAL @dwImageBase:DWORD
	LOCAL @lpAddressOfReloc:LPVOID
	LOCAL @dwRelocSize:DWORD
	LOCAL @dwReloadValue:DWORD
	LOCAL @dwRelocItemCount:DWORD

	xor eax, eax
	mov @dwImageBase, eax
	mov @lpAddressOfReloc, eax
	mov @dwRelocSize, eax
	mov @dwReloadValue, eax
	mov @dwRelocItemCount, eax

	; 获取重定位表相关数据
	mov esi, lpFileBuff
	assume esi : ptr IMAGE_DOS_HEADER
	add esi, [esi].e_lfanew
	assume esi : ptr IMAGE_NT_HEADERS
	mov eax, [esi].OptionalHeader.ImageBase
	mov @dwImageBase, eax

	lea eax, [esi].OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC * sizeof IMAGE_DATA_DIRECTORY]
	assume eax : ptr IMAGE_DATA_DIRECTORY
	mov ebx, [eax].isize
	mov @dwRelocSize, ebx

	mov eax, [eax].VirtualAddress
	add eax, lpDstAddress
	mov @lpAddressOfReloc, eax

	; 获取修正值
	mov eax, lpDstAddress
	sub eax, @dwImageBase
	mov @dwReloadValue, eax

	; 遍历重定位表进行修正
	mov esi, @lpAddressOfReloc
	assume esi : ptr IMAGE_BASE_RELOCATION

	xor ecx, ecx
	.while ecx != @dwRelocSize
		mov edi, [esi].VirtualAddress ;页起始地址RVA
		add edi, lpDstAddress ;页起始地址
	
		mov eax, [esi].SizeOfBlock
		sub eax, sizeof IMAGE_BASE_RELOCATION
		shr eax, 1 
		mov @dwRelocItemCount, eax  ;得到了该分页上需要重定位的数据个数

		mov ebx, esi
		add ebx, sizeof IMAGE_BASE_RELOCATION
		push ecx

		xor ecx, ecx
		.while ecx != @dwRelocItemCount
			movzx eax, word ptr [ebx + ecx * sizeof word]
			and eax, 0f000h
			shr eax, 12

			; 判断修正方式
			.if eax == IMAGE_REL_BASED_HIGHLOW
				; 0x3，修复4字节
				movzx eax, word ptr [ebx + ecx * sizeof word]
				and eax, 0fffh
				add eax, edi  ;获取到要修正的数据在内存的地址

				mov edx, dword ptr [eax]
				add edx, @dwReloadValue
				mov dword ptr [eax], edx ;修正
			.endif

			inc ecx
		.endw

		pop ecx
		add ecx, [esi].SizeOfBlock
		add esi, [esi].SizeOfBlock 
	.endw

	ret
RepairReloc endp


RepairImportTable proc uses edi esi edx ebx ecx lpDstAddress:LPVOID, lpFileBuff:LPVOID
	LOCAL @pAddressOfImport:LPVOID
	LOCAL @ZeroImport:IMAGE_IMPORT_DESCRIPTOR
	LOCAL @hDllModule:HMODULE

	xor eax, eax
	mov @pAddressOfImport, eax
	mov @hDllModule, eax
	invoke RtlZeroMemory, addr @ZeroImport, sizeof IMAGE_IMPORT_DESCRIPTOR

	; 获取导入表位置
	mov esi, lpFileBuff
	assume esi : ptr IMAGE_DOS_HEADER
	add esi, [esi].e_lfanew
	assume esi : ptr IMAGE_NT_HEADERS

	lea eax, [esi].OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT * sizeof IMAGE_DATA_DIRECTORY]
	assume eax : ptr IMAGE_DATA_DIRECTORY
	mov eax, [eax].VirtualAddress
	add eax, lpDstAddress
	mov @pAddressOfImport,eax

	; 遍历导入表，全0结构结尾
	mov esi, @pAddressOfImport
	assume esi : ptr IMAGE_IMPORT_DESCRIPTOR
	invoke MyMemCmp, esi, addr @ZeroImport, sizeof IMAGE_IMPORT_DESCRIPTOR
	.while eax != 0
		; 判断是否是有效导入表项
		mov edi, [esi].FirstThunk
		add edi, lpDstAddress
		.if dword ptr [edi] != NULL
			; 判断INT是否为空，INT为空时则使用IAT
			mov edx,[esi].OriginalFirstThunk
			.if edx == NULL
				mov edx,[esi].FirstThunk
			.endif
			add edx, lpDstAddress
			
			mov eax, [esi].Name1
			add eax, lpDstAddress
			pushad
			invoke LoadLibrary, eax ; check it
			mov @hDllModule, eax
			popad
			
			; 循环INT/IAT
			.while dword ptr [edx] != NULL
				; 判断是符号导出还是名称导出
				mov ebx, [edx]
				and ebx, 80000000h
				.if ebx != 0
					; oridinal
					mov ebx, dword ptr [edx]
					and ebx, 0ffffh
				.elseif
					; name
					mov ebx, dword ptr [edx]
					add ebx, lpDstAddress
					assume ebx : ptr IMAGE_IMPORT_BY_NAME
					lea ebx, [ebx].Name1
				.endif

				push ecx
				push edx
				invoke GetProcAddress, @hDllModule, ebx
				pop edx
				pop ecx
				; check it

				mov dword ptr [edi], eax
				add edi, 4
				add edx, 4
			.endw
		.endif

		add esi, sizeof IMAGE_IMPORT_DESCRIPTOR
		invoke MyMemCmp, esi, addr @ZeroImport, sizeof IMAGE_IMPORT_DESCRIPTOR
	.endw

	ret
RepairImportTable endp


StretchPE proc uses edi esi ecx lpDstAddress:LPVOID, lpFileBuff:LPVOID
	LOCAL @dwNumberOfSections:DWORD
	LOCAL @dwSizeOfOptionalHeader:DWORD
	LOCAL @dwSizeOfHeaders:DWORD

	xor eax, eax
	mov @dwNumberOfSections, eax
	mov @dwSizeOfOptionalHeader, eax
	mov @dwSizeOfHeaders, eax

	mov esi, lpFileBuff
	assume esi : ptr IMAGE_DOS_HEADER
	add esi, [esi].e_lfanew
	assume esi : ptr IMAGE_NT_HEADERS
	movzx eax, [esi].FileHeader.NumberOfSections
	mov @dwNumberOfSections, eax
	movzx eax, [esi].FileHeader.SizeOfOptionalHeader
	mov @dwSizeOfOptionalHeader, eax
	mov eax, [esi].OptionalHeader.SizeOfHeaders
	mov @dwSizeOfHeaders, eax

	; 拷贝PE头
	invoke MyMemCopy, lpDstAddress, lpFileBuff, @dwSizeOfHeaders

	; 拷贝节区
	lea esi, [esi].OptionalHeader
	add esi, @dwSizeOfOptionalHeader
	assume esi : ptr IMAGE_SECTION_HEADER

	xor ecx, ecx
	.while ecx < @dwNumberOfSections
		; 首先确定节存在文件映射
		.if [esi].SizeOfRawData != 0
			mov edi, [esi].VirtualAddress
			add edi, lpDstAddress

			mov eax, [esi].PointerToRawData
			add eax, lpFileBuff
			invoke MyMemCopy, edi, eax, [esi].SizeOfRawData
		.endif
		inc ecx
		add esi, sizeof IMAGE_SECTION_HEADER
	.endw
	ret
StretchPE endp


IsPeFormat proc uses esi lpFileBuff:LPVOID
	mov esi, lpFileBuff
	assume esi : ptr IMAGE_DOS_HEADER
	.if [esi].e_magic != 'ZM'
		mov eax, FALSE
		ret
	.endif

	add esi, [esi].e_lfanew 
	assume esi : ptr IMAGE_NT_HEADERS
	.if [esi].Signature != 'EP'
		mov eax, FALSE
		ret
	.endif
	mov eax, TRUE
	ret
IsPeFormat endp


LoadDll proc hInst:HMODULE
	LOCAL @hFile:HANDLE
	LOCAL @hFileMap:HANDLE
	LOCAL @lpFileBuff:LPVOID
	LOCAL @dwSizoOfImage:DWORD
	LOCAL @lpDstAddress:LPVOID
	LOCAL @dwAddressOfEntryPoint:DWORD

	mov @hFile, INVALID_HANDLE_VALUE 
	xor eax, eax
	mov @hFileMap, eax
	mov @lpFileBuff, eax
	mov @dwSizoOfImage, eax
	mov @lpDstAddress, eax
	mov @dwAddressOfEntryPoint, eax

	; 首先映射文件到内存
	invoke CreateFile, offset g_szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, 
						OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
	.if eax == INVALID_HANDLE_VALUE
		jmp EXIT_PROC
	.endif
	mov @hFile, eax

	invoke CreateFileMapping, @hFile, NULL, PAGE_READONLY, 0, 0, NULL
	.if eax == NULL 
		jmp EXIT_PROC
	.endif
	mov @hFileMap,eax

	invoke MapViewOfFile, @hFileMap, FILE_MAP_READ, 0, 0, 0
	.if eax == NULL 
		jmp EXIT_PROC
	.endif
	mov @lpFileBuff, eax

	; 检查文件是否是PE格式
	invoke IsPeFormat, @lpFileBuff
	.if eax == FALSE
		jmp EXIT_PROC
	.endif

	; 解析dll的pe格式
	mov esi, @lpFileBuff
	assume esi : ptr IMAGE_DOS_HEADER
	add esi, [esi].e_lfanew
	assume esi : ptr IMAGE_NT_HEADERS
	mov eax, [esi].OptionalHeader.SizeOfImage
	mov @dwSizoOfImage, eax
	mov eax, [esi].OptionalHeader.AddressOfEntryPoint
	mov @dwAddressOfEntryPoint, eax

	; 申请空间，用来存放dll
	invoke VirtualAlloc, NULL, @dwSizoOfImage, MEM_COMMIT, PAGE_EXECUTE_READWRITE
	.if eax == NULL
		jmp EXIT_PROC
	.endif
	mov @lpDstAddress, eax

	; 将文件PE拉伸展开，并拷贝到加载器预留的内存中
	invoke StretchPE, @lpDstAddress, @lpFileBuff

	; 修复导入表
	invoke RepairImportTable, @lpDstAddress, @lpFileBuff

	; 修复重定位数据
	invoke RepairReloc, @lpDstAddress, @lpFileBuff

	; 调用dll的dllMain
	mov eax, @dwAddressOfEntryPoint
	add eax, @lpDstAddress
	push NULL
	push DLL_PROCESS_ATTACH
	push @lpDstAddress
	call eax

	; Exit
EXIT_PROC:
	.if @lpFileBuff != NULL 
		invoke UnmapViewOfFile, @lpFileBuff
		mov @lpFileBuff, NULL 
	.endif

	.if @hFileMap != NULL
		invoke CloseHandle, @hFileMap
		mov @hFileMap, NULL
	.endif

	.if @hFile != INVALID_HANDLE_VALUE
		invoke CloseHandle, @hFile
		mov @hFile, INVALID_HANDLE_VALUE
	.endif

	ret
LoadDll endp

start:
	invoke GetModuleHandle,NULL
	mov	hInstance,eax
	invoke LoadDll, hInstance

	invoke ExitProcess,0
end start

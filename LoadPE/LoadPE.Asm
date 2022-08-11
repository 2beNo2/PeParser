.386
.model flat, stdcall  ;32 bit memory model
option casemap :none  ;case sensitive

include LoadPE.inc

IMAGE_SIZE equ 00020000h

.const
	g_szFileName db 'winmine.exe', 0

.code

	db IMAGE_SIZE dup(0h)



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


RepairImportTable proc uses edi esi edx ebx hInst:HMODULE, lpFileBuff:LPVOID
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
	add eax, hInst
	mov @pAddressOfImport,eax

	; 遍历导入表，全0结构结尾
	mov esi, @pAddressOfImport
	assume esi : ptr IMAGE_IMPORT_DESCRIPTOR
	invoke MyMemCmp, esi, addr @ZeroImport, sizeof IMAGE_IMPORT_DESCRIPTOR
	.while eax != 0
		; 判断是否是有效导入表项
		mov edi, [esi].FirstThunk
		add edi, hInst
		.if dword ptr [edi] != NULL
			; 判断INT是否为空，INT为空时则使用IAT
			mov edx,[esi].OriginalFirstThunk
			.if edx == NULL
				mov edx,[esi].FirstThunk
			.endif
			add edx, hInst
			
			mov eax, [esi].Name1
			add eax, hInst
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
					add ebx, hInst
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


StretchPE proc uses edi esi ecx hInst:HMODULE, lpFileBuff:LPVOID
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
	invoke MyMemCopy, hInst, lpFileBuff, @dwSizeOfHeaders

	; 拷贝节区
	lea esi, [esi].OptionalHeader
	add esi, @dwSizeOfOptionalHeader
	assume esi : ptr IMAGE_SECTION_HEADER

	xor ecx, ecx
	.while ecx < @dwNumberOfSections
		; 首先确定节存在文件映射
		.if [esi].SizeOfRawData != 0
			mov edi, [esi].VirtualAddress
			add edi, hInst

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


LoadPE proc hInst:HMODULE
	LOCAL @hFile:HANDLE
	LOCAL @hFileMap:HANDLE
	LOCAL @lpFileBuff:LPVOID
	LOCAL @dwOldProtect:DWORD

	mov @hFile, INVALID_HANDLE_VALUE 
	xor eax, eax
	mov @hFileMap, eax
	mov @lpFileBuff, eax
	mov @dwOldProtect, eax

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

	; 修改加载器预留的内存空间的内存属性
	invoke VirtualProtect, hInst, IMAGE_SIZE, PAGE_EXECUTE_READWRITE, addr @dwOldProtect

	; 将文件PE拉伸展开，并拷贝到加载器预留的内存中
	invoke StretchPE, hInst, @lpFileBuff

	; 修复导入表
	invoke RepairImportTable, hInst, @lpFileBuff

	; 跳转到被加载程序的ep
	mov esi, @lpFileBuff
	assume esi : ptr IMAGE_DOS_HEADER
	add esi, [esi].e_lfanew
	assume esi : ptr IMAGE_NT_HEADERS
	mov eax, [esi].OptionalHeader.AddressOfEntryPoint
	add eax, hInst
	jmp eax


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
LoadPE endp


start:
	invoke GetModuleHandle, NULL
	mov hInstance, eax
	invoke LoadPE, hInstance

	invoke ExitProcess, 0
end start






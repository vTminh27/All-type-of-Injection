; payload.asm
section .data
    msg db 'Hello, World!', 0
    title db 'Message', 0

section .text
    global _start

_start:
    ; Push parameters for MessageBoxA
    push 0               ; uType = MB_OK
    push msg             ; lpCaption
    push title           ; lpText
    push 0               ; hWnd = NULL

    ; Get the address of MessageBoxA
    call get_messagebox_address

    ; Call MessageBoxA
    call eax

    ; Exit the process
    push 0
    call exit_process

get_messagebox_address:
    ; Load the address of MessageBoxA into EAX
    mov eax, [fs:0x30]   ; PEB
    mov eax, [eax + 0x0C] ; PEB_LDR_DATA
    mov esi, [eax + 0x1C] ; InInitializationOrderModuleList
next_module:
    lodsd                ; ESI points to the next module
    mov esi, eax
    mov edi, [esi + 0x30] ; BaseDllName
    mov eax, [edi + 0x0C] ; DllBase
    mov edi, [eax + 0x3C] ; NT_HEADERS
    add edi, eax
    mov edi, [edi + 0x78] ; Export Directory RVA
    add edi, eax
    mov ecx, [edi + 0x18] ; NumberOfNames
    mov ebx, [edi + 0x20] ; AddressOfNames RVA
    add ebx, eax
find_function:
    dec ecx
    mov esi, [ebx + ecx * 4]
    add esi, eax
    cmp dword [esi], 0x4167654D ; "MessageBoxA"
    jnz find_function
    mov ebx, [edi + 0x24] ; AddressOfNameOrdinals RVA
    add ebx, eax
    mov cx, [ebx + ecx * 2]
    mov ebx, [edi + 0x1C] ; AddressOfFunctions RVA
    add ebx, eax
    mov eax, [ebx + ecx * 4]
    add eax, [fs:0x30] ; PEB
    ret

exit_process:
    mov eax, 0x7C81CB1B ; Address of ExitProcess in kernel32.dll
    call eax
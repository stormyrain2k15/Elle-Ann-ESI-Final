EXTERN  CreateProcessA:PROC
EXTERN  OpenProcess:PROC
EXTERN  TerminateProcess:PROC
EXTERN  CloseHandle:PROC
EXTERN  WaitForSingleObject:PROC
EXTERN  CreateToolhelp32Snapshot:PROC
EXTERN  Process32FirstW:PROC
EXTERN  Process32NextW:PROC
EXTERN  Thread32First:PROC
EXTERN  Thread32Next:PROC
EXTERN  OpenThread:PROC
EXTERN  SuspendThread:PROC
EXTERN  ResumeThread:PROC
EXTERN  VirtualAllocEx:PROC
EXTERN  VirtualFreeEx:PROC
EXTERN  WriteProcessMemory:PROC
EXTERN  CreateRemoteThread:PROC
EXTERN  LoadLibraryA:PROC
EXTERN  GetModuleHandleA:PROC
EXTERN  GetProcAddress:PROC
EXTERN  QueryFullProcessImageNameA:PROC
EXTERN  K32GetProcessImageFileNameA:PROC
EXTERN  lstrlenA:PROC

.const
szKernel32          db "kernel32.dll", 0
szLoadLibraryA      db "LoadLibraryA", 0

.code

PROCESS_ALL_ACCESS         EQU 001F0FFFh
PROCESS_TERMINATE          EQU 00000001h
PROCESS_QUERY_LIMITED      EQU 00001000h
PROCESS_VM_OPERATION       EQU 00000008h
PROCESS_VM_WRITE           EQU 00000020h
PROCESS_CREATE_THREAD      EQU 00000002h
TH32CS_SNAPPROCESS         EQU 00000002h
TH32CS_SNAPTHREAD          EQU 00000004h
THREAD_SUSPEND_RESUME      EQU 00000002h
MEM_COMMIT                 EQU 1000h
MEM_RESERVE                EQU 2000h
MEM_RELEASE                EQU 8000h
PAGE_READWRITE             EQU 4
INFINITE_WAIT              EQU 0FFFFFFFFh
CREATE_NO_WINDOW           EQU 08000000h
INVALID_HANDLE_VALUE       EQU -1

ASM_LaunchProcess PROC
    push    rbp
    mov     rbp, rsp

    sub     rsp, 0E0h
    push    rbx
    push    rdi
    push    rsi

    mov     rbx, rcx
    mov     rsi, rdx

    lea     rdi, [rbp-70h]
    xor     eax, eax
    mov     ecx, 26
    rep     stosd
    mov     DWORD PTR [rbp-70h], 68h

    lea     rdi, [rbp-88h]
    xor     eax, eax
    mov     ecx, 6
    rep     stosd

    xor     rcx, rcx
    mov     rdx, rbx
    xor     r8, r8
    xor     r9, r9
    mov     DWORD PTR [rsp+20h], 0
    mov     DWORD PTR [rsp+28h], CREATE_NO_WINDOW
    mov     QWORD PTR [rsp+30h], 0
    mov     QWORD PTR [rsp+38h], 0
    lea     rax, [rbp-70h]
    mov     QWORD PTR [rsp+40h], rax
    lea     rax, [rbp-88h]
    mov     QWORD PTR [rsp+48h], rax
    call    CreateProcessA
    test    eax, eax
    jz      launch_fail

    mov     ecx, DWORD PTR [rbp-80h]
    mov     [rsi], ecx

    mov     rcx, [rbp-88h]
    call    CloseHandle
    mov     rcx, [rbp-88h+8h]
    call    CloseHandle

    mov     eax, 1
    jmp     launch_done
launch_fail:
    xor     eax, eax
launch_done:
    pop     rsi
    pop     rdi
    pop     rbx
    add     rsp, 0E0h
    leave
    ret
ASM_LaunchProcess ENDP

ASM_KillProcess PROC
    push    rbp
    mov     rbp, rsp
    sub     rsp, 30h
    push    rbx

    mov     ebx, ecx

    mov     ecx, PROCESS_TERMINATE
    xor     edx, edx
    mov     r8d, ebx
    call    OpenProcess
    test    rax, rax
    jz      kill_fail
    mov     r12, rax

    mov     rcx, r12
    xor     edx, edx
    call    TerminateProcess
    mov     r13d, eax

    mov     rcx, r12
    call    CloseHandle
    mov     eax, r13d
    jmp     kill_done
kill_fail:
    xor     eax, eax
kill_done:
    pop     rbx
    add     rsp, 30h
    leave
    ret
ASM_KillProcess ENDP

ASM_EnumProcesses PROC
    push    rbp
    mov     rbp, rsp

    sub     rsp, 0260h
    push    rbx
    push    rdi
    push    rsi
    push    r12
    push    r13
    push    r14

    mov     rdi, rcx
    mov     r12d, edx
    mov     r13, r8
    xor     r14d, r14d

    mov     ecx, TH32CS_SNAPPROCESS
    xor     edx, edx
    call    CreateToolhelp32Snapshot
    cmp     rax, INVALID_HANDLE_VALUE
    je      enum_fail
    mov     rbx, rax

    mov     DWORD PTR [rbp-240h], 568

    mov     rcx, rbx
    lea     rdx, [rbp-240h]
    call    Process32FirstW
    test    eax, eax
    jz      enum_done
enum_loop:
    cmp     r14d, r12d
    jae     enum_end_loop

    mov     ecx, DWORD PTR [rbp-240h+8]
    mov     [rdi + r14*4], ecx
    inc     r14d
    mov     rcx, rbx
    lea     rdx, [rbp-240h]
    call    Process32NextW
    test    eax, eax
    jnz     enum_loop
enum_end_loop:
    mov     [r13], r14d
    mov     rcx, rbx
    call    CloseHandle
    mov     eax, 1
    jmp     enum_exit
enum_fail:
    mov     DWORD PTR [r13], 0
    xor     eax, eax
    jmp     enum_exit
enum_done:
    mov     [r13], r14d
    mov     rcx, rbx
    call    CloseHandle
    mov     eax, 1
enum_exit:
    pop     r14
    pop     r13
    pop     r12
    pop     rsi
    pop     rdi
    pop     rbx
    add     rsp, 0260h
    leave
    ret
ASM_EnumProcesses ENDP

ASM_GetProcessName PROC
    push    rbp
    mov     rbp, rsp
    sub     rsp, 40h
    push    rbx
    push    rdi

    mov     ebx, ecx
    mov     rdi, rdx
    mov     r12d, r8d
    mov     DWORD PTR [rbp-8], r8d

    mov     ecx, PROCESS_QUERY_LIMITED
    xor     edx, edx
    mov     r8d, ebx
    call    OpenProcess
    test    rax, rax
    jz      name_fail
    mov     rbx, rax

    mov     rcx, rbx
    xor     edx, edx
    mov     r8, rdi
    lea     r9, [rbp-8]
    call    QueryFullProcessImageNameA
    mov     r13d, eax

    mov     rcx, rbx
    call    CloseHandle

    mov     eax, r13d
    jmp     name_done
name_fail:
    xor     eax, eax
name_done:
    pop     rdi
    pop     rbx
    add     rsp, 40h
    leave
    ret
ASM_GetProcessName ENDP

ASM_IsProcessRunning PROC
    push    rbp
    mov     rbp, rsp
    sub     rsp, 30h
    push    rbx

    mov     ebx, ecx
    mov     ecx, PROCESS_QUERY_LIMITED
    xor     edx, edx
    mov     r8d, ebx
    call    OpenProcess
    test    rax, rax
    jz      run_no
    mov     r12, rax
    mov     rcx, r12
    call    CloseHandle
    mov     eax, 1
    jmp     run_done
run_no:
    xor     eax, eax
run_done:
    pop     rbx
    add     rsp, 30h
    leave
    ret
ASM_IsProcessRunning ENDP

ASM_SuspendProcess PROC
    push    rbp
    mov     rbp, rsp
    sub     rsp, 40h
    push    rbx
    push    rdi
    push    r12

    mov     ebx, ecx

    mov     ecx, TH32CS_SNAPTHREAD
    xor     edx, edx
    call    CreateToolhelp32Snapshot
    cmp     rax, INVALID_HANDLE_VALUE
    je      sus_fail
    mov     r12, rax

    mov     DWORD PTR [rbp-38h], 28
    mov     rcx, r12
    lea     rdx, [rbp-38h]
    call    Thread32First
    test    eax, eax
    jz      sus_close
sus_loop:

    mov     eax, DWORD PTR [rbp-38h+12]
    cmp     eax, ebx
    jne     sus_next
    mov     ecx, THREAD_SUSPEND_RESUME
    xor     edx, edx
    mov     r8d, DWORD PTR [rbp-38h+8]
    call    OpenThread
    test    rax, rax
    jz      sus_next
    mov     rdi, rax
    mov     rcx, rdi
    call    SuspendThread
    mov     rcx, rdi
    call    CloseHandle
sus_next:
    mov     rcx, r12
    lea     rdx, [rbp-38h]
    call    Thread32Next
    test    eax, eax
    jnz     sus_loop
sus_close:
    mov     rcx, r12
    call    CloseHandle
    mov     eax, 1
    jmp     sus_done
sus_fail:
    xor     eax, eax
sus_done:
    pop     r12
    pop     rdi
    pop     rbx
    add     rsp, 40h
    leave
    ret
ASM_SuspendProcess ENDP

ASM_ResumeProcess PROC
    push    rbp
    mov     rbp, rsp
    sub     rsp, 40h
    push    rbx
    push    rdi
    push    r12

    mov     ebx, ecx
    mov     ecx, TH32CS_SNAPTHREAD
    xor     edx, edx
    call    CreateToolhelp32Snapshot
    cmp     rax, INVALID_HANDLE_VALUE
    je      res_fail
    mov     r12, rax

    mov     DWORD PTR [rbp-38h], 28
    mov     rcx, r12
    lea     rdx, [rbp-38h]
    call    Thread32First
    test    eax, eax
    jz      res_close
res_loop:
    mov     eax, DWORD PTR [rbp-38h+12]
    cmp     eax, ebx
    jne     res_next
    mov     ecx, THREAD_SUSPEND_RESUME
    xor     edx, edx
    mov     r8d, DWORD PTR [rbp-38h+8]
    call    OpenThread
    test    rax, rax
    jz      res_next
    mov     rdi, rax
    mov     rcx, rdi
    call    ResumeThread
    mov     rcx, rdi
    call    CloseHandle
res_next:
    mov     rcx, r12
    lea     rdx, [rbp-38h]
    call    Thread32Next
    test    eax, eax
    jnz     res_loop
res_close:
    mov     rcx, r12
    call    CloseHandle
    mov     eax, 1
    jmp     res_done
res_fail:
    xor     eax, eax
res_done:
    pop     r12
    pop     rdi
    pop     rbx
    add     rsp, 40h
    leave
    ret
ASM_ResumeProcess ENDP

ASM_InjectDLL PROC
    push    rbp
    mov     rbp, rsp
    sub     rsp, 60h
    push    rbx
    push    rdi
    push    rsi
    push    r12
    push    r13
    push    r14

    mov     ebx, ecx
    mov     rsi, rdx

    mov     rcx, rsi
    call    lstrlenA
    inc     eax
    mov     r14d, eax

    mov     ecx, PROCESS_ALL_ACCESS
    xor     edx, edx
    mov     r8d, ebx
    call    OpenProcess
    test    rax, rax
    jz      inj_fail
    mov     r12, rax

    mov     rcx, r12
    xor     rdx, rdx
    mov     r8d, r14d
    mov     r9d, MEM_COMMIT OR MEM_RESERVE
    mov     DWORD PTR [rsp+20h], PAGE_READWRITE
    call    VirtualAllocEx
    test    rax, rax
    jz      inj_close_proc
    mov     r13, rax

    mov     rcx, r12
    mov     rdx, r13
    mov     r8, rsi
    mov     r9d, r14d
    mov     QWORD PTR [rsp+20h], 0
    call    WriteProcessMemory
    test    eax, eax
    jz      inj_free

    lea     rcx, szKernel32
    call    GetModuleHandleA
    test    rax, rax
    jz      inj_free
    mov     rdi, rax

    mov     rcx, rdi
    lea     rdx, szLoadLibraryA
    call    GetProcAddress
    test    rax, rax
    jz      inj_free
    mov     r15, rax

    mov     rcx, r12
    xor     rdx, rdx
    xor     r8, r8
    mov     r9, r15
    mov     QWORD PTR [rsp+20h], r13
    mov     DWORD PTR [rsp+28h], 0
    mov     QWORD PTR [rsp+30h], 0
    call    CreateRemoteThread
    test    rax, rax
    jz      inj_free
    mov     rbx, rax

    mov     rcx, rbx
    mov     edx, INFINITE_WAIT
    call    WaitForSingleObject
    mov     rcx, rbx
    call    CloseHandle

    mov     rcx, r12
    mov     rdx, r13
    xor     r8, r8
    mov     r9d, MEM_RELEASE
    call    VirtualFreeEx
    mov     rcx, r12
    call    CloseHandle

    mov     eax, 1
    jmp     inj_done

inj_free:
    mov     rcx, r12
    mov     rdx, r13
    xor     r8, r8
    mov     r9d, MEM_RELEASE
    call    VirtualFreeEx
inj_close_proc:
    mov     rcx, r12
    call    CloseHandle
inj_fail:
    xor     eax, eax
inj_done:
    pop     r14
    pop     r13
    pop     r12
    pop     rsi
    pop     rdi
    pop     rbx
    add     rsp, 60h
    leave
    ret
ASM_InjectDLL ENDP

END

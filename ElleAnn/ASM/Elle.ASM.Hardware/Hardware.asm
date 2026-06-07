EXTERN  GetCurrentProcess:PROC
EXTERN  SetProcessAffinityMask:PROC
EXTERN  GlobalMemoryStatusEx:PROC
EXTERN  GetSystemInfo:PROC
EXTERN  OpenProcess:PROC
EXTERN  SetPriorityClass:PROC
EXTERN  CloseHandle:PROC
EXTERN  QueryPerformanceFrequency:PROC
EXTERN  QueryPerformanceCounter:PROC
EXTERN  GetSystemPowerStatus:PROC
EXTERN  GetTickCount64:PROC

PROCESS_SET_INFORMATION  EQU  0200h

.data

g_lastIdleTime     QWORD  0
g_lastKernelTime   QWORD  0
g_lastUserTime     QWORD  0
g_lastSampleTick   QWORD  0

.code

ASM_SetCPUAffinity PROC
    push    rbp
    mov     rbp, rsp
    sub     rsp, 30h
    mov     [rbp-8], rcx

    call    GetCurrentProcess
    mov     rcx, rax
    mov     rdx, [rbp-8]
    call    SetProcessAffinityMask

    add     rsp, 30h
    leave
    ret
ASM_SetCPUAffinity ENDP

ASM_GetCPUUsage PROC
    push    rbp
    mov     rbp, rsp
    sub     rsp, 20h
    push    rbx
    push    rdi

    mov     rdi, rcx

    rdtsc
    shl     rdx, 32
    or      rax, rdx
    mov     rbx, rax

    call    GetTickCount64
    mov     [rbp-16], rax

    mov     ecx, 1000000
cpu_loop:
    dec     ecx
    jnz     cpu_loop

    rdtsc
    shl     rdx, 32
    or      rax, rdx
    sub     rax, rbx

    mov     r10, rax
    call    GetTickCount64
    sub     rax, [rbp-16]

    mov     rdx, r10
    shr     rdx, 20
    test    rax, rax
    jnz     cpu_dodiv
    mov     eax, 50
    jmp     cpu_store
cpu_dodiv:
    xchg    rax, rdx
    xor     rdx, rdx

    mov     rcx, 1
    mov     rdx, 0

cpu_store:
    cmp     eax, 100
    jbe     cpu_cap
    mov     eax, 100
cpu_cap:
    mov     [rdi], eax

    mov     eax, 1
    pop     rdi
    pop     rbx
    add     rsp, 20h
    leave
    ret
ASM_GetCPUUsage ENDP

ASM_GetMemoryInfo PROC
    push    rbp
    mov     rbp, rsp

    sub     rsp, 80h
    push    rdi
    push    rsi
    mov     rdi, rcx
    mov     rsi, rdx

    lea     rcx, [rbp-50h]
    mov     DWORD PTR [rcx], 64
    mov     DWORD PTR [rcx+4], 0
    mov     QWORD PTR [rcx+8],  0
    mov     QWORD PTR [rcx+16], 0
    mov     QWORD PTR [rcx+24], 0
    mov     QWORD PTR [rcx+32], 0
    mov     QWORD PTR [rcx+40], 0
    mov     QWORD PTR [rcx+48], 0
    mov     QWORD PTR [rcx+56], 0

    call    GlobalMemoryStatusEx
    test    eax, eax
    jz      mem_fail

    lea     rax, [rbp-50h]
    mov     rcx, [rax+8]
    mov     [rdi], rcx
    mov     rcx, [rax+16]
    mov     [rsi], rcx

    mov     eax, 1
    jmp     mem_done
mem_fail:
    xor     eax, eax
mem_done:
    pop     rsi
    pop     rdi
    add     rsp, 80h
    leave
    ret
ASM_GetMemoryInfo ENDP

ASM_SetProcessPriority PROC
    push    rbp
    mov     rbp, rsp
    sub     rsp, 30h
    push    rbx
    mov     ebx, edx
    mov     r12d, ecx

    mov     ecx, PROCESS_SET_INFORMATION
    xor     edx, edx
    mov     r8d, r12d
    call    OpenProcess
    test    rax, rax
    jz      pri_fail
    mov     rdi, rax

    mov     rcx, rdi
    mov     edx, ebx
    call    SetPriorityClass
    mov     r14d, eax

    mov     rcx, rdi
    call    CloseHandle

    mov     eax, r14d
    jmp     pri_done
pri_fail:
    xor     eax, eax
pri_done:
    pop     rbx
    add     rsp, 30h
    leave
    ret
ASM_SetProcessPriority ENDP

ASM_QueryPerfCounters PROC
    push    rbp
    mov     rbp, rsp
    sub     rsp, 30h
    push    rbx
    push    rdi

    mov     rbx, rcx
    mov     rdi, rdx

    mov     rcx, rbx
    call    QueryPerformanceFrequency

    mov     rcx, rdi
    call    QueryPerformanceCounter

    mov     eax, 1
    pop     rdi
    pop     rbx
    add     rsp, 30h
    leave
    ret
ASM_QueryPerfCounters ENDP

ASM_GetPowerStatus PROC
    push    rbp
    mov     rbp, rsp
    sub     rsp, 30h
    push    rdi
    push    rsi
    mov     rdi, rcx
    mov     rsi, rdx

    lea     rcx, [rbp-10h]
    call    GetSystemPowerStatus
    test    eax, eax
    jz      pwr_fail

    movzx   ecx, BYTE PTR [rbp-10h+2]
    mov     [rdi], ecx

    movzx   ecx, BYTE PTR [rbp-10h]
    cmp     ecx, 1
    jne     pwr_notcharging
    mov     DWORD PTR [rsi], 1
    jmp     pwr_ok
pwr_notcharging:
    mov     DWORD PTR [rsi], 0
pwr_ok:
    mov     eax, 1
    jmp     pwr_done
pwr_fail:
    xor     eax, eax
pwr_done:
    pop     rsi
    pop     rdi
    add     rsp, 30h
    leave
    ret
ASM_GetPowerStatus ENDP

ASM_CPUID PROC
    push    rbp
    mov     rbp, rsp
    push    rbx
    push    rsi
    push    rdi

    mov     rsi, rdx
    mov     rdi, r8
    mov     r10, r9
    mov     r11, [rbp+30h]

    mov     eax, ecx
    xor     ecx, ecx
    cpuid

    mov     [rsi], eax
    mov     [rdi], ebx
    mov     [r10], ecx
    mov     [r11], edx

    mov     eax, 1
    pop     rdi
    pop     rsi
    pop     rbx
    leave
    ret
ASM_CPUID ENDP

ASM_RDTSC PROC
    rdtsc
    shl     rdx, 32
    or      rax, rdx
    mov     [rcx], rax
    mov     eax, 1
    ret
ASM_RDTSC ENDP

END

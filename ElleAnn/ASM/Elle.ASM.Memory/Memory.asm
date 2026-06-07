.data
    ALIGN 16
    pool_base   dq 0
    pool_size   dq 0
    pool_offset dq 0
    pool_lock   dq 0

.code

SpinLock PROC
    push    rax
@@retry:
    xor     eax, eax
    lock cmpxchg QWORD PTR [pool_lock], rcx
    jnz     @@retry
    pop     rax
    ret
SpinLock ENDP

SpinUnlock PROC
    mov     QWORD PTR [pool_lock], 0
    mfence
    ret
SpinUnlock ENDP

ASM_PoolAlloc PROC
    push    rbp
    mov     rbp, rsp
    push    rbx

    add     ecx, 15
    and     ecx, 0FFFFFFF0h
    mov     ebx, ecx

    mov     rcx, 1
    call    SpinLock

    mov     rax, [pool_base]
    test    rax, rax
    jz      @@init_pool

@@alloc:

    mov     rax, [pool_offset]
    add     rax, rbx
    cmp     rax, [pool_size]
    ja      @@oom

    mov     rax, [pool_base]
    add     rax, [pool_offset]
    add     [pool_offset], rbx

    call    SpinUnlock
    pop     rbx
    leave
    ret

@@init_pool:

    mov     QWORD PTR [pool_size], 67108864
    mov     QWORD PTR [pool_offset], 0

    jmp     @@alloc

@@oom:
    call    SpinUnlock
    xor     eax, eax
    pop     rbx
    leave
    ret
ASM_PoolAlloc ENDP

ASM_PoolFree PROC

    ret
ASM_PoolFree ENDP

ASM_MapFile PROC
    push    rbp
    mov     rbp, rsp
    sub     rsp, 40h

    mov     eax, 1
    leave
    ret
ASM_MapFile ENDP

ASM_UnmapFile PROC
    push    rbp
    mov     rbp, rsp
    sub     rsp, 20h

    mov     eax, 1
    leave
    ret
ASM_UnmapFile ENDP

ASM_FastMemCopy PROC
    push    rbp
    mov     rbp, rsp

    cmp     r8d, 64
    jb      @@small

    mov     eax, r8d
    shr     eax, 6

    ALIGN 16
@@sse_loop:
    movdqu  xmm0, XMMWORD PTR [rdx]
    movdqu  xmm1, XMMWORD PTR [rdx+16]
    movdqu  xmm2, XMMWORD PTR [rdx+32]
    movdqu  xmm3, XMMWORD PTR [rdx+48]
    movdqu  XMMWORD PTR [rcx], xmm0
    movdqu  XMMWORD PTR [rcx+16], xmm1
    movdqu  XMMWORD PTR [rcx+32], xmm2
    movdqu  XMMWORD PTR [rcx+48], xmm3
    add     rdx, 64
    add     rcx, 64
    dec     eax
    jnz     @@sse_loop

    and     r8d, 63
    jz      @@done

@@small:

    mov     al, [rdx]
    mov     [rcx], al
    inc     rdx
    inc     rcx
    dec     r8d
    jnz     @@small

@@done:
    leave
    ret
ASM_FastMemCopy ENDP

ASM_FastMemSet PROC
    push    rbp
    mov     rbp, rsp

    movzx   eax, dl
    imul    eax, 01010101h
    movd    xmm0, eax
    pshufd  xmm0, xmm0, 0

    cmp     r8d, 64
    jb      @@small

    mov     eax, r8d
    shr     eax, 6

    ALIGN 16
@@sse_loop:
    movdqu  XMMWORD PTR [rcx], xmm0
    movdqu  XMMWORD PTR [rcx+16], xmm0
    movdqu  XMMWORD PTR [rcx+32], xmm0
    movdqu  XMMWORD PTR [rcx+48], xmm0
    add     rcx, 64
    dec     eax
    jnz     @@sse_loop

    and     r8d, 63
    jz      @@done

@@small:
    mov     [rcx], dl
    inc     rcx
    dec     r8d
    jnz     @@small

@@done:
    leave
    ret
ASM_FastMemSet ENDP

ASM_MemCompare PROC
    push    rbp
    mov     rbp, rsp

    cmp     r8d, 16
    jb      @@byte_cmp

    mov     eax, r8d
    shr     eax, 4

    ALIGN 16
@@sse_cmp:
    movdqu  xmm0, XMMWORD PTR [rcx]
    movdqu  xmm1, XMMWORD PTR [rdx]
    pcmpeqb xmm0, xmm1
    pmovmskb eax, xmm0
    cmp     eax, 0FFFFh
    jne     @@diff
    add     rcx, 16
    add     rdx, 16
    sub     r8d, 16
    cmp     r8d, 16
    jae     @@sse_cmp

@@byte_cmp:
    test    r8d, r8d
    jz      @@equal
    mov     al, [rcx]
    cmp     al, [rdx]
    jne     @@diff
    inc     rcx
    inc     rdx
    dec     r8d
    jnz     @@byte_cmp

@@equal:
    xor     eax, eax
    leave
    ret

@@diff:
    mov     eax, 1
    leave
    ret
ASM_MemCompare ENDP

END

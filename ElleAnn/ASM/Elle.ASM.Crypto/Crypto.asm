.data

    ALIGN 16

    sha256_k DWORD 0428a2f98h, 071374491h, 0b5c0fbcfh, 0e9b5dba5h
             DWORD 03956c25bh, 059f111f1h, 0923f82a4h, 0ab1c5ed5h
             DWORD 0d807aa98h, 012835b01h, 0243185beh, 0550c7dc3h
             DWORD 072be5d74h, 080deb1feh, 09bdc06a7h, 0c19bf174h
             DWORD 0e49b69c1h, 0efbe4786h, 00fc19dc6h, 0240ca1cch
             DWORD 02de92c6fh, 04a7484aah, 05cb0a9dch, 076f988dah
             DWORD 0983e5152h, 0a831c66dh, 0b00327c8h, 0bf597fc7h
             DWORD 0c6e00bf3h, 0d5a79147h, 006ca6351h, 014292967h
             DWORD 027b70a85h, 02e1b2138h, 04d2c6dfch, 053380d13h
             DWORD 0650a7354h, 0766a0abbh, 081c2c92eh, 092722c85h
             DWORD 0a2bfe8a1h, 0a81a664bh, 0c24b8b70h, 0c76c51a3h
             DWORD 0d192e819h, 0d6990624h, 0f40e3585h, 0106aa070h
             DWORD 019a4c116h, 01e376c08h, 02748774ch, 034b0bcb5h
             DWORD 0391c0cb3h, 04ed8aa4ah, 05b9cca4fh, 0682e6ff3h
             DWORD 0748f82eeh, 078a5636fh, 084c87814h, 08cc70208h
             DWORD 090befffah, 0a4506cebh, 0bef9a3f7h, 0c67178f2h

    sha256_h0 DWORD 06a09e667h
    sha256_h1 DWORD 0bb67ae85h
    sha256_h2 DWORD 03c6ef372h
    sha256_h3 DWORD 0a54ff53ah
    sha256_h4 DWORD 0510e527fh
    sha256_h5 DWORD 09b05688ch
    sha256_h6 DWORD 01f83d9abh
    sha256_h7 DWORD 05be0cd19h

    ALIGN 16
    crc32_table DWORD 256 DUP (0)
    crc32_init  DWORD 0

.code

ASM_SHA256_SCAFFOLDED PROC
    push    rbp
    mov     rbp, rsp

    mov     rdi, r8
    mov     eax, 0DEAD5CAFh
    mov     ecx, 8
@@fill:
    mov     [rdi], eax
    add     rdi, 4
    dec     ecx
    jnz     @@fill
    leave
    ret
ASM_SHA256_SCAFFOLDED ENDP

ASM_AES256Encrypt_SCAFFOLDED PROC
    push    rbp
    mov     rbp, rsp
    sub     rsp, 40h
    mov     rdi, r9
    mov     r10d, DWORD PTR [rbp+30h]
    test    r10d, r10d
    jz      @@done
    mov     eax, 0DEAD5CAFh
@@fill:
    mov     [rdi], al
    inc     rdi
    dec     r10d
    jnz     @@fill
@@done:
    leave
    ret
ASM_AES256Encrypt_SCAFFOLDED ENDP

ASM_AES256Decrypt_SCAFFOLDED PROC
    push    rbp
    mov     rbp, rsp
    sub     rsp, 40h
    mov     rdi, r9
    mov     r10d, DWORD PTR [rbp+30h]
    test    r10d, r10d
    jz      @@done
    mov     eax, 0DEAD5CAFh
@@fill:
    mov     [rdi], al
    inc     rdi
    dec     r10d
    jnz     @@fill
@@done:
    leave
    ret
ASM_AES256Decrypt_SCAFFOLDED ENDP

ASM_XorCipher PROC
    push    rbp
    mov     rbp, rsp
    push    rbx
    push    rdi
    push    rsi

    mov     rsi, rcx
    mov     rdi, rdx
    mov     ebx, r8d
    mov     rcx, r9
    mov     edx, DWORD PTR [rbp+30h]

    xor     eax, eax

    ALIGN 16
@@xor_loop:
    test    ebx, ebx
    jz      @@done

    movzx   r8d, BYTE PTR [rsi]

    push    rax
    xor     edx, edx
    div     DWORD PTR [rbp+30h]
    movzx   r9d, BYTE PTR [rcx+rdx]
    pop     rax

    xor     r8d, r9d
    mov     [rdi], r8b

    inc     rsi
    inc     rdi
    inc     eax
    dec     ebx
    jmp     @@xor_loop

@@done:
    pop     rsi
    pop     rdi
    pop     rbx
    leave
    ret
ASM_XorCipher ENDP

ASM_CRC32 PROC
    push    rbp
    mov     rbp, rsp

    mov     eax, 0FFFFFFFFh

    ALIGN 16
@@crc_loop:
    test    edx, edx
    jz      @@finish

    cmp     edx, 8
    jb      @@byte_crc

    crc32   rax, QWORD PTR [rcx]
    add     rcx, 8
    sub     edx, 8
    jmp     @@crc_loop

@@byte_crc:
    crc32   eax, BYTE PTR [rcx]
    inc     rcx
    dec     edx
    jnz     @@byte_crc

@@finish:
    not     eax

    leave
    ret
ASM_CRC32 ENDP

ASM_RandomBytes PROC
    push    rbp
    mov     rbp, rsp
    push    rbx
    push    rdi

    mov     rdi, rcx
    mov     ebx, edx

    ALIGN 16
@@rng_loop:
    cmp     ebx, 8
    jb      @@rng_small

    rdrand  rax
    jnc     @@rng_loop
    mov     [rdi], rax
    add     rdi, 8
    sub     ebx, 8
    jmp     @@rng_loop

@@rng_small:
    test    ebx, ebx
    jz      @@rng_done
    rdrand  rax
    jnc     @@rng_small
@@rng_byte:
    mov     [rdi], al
    shr     rax, 8
    inc     rdi
    dec     ebx
    jnz     @@rng_byte

@@rng_done:
    pop     rdi
    pop     rbx
    leave
    ret
ASM_RandomBytes ENDP

END

%include "sseutils64.nasm"
section .data
    ;const   dd 1000.0
 section .bss			; Sezione contenente dati non inizializzati
 	alignb 32
 	;fl	resd 1
    ;output resd 1
    ;score resq 1
    labels resq 1
    k resq 1
    num_features resq 1
    merito_corrente resq 1
    N resq 1
    dataset resq 1
    ou resq 1
    ;a resd 1
    b resq 1
    c resq 1
    d resq 1
    ed resq 1
    ;k10 resq 1
    ;k11 resq 1
    ;k12 resq 1
    ;k13 resq 1
    ;k14 resq 1
    ;k15 resq 1
    ;c resq 1
    ;d resd 1
    ;ed resd 1
    i resq 1
    m0 resq 1
    m1 resq 1

section .text
extern calcola_merito

%macro	merito	0
    pushaq
	mov RDI, [dataset]
    mov RSI, [ou]
    mov RDX, [i]
    mov RCX, [labels]
    mov R8, [N]
    mov R9, [num_features]
    ; Controlla l'allineamento corrente di rsp
    mov rax, rsp
    and rax, 0xF ; Maschera con 0xF (15 in decimale) per ottenere i primi 4 bit
    cmp rax, 0   ; Verifica se i primi 4 bit sono 0
    je  allineato ; Salta se già allineato

    ; Se non è allineato, regola rsp per garantire l'allineamento
    sub rsp, rax

allineato:
	call calcola_merito
	vmovsd [merito_corrente], xmm0
    popaq
%endmacro

global calcola_cfs8
; void calcola_cfs(float* ds, float* c, int k, int num_features, int N, int* out, float* score)
calcola_cfs8:
;RDI contiene DS, era in EAX.
;RSI contiene C (le labels)
;RDX contiene k
;RCX contiene num_features
;R8 contiene N
;R9 contiene out (corrisponde con l'insieme S), era EDX
; double calcola_cfs(double* ds, double* c, int k, int num_features, int N, int* out)
    push rbp; salva il Base Pointer
	mov rbp, rsp; il Base Pointer punta al Record di Attivazione corrente

    mov [dataset], RDI; indirizzo del dataset
    mov [labels], RCX; indirizzo delle labels (c)
    mov [k], RDX; k
    mov [num_features], RCX; numero colonne
    mov [N], R8
    mov [ou], R9; out/s
    
    mov RSI, 0; current_size=0
    ;eax ds
    ;ebx libero
    ;edi k
    ;ecx N
    ;edx S/out
    mov RDI, [k]
    mov RCX, [N]
    mov RDX, [ou]

    mov [merito_corrente], RSI


_while:
    cmp RSI, [k]
    jge _end
    mov RBX, -1; max_merit_feature_index = -1
    vxorpd xmm0, xmm0; max_merit=0.0
    mov RSI, 0
    _for:
        cmp RDI, [num_features]
        jge _aggiorna_while
        mov RCX, 0; indice
        _for_interno:
            cmp RDI, [RDX+RCX*4]
            je _true
            inc RCX
            cmp RCX, RSI
            jge _continua
            jmp _for_interno
        _true:
            inc RDI
            jmp _for
    _continua:
        mov [RDX+RSI*4], RDI; s[current_size]=i
        ;calcolo merito corrente:
        inc RSI
        ;movss [fl], xmm0
        ;printss fl
        ;mov [a], eax
        mov [b], RBX
        mov [c], ecx
        mov [d], edx
        mov [ed], edi
        mov [i], esi
        vmovsd [m0], xmm0
        merito
        ;mov eax, [a]
        mov RBX, [b]
        mov RCX, [c]
        mov RDX, [d]
        mov RDI, [ed]
        mov RSI, [i]
        vmovsd xmm0, [m0]
        sub RSI, 1
        vmovsd xmm1, [merito_corrente]
        ;movss [fl], xmm0
        ;printss fl
        ;printss merito_corrente
        ucomisd xmm1, xmm0
        jbe _skippa
        ;CVTSI2SS xmm2, ebx
        ;movss [fl], xmm2
        ;printss fl
        ;aggiorno il massimo
        movsd xmm0, xmm1
        mov RBX, RDI
    _skippa:
        inc RDI
        jmp _for
_aggiorna_while:
    mov [RDX+RSI*4], RBX; s[current_size] = max_merit_feature_index
    inc RSI
    jmp _while

_end:
    mov rsp, rbp; ripristina lo Stack Pointer
	pop rbp; ripristina il Base Pointer
	ret; torna alla funzione C chiamante
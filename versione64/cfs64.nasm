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
    sir resq 1
    ;k10 resq 1
    ;k11 resq 1
    ;k12 resq 1
    ;k13 resq 1
    ;k14 resq 1
    ;k15 resq 1
    c resq 1
    ;d resd 1
    ;ed resd 1
    i resq 1
    m0 resq 1
    m1 resq 1

section .text
extern calcola_merito
;double calcola_merito(double* dataset, int* selected_features, int num_chosen_features,double* labels, int N, int d){
;RDI = dataset
;RSI = selected_features
;RDX = num_chosen_features
;RCX = labels
;R8 = n
;R9 = d
;MERITO CALCOLATO IN XMM0
%macro	merito	0
    ;pushaq
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
    ;popaq
%endmacro

global calcola_cfs3
; double calcola_cfs(double* ds, double* c, int k, int num_features, int N, int* out)
calcola_cfs3:
    ; ------------------------------------------------------------
	; Sequenza di ingresso nella funzione
	; ------------------------------------------------------------
	push rbp; salva il Base Pointer
	mov rbp, rsp; il Base Pointer punta al Record di Attivazione corrente

    ;RDI contiene DS, era in EAX.
    ;RSI contiene C (le labels)
    ;RDX contiene k
    ;RCX contiene num_features
    ;R8 contiene N
    ;R9 contiene out (corrisponde con l'insieme S), era EDX
    mov [labels], RSI
    mov [k], RDX
    mov [num_features], RCX
    mov [N], R8
    mov [dataset], RDI
    mov [ou], R9
    
    xor RAX, RAX; current_size=0
    ;RAX = vecchio esi. indice.
    ;mov [merito_corrente], esi


_while:
    cmp RAX, [k];vecchio esi
    jge _end
    mov RBX, -1; max_merit_feature_index = -1
    ;RBX è il vecchio EBX.
    VCVTSI2SD xmm0, rbx; max_merit=-1.0
    ;vxorpd ymm0, ymm0; max_merit=0.0
    vxorpd ymm1, ymm1
    xor RSI, RSI
    ;RSI è il vecchio edi
    _for:
        cmp RSI, [num_features]
        jge _aggiorna_while
        xor RCX, RCX; indice, vecchio ECX
        _for_interno:
            cmp RCX, RAX
            jge _continua
            cmp RSI, [R9+RCX*4]
            je _true
            inc RCX            
            jmp _for_interno
        _true:
            inc RSI
            jmp _for
    _continua:
        mov [R9+RAX*4], RSI; s[current_size]=i
        ;calcolo merito corrente:
        
;calcola_merito(double* dataset, int* selected_features, int num_chosen_features,double* labels, int N, int d){
        inc RAX
        mov [i], RAX
        mov [b], RBX
        mov [sir], RSI
        ;mov [dataset], RDI
        mov [N], R8
        mov [ou], R9
        ;vmovsd [m1], xmm1
        vmovsd [m0], xmm0
        ;pushaq
        merito
        mov RAX, qword [i]
        ;popaq
        sub RAX, 1
        mov RBX, qword [b]
        mov RSI, qword [sir]
        ;mov RDI, qword [dataset]
        mov R8, qword [N]
        mov r9, qword [ou]

        vmovsd xmm1, [merito_corrente]
        vmovsd xmm0, [m0]
        vucomisd xmm1, xmm0
        jbe _skippa
        vmovsd xmm0, xmm1
        ;vmovsd [m0], xmm0
        mov RBX, RSI
    _skippa:
        inc RSI
        jmp _for
_aggiorna_while:
    mov [R9+RAX*4], RBX; s[current_size] = max_merit_feature_index
    inc RAX
    jmp _while

_end:
    ;vmovsd xmm0, [m0]
    
    mov rsp, rbp; ripristina lo Stack Pointer
	pop rbp; ripristina il Base Pointer
	ret; torna alla funzione C chiamante

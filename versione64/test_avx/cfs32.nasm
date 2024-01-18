%include "sseutils64.nasm"
;%include "final_cfs32c.o"
section .data
    ;const   dd 1000.0
 section .bss			; Sezione contenente dati non inizializzati
 	alignb 32
 	;fl	resd 1
    ;output resd 1
    score resd 1
    labels resd 1
    k resd 1
    num_features resd 1
    merito_corrente resq 1
    N resd 1
    a resd 1
    b resd 1
    c resd 1
    d resd 1
    ed resd 1
    i resd 1
    m0 resq 1

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
%macro	merito	6
    pushaq
	mov RDI, %1
    mov RSI, %2
    mov RDX, %3
    mov RCX, %4
    mov R8, %5
    mov R9, %6
	call calcola_merito
	vmovsd [merito_corrente], xmm0
    popaq
%endmacro

global calcola_cfs
; void calcola_cfs(double* ds, double* c, int k, int num_features, int N, int* out, double* score)
calcola_cfs:
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
    ;sullo stack è presente score.
    pop R11;sposto score.
    mov [score], R11
    mov [labels], RSI
    mov [k], RDX
    mov [num_features], RCX
    mov [N], R8
    
    xor RAX, RAX; current_size=0
    ;RAX = vecchio esi. indice.
    ;mov [merito_corrente], esi


_while:
    cmp RAX, [k];vecchio esi
    jge _end
    mov RBX, -1; max_merit_feature_index = -1
    ;RBX è il vecchio EBX.
    vxorpd ymm0, ymm0; max_merit=0.0
    xor RSI, RSI
    ;RSI è il vecchio edi
    _for:
        cmp RSI, [num_features]
        jge _aggiorna_while
        xor RCX, RCX; indice, vecchio ECX
        _for_interno:
            cmp RSI, [R9+RCX*4]
            je _true
            inc RCX
            cmp RCX, RAX
            jge _continua
            jmp _for_interno
        _true:
            inc RSI
            jmp _for
    _continua:
        mov [R9+RAX*4], RSI; s[current_size]=i
        ;calcolo merito corrente:
        inc RAX
;calcola_merito(double* dataset, int* selected_features, int num_chosen_features,double* labels, int N, int d){

        merito RDI, R9, RAX, [labels], [N], [num_features]
        sub RAX, 1
        vmovsd xmm0, [merito_corrente]
        vucomisd xmm1, xmm0
        jbe _skippa
        vmovsd xmm0, xmm1
        mov RBX, RSI
    _skippa:
        inc RSI
        jmp _for
_aggiorna_while:
    mov [R9+RAX*4], RBX; s[current_size] = max_merit_feature_index
    inc RAX
    jmp _while

_end:
    mov RAX, [score]
    vmovsd [RAX], xmm0
    
    mov rsp, rbp; ripristina lo Stack Pointer
	pop rbp; ripristina il Base Pointer
	ret; torna alla funzione C chiamante

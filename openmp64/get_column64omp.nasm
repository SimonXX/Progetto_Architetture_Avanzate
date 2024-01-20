%include "sseutils64.nasm"
section .data
section .text
;METODO GET_COLUMN IMPLEMENTATO IN ASSEMBLY MEDIANTE TECNICA DEL LOOP UNROLLING
;FATTORE DI UNROLL PARI A 16
global get_column
get_column:
    ; ------------------------------------------------------------
	; Sequenza di ingresso nella funzione
	; ------------------------------------------------------------
	push rbp; salva il Base Pointer
	mov rbp, rsp; il Base Pointer punta al Record di Attivazione corrente

    ;void get_column(double* risultato, double* dataset, int rows, int columns, int choosed_col);

    ;RDI contiene l'indirizzo del risultato da sovrascrivere
    ;RSI contiene l'indirizzo del dataset
    ;RDX contiene il numero di righe
    ;RCX contiene il numero di colonne
    ;R8 contiene l'indice di colonna scelto
    mov R9, RDI
    ;R9 CONTIENE ORA L'INDIRIZZO DEL RISULTATO
    mov RBX, RCX
    mov RAX, RDX
    XOR RDX, RDX; pulisco la parte alta del dividendo
    ;mov ebx, [ebp+12]; indirizzo del dataset
    ;mov eax, [ebp+16]; numero di righe
    ;mov edi, [ebp+24]; colonna scelta
    mov RDI, 16
    idiv RDI
    push RDX; pusho il resto della divisione intera. Il quoziente si trova in rax.
    mov RDI, 0
    imul RAX, 16
    cmp RAX, 0
    je _prepare_loop_rest
    vxorpd ymm0, ymm0
_loop:
    cmp RDI, RAX
    jge _continue
    ;mov RCX, RBX
    imul RCX, RDI
    add RCX, R8;offset/8

;1
    vmovsd xmm0, [RSI + RCX*8];leggo dataset[i*cols+col_index]
    vmovsd [R9+RDI*8], xmm0;lo copio in column[i]
;2
    inc RDI
    mov RCX, RBX
    imul RCX, RDI
    add RCX, R8
    vmovsd xmm0, [RSI+RCX*8]
    vmovsd [R9+RDI*8], xmm0
;3
    inc RDI
    mov RCX, RBX
    imul RCX, RDI
    add RCX, R8
    vmovsd xmm0, [RSI+RCX*8]
    vmovsd [R9+RDI*8], xmm0
;4
    inc RDI
    mov RCX, RBX
    imul RCX, RDI
    add RCX, R8
    vmovsd xmm0, [RSI+RCX*8]
    vmovsd [R9+RDI*8], xmm0
;5
    inc RDI
    mov RCX, RBX
    imul RCX, RDI
    add RCX, R8
    vmovsd xmm0, [RSI+RCX*8]
    vmovsd [R9+RDI*8], xmm0
;6
    inc RDI
    mov RCX, RBX
    imul RCX, RDI
    add RCX, R8
    vmovsd xmm0, [RSI+RCX*8]
    vmovsd [R9+RDI*8], xmm0
;7
    inc RDI
    mov RCX, RBX
    imul RCX, RDI
    add RCX, R8
    vmovsd xmm0, [RSI+RCX*8]
    vmovsd [R9+RDI*8], xmm0
;8

    inc RDI
    mov RCX, RBX
    imul RCX, RDI
    add RCX, R8
    vmovsd xmm0, [RSI+RCX*8]
    vmovsd [R9+RDI*8], xmm0
;9
    inc RDI
    mov RCX, RBX
    imul RCX, RDI
    add RCX, R8
    vmovsd xmm0, [RSI+RCX*8]
    vmovsd [R9+RDI*8], xmm0
;10
    inc RDI
    mov RCX, RBX
    imul RCX, RDI
    add RCX, R8
    vmovsd xmm0, [RSI+RCX*8]
    vmovsd [R9+RDI*8], xmm0
;11
    inc RDI
    mov RCX, RBX
    imul RCX, RDI
    add RCX, R8
    vmovsd xmm0, [RSI+RCX*8]
    vmovsd [R9+RDI*8], xmm0
;12
    inc RDI
    mov RCX, RBX
    imul RCX, RDI
    add RCX, R8
    vmovsd xmm0, [RSI+RCX*8]
    vmovsd [R9+RDI*8], xmm0
;13
    inc RDI
    mov RCX, RBX
    imul RCX, RDI
    add RCX, R8
    vmovsd xmm0, [RSI+RCX*8]
    vmovsd [R9+RDI*8], xmm0
;14
    inc RDI
    mov RCX, RBX
    imul RCX, RDI
    add RCX, R8
    vmovsd xmm0, [RSI+RCX*8]
    vmovsd [R9+RDI*8], xmm0
;15
    inc RDI
    mov RCX, RBX
    imul RCX, RDI
    add RCX, R8
    vmovsd xmm0, [RSI+RCX*8]
    vmovsd [R9+RDI*8], xmm0
;16
    inc RDI
    mov RCX, RBX
    imul RCX, RDI
    add RCX, R8
    vmovsd xmm0, [RSI+RCX*8]
    vmovsd [R9+RDI*8], xmm0

    inc RDI
    mov RCX, RBX


    jmp _loop

_continue:
    imul RCX, RDI
    imul RCX, 8

    add RSI, RCX; aggiorno il puntatore al dataset. ora punta al prossimo elemento da leggere/estrarre

    imul RDI, 8
    add R9, RDI; aggiorno il puntatore al risultato. ora punta alla prossima posizione in cui scrivere.

    mov RCX, RBX
    mov RDI, 0; azzero l'indice in vista del prossimo loop.
_prepare_loop_rest:
    pop RAX; inserisco il resto della divisione in eax.
_loop_rest:
    cmp RDI, RAX
    jge _end
    imul RCX, RDI
    add RCX, R8
    vmovsd xmm0, [RSI+RCX*8]
    vmovsd [R9+RDI*8], xmm0
    mov RCX, RBX
    inc RDI
    jmp _loop_rest


_end:
    mov rsp, rbp; ripristina lo Stack Pointer
	pop rbp; ripristina il Base Pointer
	ret; torna alla funzione C chiamante

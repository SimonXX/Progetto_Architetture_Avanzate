;%include "sseutils64.nasm"
section .data
    ;const   dd 1000.0
;section .bss			; Sezione contenente dati non inizializzati
; 	alignb 32
; 	prova		resq		1
section .text
    ;void calcola_dev_std(double media, double* data, int n);
global calcola_dev_std
calcola_dev_std:
    ; ------------------------------------------------------------
	; Sequenza di ingresso nella funzione
	; ------------------------------------------------------------
    push rbp; salva il Base Pointer
	mov rbp, rsp; il Base Pointer punta al Record di Attivazione corrente
	;pushaq; salva i registri generali

    ;RDI contiene l'indirizzo a data
    ;RSI contiene il valore di n
    ;XMM0 contiene il valore di media
    ;VMOVSD [media], xmm0
    VBROADCASTSD ymm0, xmm0; ricopio il valore della media in ogni celletta
    mov rdx, rsi; copio n
    sub rdx, 1; salvo n-1
    VXORPD ymm1, ymm1, ymm1; varianza=0.0
;ecx era n, ora è rsi
;ebx era l'indirizzo di data, ora RDI
    cmp rsi, 4
    jl _rest_loop

_loop:
    VMOVAPD ymm2, [rdi]
    vsubpd ymm2, ymm0; xi-mean
    vmulpd ymm2, ymm2; elevo al quadrato
    vaddpd ymm1, ymm1, ymm2; aggiorno la somma
    add rdi, 32; aggiorno l'indirizzo, 4 double più avanti (8 byte per ogni double)
    sub rsi, 4; aggiorno la dimensione 
    cmp rsi, 3
    jg _loop
    vhaddpd ymm1, ymm1, ymm1
    vextractf128 xmm4, ymm1, 0; parte bassa di ymm1
    vextractf128 xmm5, ymm1, 1; parte alta di ymm1
    vaddpd xmm1, xmm4, xmm5
_rest_loop:
    cmp rsi, 0
    je _end
    vmovsd xmm2, qword [rdi]
    vsubsd xmm2, xmm0; xi-mean
    vmulsd xmm2, xmm2; quadrato
    vaddsd xmm1, xmm2; aggiorno somma
    add rdi, 8
    sub rsi, 1
    jmp _rest_loop

_end:
    VCVTSI2SD xmm2, rdx; converto n-1 in double
    vdivsd xmm1, xmm2
    vsqrtsd xmm1, xmm1
    vmovsd xmm0, xmm1
    ; ------------------------------------------------------------
	; Sequenza di uscita dalla funzione
	; ------------------------------------------------------------
		
	;popaq; ripristina i registri generali
	mov rsp, rbp; ripristina lo Stack Pointer
	pop rbp; ripristina il Base Pointer
	ret; torna alla funzione C chiamante


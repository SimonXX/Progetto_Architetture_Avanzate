section .data
    ;const   dd 1000.0
section .text
	;extern double calcola_media(double* array, int n);
global calcola_media
calcola_media:
    ; ------------------------------------------------------------
	; Sequenza di ingresso nella funzione
	; ------------------------------------------------------------
	push rbp; salva il Base Pointer
	mov rbp, rsp; il Base Pointer punta al Record di Attivazione corrente
	;in RDI è presente l'indirizzo al vettore
	;in RSI è presente il valore di n.

	vxorpd ymm0, ymm0; somma totale=0
	mov rax, rsi; copio la dimensione dell'array

	cmp rsi, 4
	jl _loop_rest
	
_loop:
    vmovapd ymm1, [rdi]
    vaddpd ymm0, ymm1; aggiorno somma corrente.
    add rdi, 32; aggiorno l'indirizzo, 4 float più avanti
    sub rsi, 4; aggiorno la dimensione 
    cmp rsi, 3
    jg _loop
	vhaddpd ymm0, ymm0, ymm0
	vextractf128 xmm4, ymm0, 0; parte bassa di ymm0
    vextractf128 xmm5, ymm0, 1; parte alta di ymm0
    vaddpd xmm0, xmm4, xmm5
_loop_rest:
	cmp rsi, 0
	jle _end
	vaddsd xmm0, qword [rdi]
	add rdi, 8
	sub rsi, 1
	jmp _loop_rest

_end:
    VCVTSI2SD xmm1, rax
	vdivsd xmm0, xmm0, xmm1

	mov rsp, rbp; ripristina lo Stack Pointer
	pop rbp; ripristina il Base Pointer
	ret; torna alla funzione C chiamante

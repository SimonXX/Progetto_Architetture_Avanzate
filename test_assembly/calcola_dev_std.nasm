
section .data
    ;const   dd 1000.0
; section .bss			; Sezione contenente dati non inizializzati
; 	alignb 16
; 	fl		resd		1
section .text
global calcola_dev_std
calcola_dev_std:
    ; ------------------------------------------------------------
	; Sequenza di ingresso nella funzione
	; ------------------------------------------------------------
	push		ebp		; salva il Base Pointer
	mov		ebp, esp	; il Base Pointer punta al Record di Attivazione corrente
	push		ebx		; salva i registri da preservare
	push		esi
	push		edi

    mov eax, [ebp+8]; variabile risultato in cui inserire il valore ottenuto
    movss xmm0, [ebp+12]; media
    shufps xmm0, xmm0, 0; ricopio il valore della media in ogni celletta
    mov ebx, [ebp+16]; indirizzo vettore
    mov ecx, [ebp+20]; dimensione vettore
    mov edx, ecx; copio n
    sub edx, 1; salvo n-1
    xorps xmm1, xmm1; varianza=0.0

    cmp ecx, 4
    jl _rest_loop

_loop:
    movups xmm2, [ebx]
    subps xmm2, xmm0; xi-mean
    mulps xmm2, xmm2; elevo al quadrato
    addps xmm1, xmm2; aggiorno la somma
    add ebx, 16; aggiorno l'indirizzo, 4 float più avanti
    sub ecx, 4; aggiorno la dimensione 
    cmp ecx, 3
    jg _loop
_rest_loop:
    cmp ecx, 0
    je _end
    movss xmm2, [ebx]
    subss xmm2, xmm0; xi-mean
    mulss xmm2, xmm2; quadrato
    addss xmm1, xmm2; aggiorno somma
    add ebx, 4
    sub ecx, 1
    jmp _rest_loop

_end:
    CVTSI2SS xmm2, edx
    haddps xmm1, xmm1
    haddps xmm1, xmm1
    divss xmm1, xmm2
    sqrtss xmm1, xmm1

    movss [eax], xmm1; salva risultato
    pop	edi		; ripristina i registri da preservare
	pop	esi
	pop	ebx
	mov	esp, ebp	; ripristina lo Stack Pointer
	pop	ebp		; ripristina il Base Pointer
	ret			; torna alla funzione C chiamante

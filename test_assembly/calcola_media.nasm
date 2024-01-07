section .data
    ;const   dd 1000.0
section .text
global calcola_media
calcola_media:
    ; ------------------------------------------------------------
	; Sequenza di ingresso nella funzione
	; ------------------------------------------------------------
	push		ebp		; salva il Base Pointer
	mov		ebp, esp	; il Base Pointer punta al Record di Attivazione corrente
	push		ebx		; salva i registri da preservare
	push		esi
	push		edi

    mov ebx, [ebp+8]; variabile risultato in cui inserire il valore ottenuto
	mov ecx, [ebp+12]; indirizzo array
	mov edx, [ebp+16]; dimensione array
	xorps xmm0, xmm0; somma totale
	mov eax, edx; copio la dimensione dell'array

	cmp edx, 4
	jl _loop_rest
	
_loop:
    movups xmm1, [ecx]
    addps xmm0, xmm1
    add ecx, 16; aggiorno l'indirizzo, 4 float più avanti
    sub edx, 4; aggiorno la dimensione 
    cmp edx, 3
    jg _loop
_loop_rest:
	cmp edx, 0
	jle _end
	addss xmm0, [ecx]
	add ecx, 4
	sub edx, 1
	jmp _loop_rest

_end:
	haddps xmm0, xmm0
	haddps xmm0, xmm0
    CVTSI2SS xmm1, eax
	divss xmm0, xmm1
	movss [ebx], xmm0

	pop	edi		; ripristina i registri da preservare
	pop	esi
	pop	ebx
	mov	esp, ebp	; ripristina lo Stack Pointer
	pop	ebp		; ripristina il Base Pointer
	ret			; torna alla funzione C chiamante

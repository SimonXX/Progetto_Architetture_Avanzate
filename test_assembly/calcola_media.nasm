section .data
    const   dd 1000.0
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

    mov eax, [ebp+8]; variabile risultato in cui inserire il valore ottenuto
    movss xmm0, [const]
    mov edx, 10
    movss [eax], xmm0
    ;mov [eax], edx
	pop	edi		; ripristina i registri da preservare
	pop	esi
	pop	ebx
	mov	esp, ebp	; ripristina lo Stack Pointer
	pop	ebp		; ripristina il Base Pointer
	ret			; torna alla funzione C chiamante

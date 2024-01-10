;%include "sseutils32.nasm"
section .data
    ;const   dd 1000.0
 ;section .bss			; Sezione contenente dati non inizializzati
 ;	alignb 16
 ;	fl		resd		1
section .text
;METODO GET_COLUMN IMPLEMENTATO IN ASSEMBLY MEDIANTE TECNICA DEL LOOP UNROLLING
;FATTORE DI UNROLL PARI A 8
global get_column
get_column:
    ; ------------------------------------------------------------
	; Sequenza di ingresso nella funzione
	; ------------------------------------------------------------
	push		ebp		; salva il Base Pointer
	mov		ebp, esp	; il Base Pointer punta al Record di Attivazione corrente
	push		ebx		; salva i registri da preservare
	push		esi
	push		edi

    mov ecx, [ebp+8]; indirizzo dell'array risultato in cui inserire la colonna estratta
    mov ebx, [ebp+12]; indirizzo del dataset
    mov eax, [ebp+16]; numero di righe
    
    mov edi, [ebp+24]; colonna scelta
    
;for (int i = 0; i < rows; ++i) {
;        column[i] = matrix[i * cols + col_index];
;    }
    ;CVTSI2SS xmm0, eax
    ;movss [fl],xmm0
    ;printss fl
    mov esi, 8
    idiv esi
    push edx; pusho il resto della divisione intera. Il quoziente si trova in eax.
    mov edx, [ebp+20]; numero di colonne
    ;CVTSI2SS xmm0, edx
    ;movss [fl],xmm0
    ;printss fl
    mov esi, 0
    imul eax, 8
    cmp eax, 0
    je _prepare_loop_rest
_loop:
    cmp esi, eax
    jge _continue
    push edx
    imul edx, esi
    add edx, edi;offset/4
;1
    movss xmm0, [ebx + edx*4];leggo dataset[i*cols+col_index]
    movss [ecx+esi*4], xmm0;lo copio in column[i]
;2
    inc esi
    pop edx
    push edx
    imul edx, esi
    add edx, edi
    movss xmm0, [ebx+edx*4]
    movss [ecx+esi*4], xmm0
;3
    inc esi
    pop edx
    push edx
    imul edx, esi
    add edx, edi
    movss xmm0, [ebx+edx*4]
    movss [ecx+esi*4], xmm0
;4
    inc esi
    pop edx
    push edx
    imul edx, esi
    add edx, edi
    movss xmm0, [ebx+edx*4]
    movss [ecx+esi*4], xmm0
;5
    inc esi
    pop edx
    push edx
    imul edx, esi
    add edx, edi
    movss xmm0, [ebx+edx*4]
    movss [ecx+esi*4], xmm0
;6
    inc esi
    pop edx
    push edx
    imul edx, esi
    add edx, edi
    movss xmm0, [ebx+edx*4]
    movss [ecx+esi*4], xmm0
;7
    inc esi
    pop edx
    push edx
    imul edx, esi
    add edx, edi
    movss xmm0, [ebx+edx*4]
    movss [ecx+esi*4], xmm0
;8
    inc esi
    pop edx
    push edx
    imul edx, esi
    add edx, edi
    movss xmm0, [ebx+edx*4]
    movss [ecx+esi*4], xmm0

    inc esi
    pop edx

    jmp _loop

_continue:
    ; CVTSI2SS xmm0, edx
    ; movss [fl],xmm0
    ; printss fl

    push edx
    imul edx, esi
    imul edx, 4

    ; CVTSI2SS xmm0, edx
    ; movss [fl],xmm0
    ; printss fl

    add ebx, edx; aggiorno il puntatore al dataset. ora punta al prossimo elemento da leggere/estrarre

    imul esi, 4
    add ecx, esi; aggiorno il puntatore al risultato. ora punta alla prossima posizione in cui scrivere.

    pop edx; ripristino il valore di edx (ossia il numero di colonne)
    mov esi, 0; azzero l'indice in vista del prossimo loop.
_prepare_loop_rest:
    pop eax; inserisco il resto della divisione in eax.
_loop_rest:
    cmp esi, eax
    jge _end
    push edx
    imul edx, esi
    add edx, edi
    movss xmm0, [ebx+edx*4]
    movss [ecx+esi*4], xmm0
    pop edx
    inc esi
    jmp _loop_rest


_end:
    pop	edi		; ripristina i registri da preservare
	pop	esi
	pop	ebx
	mov	esp, ebp	; ripristina lo Stack Pointer
	pop	ebp		; ripristina il Base Pointer
	ret			; torna alla funzione C chiamante

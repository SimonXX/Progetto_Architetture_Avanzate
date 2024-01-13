%include "sseutils32.nasm"
;%include "final_cfs32c.o"
section .data
    ;const   dd 1000.0
 section .bss			; Sezione contenente dati non inizializzati
 	alignb 16
 	;fl	resd 1
    ;output resd 1
    score resd 1
    lables resd 1
    k resd 1
    num_features resd 1
    merito_corrente resd 1
    N resd 1
    a resd 1
    b resd 1
    c resd 1
    d resd 1
    ed resd 1
    i resd 1
    m0 resd 1

section .text
extern calcola_merito
;calcola_merito(float* dataset, int* selected_features, int num_chosen_features,float* labels, int N, int d, float* risultato){

%macro	merito	4
	mov	ecx, %1
	push ecx; pusho il puntatore al risultato
    mov ecx, [num_features]
    push ecx; pusho d
    mov ecx, [N]
    push ecx; pusho N
	mov	ecx, [lables]
	push ecx; pusho le lables
    mov ecx, %2
    push ecx; pusho il numero di feature selezionate
    mov ecx, %3
    push ecx; pusho il puntatore all'indice delle feature selezionate
    mov ecx, %4
    push ecx; pusho l'indirizzo del dataset
    ;printss m0
	call calcola_merito
	add	esp, 28
%endmacro

global calcola_cfs
; void calcola_cfs(float* ds, float* c, int k, int num_features, int N, int* out, float* score)
calcola_cfs:
    ; ------------------------------------------------------------
	; Sequenza di ingresso nella funzione
	; ------------------------------------------------------------
	push		ebp		; salva il Base Pointer
	mov		ebp, esp	; il Base Pointer punta al Record di Attivazione corrente
	push		ebx		; salva i registri da preservare
	push		esi
	push		edi

    mov eax, [ebp+8]; indirizzo del dataset
    mov ebx, [ebp+12]; indirizzo delle labels (c)
    mov [lables], ebx
    mov edi, [ebp+16]; k
    mov [k], edi
    mov ecx, [ebp+20]; num_features, aka numero delle colonne del dataset
    mov [num_features], ecx
    mov ecx, [ebp+24]
    mov [N], ecx; N
    mov edx, [ebp+28]; indirizzo dell'insieme S contenente le feature correnti selezionate. Corrisponde con "out", ossia scrivo direttamente il risultato.
    mov ebx, [ebp+32]
    ;mov [output], ebx; salvo in memoria l'indirizzo di out
    ;mov ebx, [ebp+36]
    mov [score], ebx; salvo in memoria l'indirizzo di score.
    
    mov esi, 0; current_size=0

    mov [merito_corrente], esi


_while:
    cmp esi, [k]
    jge _end
    mov ebx, -1; max_merit_feature_index = -1
    xorps xmm0, xmm0; max_merit=0.0
    mov edi, 0
    _for:
        cmp edi, [num_features]
        jge _aggiorna_while
        mov ecx, 0; indice
        _for_interno:
            cmp edi, [edx+ecx*4]
            je _true
            inc ecx
            cmp ecx, esi
            jge _continua
            jmp _for_interno
        _true:
            inc edi
            jmp _for
    _continua:
        mov [edx+esi*4], edi; s[current_size]=i
        ;calcolo merito corrente:
        inc esi
        ;movss [fl], xmm0
        ;printss fl
        mov [a], eax
        mov [b], ebx
        mov [c], ecx
        mov [d], edx
        mov [ed], edi
        mov [i], esi
        movss [m0], xmm0
        merito merito_corrente, esi, edx, eax
        mov eax, [a]
        mov ebx, [b]
        mov ecx, [c]
        mov edx, [d]
        mov edi, [ed]
        mov esi, [i]
        movss xmm0, [m0]
        sub esi, 1
        movss xmm1, [merito_corrente]
        ;movss [fl], xmm0
        ;printss fl
        ;printss merito_corrente
        ucomiss xmm1, xmm0
        jbe _skippa
        ;CVTSI2SS xmm2, ebx
        ;movss [fl], xmm2
        ;printss fl
        ;aggiorno il massimo
        movss xmm0, xmm1
        mov ebx, edi
    _skippa:
        inc edi
        jmp _for
_aggiorna_while:
    mov [edx+esi*4], ebx; s[current_size] = max_merit_feature_index
    inc esi
    jmp _while

_end:
    mov eax, [score]
    movss [eax], xmm0
    ;printss score
    ;Sequenza di uscita
    pop	edi		; ripristina i registri da preservare
	pop	esi
	pop	ebx
	mov	esp, ebp	; ripristina lo Stack Pointer
	pop	ebp		; ripristina il Base Pointer
	ret			; torna alla funzione C chiamante

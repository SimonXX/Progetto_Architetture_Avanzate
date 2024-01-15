;%include "sseutils32.nasm"
section .data
    ;const   dd 1000.0
 section .bss			; Sezione contenente dati non inizializzati
 	alignb 16
 	fl		resd		1
section .text
global calcola_rff
calcola_rff:
    ; ------------------------------------------------------------
	; Sequenza di ingresso nella funzione
	; ------------------------------------------------------------
	push		ebp		; salva il Base Pointer
	mov		ebp, esp	; il Base Pointer punta al Record di Attivazione corrente
	push		ebx		; salva i registri da preservare
	push		esi
	push		edi

    mov ecx, [ebp+8]; variabile risultato in cui inserire il valore ottenuto
    mov eax, [ebp+12]; feature x
    movss xmm0, [ebp+16]; mean x
    shufps xmm0, xmm0, 0; ricopio il valore in ogni celletta del registro
    mov ebx, [ebp+20]; feature y
    movss xmm1, [ebp+24]; mean y
    shufps xmm1, xmm1, 0
    mov edi, [ebp+28]; dimensione delle feature
    xorps xmm4, xmm4; numerator = 0
    xorps xmm5, xmm5; denomx = 0
    xorps xmm6, xmm6; denomy = 0

    cmp edi, 16
    jl _rest_loop
;     for (int i = 0; i < n; ++i) {
;         numerator += (feature_x[i] - mean_x) * (feature_y[i] - mean_y);
;         denom_x += powf(feature_x[i] - mean_x, 2);
;         denom_y += powf(feature_y[i] - mean_y, 2);
;     }

;     float r_ff = numerator / (sqrtf(denom_x) * sqrtf(denom_y));
;     return r_ff;
_loop:
    movaps xmm2, [eax]; carico x
    movaps xmm3, [ebx]; carico y
    subps xmm2, xmm0; calcolo x[i]-meanx
    subps xmm3, xmm1; calcolo y[i]-meany

    movaps xmm7, xmm2
    mulps xmm7, xmm3; calcolo delta numeratore
    addps xmm4, xmm7; aggiorno numeratore
    mulps xmm2, xmm2; calcolo il quadrato
    mulps xmm3, xmm3; calcolo il quadrato
    addps xmm5, xmm2; aggiorno denomx
    addps xmm6, xmm3; aggiorno denomy


    add eax, 16; aggiorno l'indirizzo di x
    add ebx, 16; aggiorno l'indirizzo di y, 4 float più avanti

    ; 2
    movaps xmm2, [eax]; carico x
    movaps xmm3, [ebx]; carico y
    subps xmm2, xmm0; calcolo x[i]-meanx
    subps xmm3, xmm1; calcolo y[i]-meany

    movaps xmm7, xmm2
    mulps xmm7, xmm3; calcolo delta numeratore
    addps xmm4, xmm7; aggiorno numeratore
    mulps xmm2, xmm2; calcolo il quadrato
    mulps xmm3, xmm3; calcolo il quadrato
    addps xmm5, xmm2; aggiorno denomx
    addps xmm6, xmm3; aggiorno denomy


    add eax, 16; aggiorno l'indirizzo di x
    add ebx, 16; aggiorno l'indirizzo di y, 4 float più avanti
    ;3
    movaps xmm2, [eax]; carico x
    movaps xmm3, [ebx]; carico y
    subps xmm2, xmm0; calcolo x[i]-meanx
    subps xmm3, xmm1; calcolo y[i]-meany

    movaps xmm7, xmm2
    mulps xmm7, xmm3; calcolo delta numeratore
    addps xmm4, xmm7; aggiorno numeratore
    mulps xmm2, xmm2; calcolo il quadrato
    mulps xmm3, xmm3; calcolo il quadrato
    addps xmm5, xmm2; aggiorno denomx
    addps xmm6, xmm3; aggiorno denomy


    add eax, 16; aggiorno l'indirizzo di x
    add ebx, 16; aggiorno l'indirizzo di y, 4 float più avanti
    ;4
    movaps xmm2, [eax]; carico x
    movaps xmm3, [ebx]; carico y
    subps xmm2, xmm0; calcolo x[i]-meanx
    subps xmm3, xmm1; calcolo y[i]-meany

    movaps xmm7, xmm2
    mulps xmm7, xmm3; calcolo delta numeratore
    addps xmm4, xmm7; aggiorno numeratore
    mulps xmm2, xmm2; calcolo il quadrato
    mulps xmm3, xmm3; calcolo il quadrato
    addps xmm5, xmm2; aggiorno denomx
    addps xmm6, xmm3; aggiorno denomy


    add eax, 16; aggiorno l'indirizzo di x
    add ebx, 16; aggiorno l'indirizzo di y, 4 float più avanti


    sub edi, 16; aggiorno la dimensione 
    cmp edi, 15
    jg _loop
_rest_loop:
    ; movss [fl], xmm5
    ; printss fl
    ; movss [fl], xmm6
    ; printss fl
    ; movss [fl], xmm4
    ; printss fl
    cmp edi, 0
    je _end
    movss xmm2, [eax]; carico x
    movss xmm3, [ebx]; carico y
    subss xmm2, xmm0
    subss xmm3, xmm1
    movss xmm7, xmm2
    mulss xmm7, xmm3; calcolo delta numeratore
    addss xmm4, xmm7; aggiorno numeratore
    mulss xmm2, xmm2
    mulss xmm3, xmm3
    addss xmm5, xmm2; aggiorno denomx
    addss xmm6, xmm3; aggiorno denomy

    add eax, 4
    add ebx, 4; aggiorno indirizzi
    sub edi, 1; aggiorno dimensione vettore
    jmp _rest_loop

_end:
    haddps xmm5, xmm5
    haddps xmm5, xmm5; unisco i risultati
    haddps xmm6, xmm6
    haddps xmm6, xmm6; unisco risultati denomy
    haddps xmm4, xmm4; unisco risultati numeratore
    haddps xmm4, xmm4

    sqrtss xmm5, xmm5; calcolo sqrt denomx
    sqrtss xmm6, xmm6; calcolo sqrt denomy
    mulss xmm5, xmm6
    divss xmm4, xmm5; calcolo risultato

    movss [ecx], xmm4; salva risultato
    pop	edi		; ripristina i registri da preservare
	pop	esi
	pop	ebx
	mov	esp, ebp	; ripristina lo Stack Pointer
	pop	ebp		; ripristina il Base Pointer
	ret			; torna alla funzione C chiamante

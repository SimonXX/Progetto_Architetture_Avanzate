;%include "sseutils32.nasm"
section .data
    ;const   dd 1000.0
 ;section .bss			; Sezione contenente dati non inizializzati
; 	alignb 16
; 	fl		resd		1
section .text
    ;double calcola_rff(double* feature_x, double mean_x, double* feature_y, double mean_y, int n);
global calcola_rff
calcola_rff:
    ; ------------------------------------------------------------
	; Sequenza di ingresso nella funzione
	; ------------------------------------------------------------
	push rbp; salva il Base Pointer
	mov rbp, rsp; il Base Pointer punta al Record di Attivazione corrente
    ;RDI contiene l'indirizzo a x
    ;xmm0 contiene mean_x
    ;RSI contiene l'indirizzo a y
    ;xmm1 contiene mean_y
    ;RDX contiene il valore di n

    
    VBROADCASTSD ymm0, xmm0; ricopio il valore di mean_x in ogni celletta del registro
    VBROADCASTSD ymm1, xmm1; ricopio il valore di mean_y in ogni celletta
    vxorpd ymm4, ymm4, ymm4; numerator = 0
    vxorpd ymm5, ymm5, ymm5; denomx = 0
    vxorpd ymm6, ymm6, ymm6; denomy = 0

    cmp rdx, 20
    jl _rest_loop
;     for (int i = 0; i < n; ++i) {
;         numerator += (feature_x[i] - mean_x) * (feature_y[i] - mean_y);
;         denom_x += powf(feature_x[i] - mean_x, 2);
;         denom_y += powf(feature_y[i] - mean_y, 2);
;     }

;     float r_ff = numerator / (sqrtf(denom_x) * sqrtf(denom_y));
;     return r_ff;
_loop:
    vmovapd ymm2, [rsi]; carico x
    vmovapd ymm3, [rdi]; carico y
    vsubpd ymm2, ymm0; calcolo x[i]-meanx
    vsubpd ymm3, ymm1; calcolo y[i]-meany

    vmovapd ymm7, ymm2
    vmulpd ymm7, ymm3; calcolo delta numeratore
    vaddpd ymm4, ymm7; aggiorno numeratore
    vmulpd ymm2, ymm2; calcolo il quadrato
    vmulpd ymm3, ymm3; calcolo il quadrato
    vaddpd ymm5, ymm2; aggiorno denomx
    vaddpd ymm6, ymm3; aggiorno denomy


    add rdi, 32; aggiorno l'indirizzo di x
    add rsi, 32; aggiorno l'indirizzo di y, 4 double più avanti
    ;2
    vmovapd ymm2, [rsi]; carico x
    vmovapd ymm3, [rdi]; carico y
    vsubpd ymm2, ymm0; calcolo x[i]-meanx
    vsubpd ymm3, ymm1; calcolo y[i]-meany

    vmovapd ymm7, ymm2
    vmulpd ymm7, ymm3; calcolo delta numeratore
    vaddpd ymm4, ymm7; aggiorno numeratore
    vmulpd ymm2, ymm2; calcolo il quadrato
    vmulpd ymm3, ymm3; calcolo il quadrato
    vaddpd ymm5, ymm2; aggiorno denomx
    vaddpd ymm6, ymm3; aggiorno denomy


    add rdi, 32; aggiorno l'indirizzo di x
    add rsi, 32; aggiorno l'indirizzo di y, 4 double più avanti
    ;3
    vmovapd ymm2, [rsi]; carico x
    vmovapd ymm3, [rdi]; carico y
    vsubpd ymm2, ymm0; calcolo x[i]-meanx
    vsubpd ymm3, ymm1; calcolo y[i]-meany

    vmovapd ymm7, ymm2
    vmulpd ymm7, ymm3; calcolo delta numeratore
    vaddpd ymm4, ymm7; aggiorno numeratore
    vmulpd ymm2, ymm2; calcolo il quadrato
    vmulpd ymm3, ymm3; calcolo il quadrato
    vaddpd ymm5, ymm2; aggiorno denomx
    vaddpd ymm6, ymm3; aggiorno denomy


    add rdi, 32; aggiorno l'indirizzo di x
    add rsi, 32; aggiorno l'indirizzo di y, 4 double più avanti
    ;4
    vmovapd ymm2, [rsi]; carico x
    vmovapd ymm3, [rdi]; carico y
    vsubpd ymm2, ymm0; calcolo x[i]-meanx
    vsubpd ymm3, ymm1; calcolo y[i]-meany

    vmovapd ymm7, ymm2
    vmulpd ymm7, ymm3; calcolo delta numeratore
    vaddpd ymm4, ymm7; aggiorno numeratore
    vmulpd ymm2, ymm2; calcolo il quadrato
    vmulpd ymm3, ymm3; calcolo il quadrato
    vaddpd ymm5, ymm2; aggiorno denomx
    vaddpd ymm6, ymm3; aggiorno denomy


    add rdi, 32; aggiorno l'indirizzo di x
    add rsi, 32; aggiorno l'indirizzo di y, 4 double più avanti
    ;5
    vmovapd ymm2, [rsi]; carico x
    vmovapd ymm3, [rdi]; carico y
    vsubpd ymm2, ymm0; calcolo x[i]-meanx
    vsubpd ymm3, ymm1; calcolo y[i]-meany

    vmovapd ymm7, ymm2
    vmulpd ymm7, ymm3; calcolo delta numeratore
    vaddpd ymm4, ymm7; aggiorno numeratore
    vmulpd ymm2, ymm2; calcolo il quadrato
    vmulpd ymm3, ymm3; calcolo il quadrato
    vaddpd ymm5, ymm2; aggiorno denomx
    vaddpd ymm6, ymm3; aggiorno denomy


    add rdi, 32; aggiorno l'indirizzo di x
    add rsi, 32; aggiorno l'indirizzo di y, 4 double più avanti

    sub rdx, 20; aggiorno la dimensione 
    cmp rdx, 19
    jg _loop

    vhaddpd ymm4, ymm4, ymm4
    vextractf128 xmm14, ymm4, 0; parte bassa
    vextractf128 xmm15, ymm4, 1; parte alta 
    vaddpd xmm4, xmm14, xmm15

    vhaddpd ymm5, ymm5, ymm5
    vextractf128 xmm14, ymm5, 0; parte bassa
    vextractf128 xmm15, ymm5, 1; parte alta 
    vaddpd xmm5, xmm14, xmm15

    vhaddpd ymm6, ymm6, ymm6
    vextractf128 xmm14, ymm6, 0; parte bassa
    vextractf128 xmm15, ymm6, 1; parte alta 
    vaddpd xmm6, xmm14, xmm15  
    
_rest_loop:
    cmp rdx, 0
    je _end
    vmovsd xmm2, [rdi]; carico x
    vmovsd xmm3, [rsi]; carico y
    vsubsd xmm2, xmm0
    vsubsd xmm3, xmm1
    vmovsd xmm7, xmm2
    vmulsd xmm7, xmm3; calcolo delta numeratore
    vaddsd xmm4, xmm7; aggiorno numeratore
    vmulsd xmm2, xmm2
    vmulsd xmm3, xmm3
    vaddsd xmm5, xmm2; aggiorno denomx
    vaddsd xmm6, xmm3; aggiorno denomy

    add rdi, 8
    add rsi, 8; aggiorno indirizzi
    sub rdx, 1; aggiorno dimensione vettore
    jmp _rest_loop  

_end:
    vsqrtsd xmm5, xmm5; calcolo sqrt denomx
    vsqrtsd xmm6, xmm6; calcolo sqrt denomy
    vmulsd xmm5, xmm6
    vdivsd xmm4, xmm5; calcolo risultato

    vmovsd xmm0, xmm4; sposto il risultato
    mov rsp, rbp; ripristina lo Stack Pointer
	pop rbp; ripristina il Base Pointer
	ret; torna alla funzione C chiamante

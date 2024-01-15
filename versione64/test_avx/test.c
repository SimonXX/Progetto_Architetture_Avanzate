#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <xmmintrin.h>

void* get_block(int size, int elements) { 
	return _mm_malloc(elements*size,32); 
}

void free_block(void* p) { 
	_mm_free(p);
}

extern double calcola_media(double* array, int n);

double mean(double* v, int n){
    double somma = 0;
    for(int i=0; i<n; i++) somma += v[i];
    return somma/n;
}

int main(){
    int k = 727;
    double* v = get_block(k, sizeof(double));
    for(int i=1; i<k+1;i++) v[i-1] = (double)i;
    double risultato=calcola_media(v,k);
    printf("valore medio corretto:%f\n", mean(v,k));
    printf("Il valore medio del vettore è %f\n", risultato);
    free_block(v);
}
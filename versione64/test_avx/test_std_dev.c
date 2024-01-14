#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <xmmintrin.h>

double calculate_mean(double *data, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        sum += data[i];
    }
    double mean = sum / n;
    return mean;
}

// Funzione per calcolare la deviazione standard di un vettore di valori
double calculate_std_dev(double *data, int n) {
    double mean = calculate_mean(data, n);
    double variance = 0.0;
    for (int i = 0; i < n; ++i) {
        variance += pow(data[i] - mean, 2);
    }
    double std_dev = sqrt(variance / (n - 1));
    return std_dev;
}

void* get_block(int size, int elements) { 
	return _mm_malloc(elements*size,32); 
}

void free_block(void* p) { 
	_mm_free(p);
}

extern double calcola_dev_std(double media, double* data, int n);

int main(){
    int k = 43;
    double* v = get_block(k, sizeof(double));
    for(int i=1; i<k+1;i++) v[i-1] = (double)i;
    //double v[] = {1.0,2.0,3.0,4.0,  5.0,6.0,7.0,8.0,  9.0,10.0,11.0,12.0,  13.0,14.0,15.0,16.0,  17.0,18.0,19.0,20.0};
    printf("Deviazione standard:%f\n",calculate_std_dev(v,k));
    double risultato;
    risultato = calcola_dev_std(calculate_mean(v,k), v, k);
    printf("Deviazione standard calcolata:%lf\n", risultato);
    free_block(v);
}
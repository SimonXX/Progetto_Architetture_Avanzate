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

double calculate_mean(double *data, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        sum += data[i];
    }
    double mean = sum / n;
    return mean;
}

double calculate_ff_corr(double *feature_x, double *feature_y, int n) {
    double mean_x = calculate_mean(feature_x, n);
    double mean_y = calculate_mean(feature_y, n);

    double numerator = 0.0f, denom_x = 0.0f, denom_y = 0.0f;

    for (int i = 0; i < n; ++i) {
        numerator += (feature_x[i] - mean_x) * (feature_y[i] - mean_y);
        denom_x += powf(feature_x[i] - mean_x, 2);
        denom_y += powf(feature_y[i] - mean_y, 2);
    }

    double r_ff = numerator / (sqrtf(denom_x) * sqrtf(denom_y));
    return r_ff;
}

extern double calcola_rff(double* feature_x, double mean_x, double* feature_y, double mean_y, int n);

int main(){
    int k = 600;
    double* x = get_block(k, sizeof(double));
    double* y = get_block(k, sizeof(double));
    for(int i=1; i<k+1;i++) {
        x[i-1] = (double)i;
        y[i-1] = (double)k-i;
    }
    double risultato=calcola_rff(x,calculate_mean(x,k),y,calculate_mean(y,k),k);
    printf("Rff calcolato:%f\n",calculate_ff_corr(x,y,k));
    printf("Rff calcolato in assembly:%f\n",risultato);

}
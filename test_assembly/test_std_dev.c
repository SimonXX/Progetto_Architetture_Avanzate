#include <stdio.h>
#include <stdlib.h>
#include <math.h>

float calculate_mean(float *data, int n) {
    float sum = 0.0;
    for (int i = 0; i < n; ++i) {
        sum += data[i];
    }
    float mean = sum / n;
    return mean;
}

// Funzione per calcolare la deviazione standard di un vettore di valori
float calculate_std_dev(float *data, int n) {
    float mean = calculate_mean(data, n);
    float variance = 0.0;
    for (int i = 0; i < n; ++i) {
        variance += pow(data[i] - mean, 2);
    }
    float std_dev = sqrt(variance / (n - 1));
    return std_dev;
}

extern void calcola_dev_std(float* risultato, float media, float* data, int n);

int main(){
    float v[] = {1.0,2.0,3.0,4.0,5.0,6.0,7.0};
    printf("Deviazione standard:%f\n",calculate_std_dev(v,7));
    float risultato;
    calcola_dev_std(&risultato, calculate_mean(v,7), v, 7);
    printf("Deviazione standard calcolata:%f\n", risultato);
}
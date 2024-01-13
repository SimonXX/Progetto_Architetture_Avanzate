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
    float v[] = {1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,11.0,12.0,13.0,14.0,15.0,16.0,17.0,18.0,19.0};
    printf("Deviazione standard:%f\n",calculate_std_dev(v,18));
    float risultato;
    calcola_dev_std(&risultato, calculate_mean(v,18), v, 18);
    printf("Deviazione standard calcolata:%f\n", risultato);
}
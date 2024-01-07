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

float calculate_ff_corr(float *feature_x, float *feature_y, int n) {
    float mean_x = calculate_mean(feature_x, n);
    float mean_y = calculate_mean(feature_y, n);

    float numerator = 0.0f, denom_x = 0.0f, denom_y = 0.0f;

    for (int i = 0; i < n; ++i) {
        numerator += (feature_x[i] - mean_x) * (feature_y[i] - mean_y);
        denom_x += powf(feature_x[i] - mean_x, 2);
        denom_y += powf(feature_y[i] - mean_y, 2);
    }

    float r_ff = numerator / (sqrtf(denom_x) * sqrtf(denom_y));
    return r_ff;
}

extern void calcola_rff(float* risultato, float* feature_x, float mean_x, float* feature_y, float mean_y, int n);

int main(){
    float x[] = {1.0,2.0,3.0,4.0,5.0,6.0,7.0};
    float y[] = {14.0,13.0,12.0,11.0,10.0,9.0,8.0};
    float risultato;
    calcola_rff(&risultato,x,calculate_mean(x,7),x,calculate_mean(x,7),7);
    printf("Rff calcolato:%f\n",calculate_ff_corr(x,x,7));
    printf("Rff calcolato in assembly:%f\n",risultato);

}
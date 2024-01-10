#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <xmmintrin.h>

extern void calcola_dev_std(float* risultato, float media, float* data, int n);
extern void calcola_rff(float* risultato, float* feature_x, float mean_x, float* feature_y, float mean_y, int n);
extern void get_column(float* risultato, float* dataset, int rows, int columns, int choosed_col);
extern void calcola_media(float* risultato, float* array, int n);

float calculate_cf_corr(float *feature, float *labels, int n) {
    float mean_0 = 0.0, mean_1 = 0.0;
    int count_0 = 0, count_1 = 0;

    for (int i = 0; i < n; ++i) {
        if (labels[i] == 0) {
            mean_0 += feature[i];
            count_0++;
        } else {
            mean_1 += feature[i];
            count_1++;
        }
    }

    mean_0 /= count_0;
    mean_1 /= count_1;

    float std_dev;
    float media;
    calcola_media(&media, feature, n);
    calcola_dev_std(&std_dev, media, feature, n);

    float r_cf = ((mean_0 - mean_1) / std_dev) * sqrt((count_0 * count_1) / pow(n, 2));
    return r_cf;
}

float calculate_avg_cf_corr(float* ds, int* selected_features, int num_chosen_features, float* labels, int N, int d) {
    float total_cf_corr = 0.0;
    for (int a = 0; a < num_chosen_features; ++a) {
        int i=selected_features[a];
        float* column;
        get_column(column, ds, N, d, i);
        float cf_corr = fabsf(calculate_cf_corr(column, labels, N));
        total_cf_corr += cf_corr;
    }
    float avg_cf_corr = total_cf_corr  / num_chosen_features;
    //float avg_cf_corr = total_cf_corr  ;
    return avg_cf_corr;
}

float calculate_avg_ff_corr(float *ds, int* selected_features, int num_chosen_features, int N, int d) {
    float total_ff_corr = 0.0f;
    int num_pairs = num_chosen_features * (num_chosen_features - 1) / 2;
    if(num_chosen_features==1) {
        return 1.0;
    }
    for(int a=0;a<num_chosen_features;a++){
        float* feature_a;
        get_column(feature_a, ds, N, d, selected_features[a]);
        for(int b=a+1;b<num_chosen_features;b++){
            float* feature_b;
            get_column(feature_b, ds, N, d, selected_features[b]);
            float rff;
            float media_a, media_b;
            calcola_media(&media_a, feature_a, N); calcola_media(&media_b, feature_b, N);
            calcola_rff(&rff, feature_a, media_a, feature_b, media_b, N);
            total_ff_corr += fabsf(rff);
        }
    }
    float avg_ff_corr = total_ff_corr / num_pairs;
    return avg_ff_corr;
}

float calculate_merit(float avg_cf_corr, float avg_ff_corr, int k) {
    float merit = (k * avg_cf_corr) / sqrtf(k + k * (k - 1) * avg_ff_corr);
    
    //float merit =  avg_cf_corr / sqrtf(k + k * (k - 1) * avg_ff_corr);
    return merit;
}

void calcola_merito(float* dataset, int* selected_features, int num_chosen_features,float* labels, int N, int d, float* risultato){
    *risultato = calculate_merit(calculate_avg_cf_corr(dataset, selected_features, num_chosen_features, labels, N, d), calculate_avg_ff_corr(dataset, selected_features, num_chosen_features, N, d), num_chosen_features);
}
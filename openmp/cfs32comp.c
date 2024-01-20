#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <xmmintrin.h>

#define type float
#define MATRIX type*
#define VECTOR type*

typedef struct {
    MATRIX ds;
    VECTOR labels;
    int* out;\
    type sc;
    int k;
    int N;
    int d;
    int display;
    int silent;
} params;

void* get_block(int size, int elements) { 
    return _mm_malloc(elements*size,16); 
}

void free_block(void* p) { 
    _mm_free(p);
}

MATRIX alloc_matrix(int rows, int cols) {
    return (MATRIX) get_block(sizeof(type), rows * cols);
}

int* alloc_int_matrix(int rows, int cols) {
    return (int*) get_block(sizeof(int), rows * cols);
}

void dealloc_matrix(void* mat) {
    free_block(mat);
}

MATRIX load_data(char* filename, int *n, int *k) {
    FILE* fp;
    int rows, cols, status, i;
    fp = fopen(filename, "rb");
    if (fp == NULL){
        printf("'%s': bad data file name!\n", filename);
        exit(0);
    }
    status = fread(&cols, sizeof(int), 1, fp);
    status = fread(&rows, sizeof(int), 1, fp);
    MATRIX data = alloc_matrix(rows, cols);
    status = fread(data, sizeof(type), rows * cols, fp);
    fclose(fp);
    *n = rows;
    *k = cols;
    return data;
}

void save_data(char* filename, void* X, int n, int k) {
    FILE* fp;
    int i;
    fp = fopen(filename, "wb");
    if(X != NULL){
        fwrite(&k, 4, 1, fp);
        fwrite(&n, 4, 1, fp);
        for (i = 0; i < n; i++) {
            fwrite(X, sizeof(type), k, fp);
            X += sizeof(type) * k;
        }
    } else {
        int x = 0;
        fwrite(&x, 4, 1, fp);
        fwrite(&x, 4, 1, fp);
    }
    fclose(fp);
}

void save_out(char* filename, type sc, int* X, int k) {
    FILE* fp;
    int i;
    int n = 1;
    k++;
    fp = fopen(filename, "wb");
    if(X != NULL){
        fwrite(&n, 4, 1, fp);
        fwrite(&k, 4, 1, fp);
        fwrite(&sc, sizeof(type), 1, fp);
        fwrite(X, sizeof(int), k, fp);
    }
    fclose(fp);
}

extern void calcola_dev_std(float* risultato, float media, float* data, int n);
extern void calcola_rff(float* risultato, float* feature_x, float mean_x, float* feature_y, float mean_y, int n);
extern void get_column(float* risultato, float* dataset, int rows, int columns, int choosed_col);
extern void calcola_media(float* risultato, float* array, int n);
extern void calcola_cfs(float* ds, float* c, int k, int num_features, int N, int* out, float* score);

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
    #pragma omp parallel for
    for (int a = 0; a < num_chosen_features; ++a) {
        int i=selected_features[a];
        float* column = get_block(N,sizeof(float));
        get_column(column, ds, N, d, i);
        float cf_corr = fabsf(calculate_cf_corr(column, labels, N));
        free_block(column);
        total_cf_corr += cf_corr;
    }
    float avg_cf_corr = total_cf_corr  / num_chosen_features;
    return avg_cf_corr;
}

float secondo_for(int a, int* selected_features, int num_chosen_features, float* feature_b, float* feature_a, float media_a, float* ds, int N, int d){
    float total_ff_corr=0.0;
    for(int b=a+1;b<num_chosen_features;b++){
            get_column(feature_b, ds, N, d, selected_features[b]);
            float rff;
            float media_b;
            calcola_media(&media_b, feature_b, N);
            calcola_rff(&rff, feature_a, media_a, feature_b, media_b, N);
            total_ff_corr += fabsf(rff);
        }
        return total_ff_corr;
}


float calculate_avg_ff_corr(float *ds, int* selected_features, int num_chosen_features, int N, int d) {
    float total_ff_corr = 0.0f;
    int num_pairs = num_chosen_features * (num_chosen_features - 1) / 2;
    if(num_chosen_features==1) {
        return 1.0;
    }
    #pragma omp parallel for
    for(int a=0;a<num_chosen_features;a++){
        float* feature_a = get_block(N, sizeof(float));
        float media_a;
        calcola_media(&media_a, feature_a, N);
        get_column(feature_a, ds, N, d, selected_features[a]);
        float* feature_b = get_block(N, sizeof(float));
        
        total_ff_corr += secondo_for(a, selected_features, num_chosen_features, feature_b, feature_a, media_a, ds, N, d);

        free_block(feature_a);
        free_block(feature_b);
    }
    float avg_ff_corr = total_ff_corr / num_pairs;
    return avg_ff_corr;
}

void calcola_merito(float* dataset, int* selected_features, int num_chosen_features,float* labels, int N, int d, float* risultato){
    *risultato = (num_chosen_features * calculate_avg_cf_corr(dataset, selected_features, num_chosen_features, labels, N, d)) / sqrtf( num_chosen_features + num_chosen_features*(num_chosen_features-1) * calculate_avg_ff_corr(dataset, selected_features, num_chosen_features, N, d) );
    return;
}

void cfs(params* input){
    float* dataset = input->ds;
    float* c = input->labels;
    int k = input->k;
    int num_features = input->d;
    int N = input->N;
    calcola_cfs(dataset, c, k, num_features, N, input->out, &(input->sc));
}

int main(int argc, char** argv) {

    char fname[256];
    char* dsfilename = NULL;
    char* labelsfilename = NULL;
    clock_t t;
    float time;
    
    //params* input = get_block(sizeof(params),1);
    params* input = malloc(sizeof(params));
    input->ds = NULL;
    input->labels = NULL;
    input->k = -1;
    input->sc = -1;
    input->silent = 0;
    input->display = 0;

    if(argc <= 1){
        printf("%s -ds <DS> -labels <LABELS> -k <K> [-s] [-d]\n", argv[0]);
        printf("\nParameters:\n");
        printf("\tDS: dataset file name\n");
        printf("\tLABELS: labels file name\n");
        printf("\tk: number of features to extract\n");
        printf("\nOptions:\n");
        printf("\t-s: silent mode, no output, default 0 - false\n");
        printf("\t-d: display results on screen, default 0 - false\n");
        exit(0);
    }

    int par = 1;
    while (par < argc) {
        if (strcmp(argv[par],"-s") == 0) {
            input->silent = 1;    
            par++;
        } else if (strcmp(argv[par],"-d") == 0) {
            input->display = 1;
            par++;
        } else if (strcmp(argv[par],"-ds") == 0) {
            par++;
            if (par >= argc) {
                printf("Missing dataset file name!\n");
                exit(1);
            }
            dsfilename = argv[par];
            par++;
        } else if (strcmp(argv[par],"-labels") == 0) {
            par++;
            if (par >= argc) {
                printf("Missing labels file name!\n");
                exit(1);
            }
            labelsfilename = argv[par];
            par++;
        } else if (strcmp(argv[par],"-k") == 0) {
            par++;
            if (par >= argc) {
                printf("Missing k value!\n");
                exit(1);
            }
            input->k = atoi(argv[par]);
            par++;
        } else{
            printf("WARNING: unrecognized parameter '%s'!\n",argv[par]);
            par++;
        }
    }

    if(dsfilename == NULL || strlen(dsfilename) == 0){
        printf("Missing ds file name!\n");
        exit(1);
    }

    if(labelsfilename == NULL || strlen(labelsfilename) == 0){
        printf("Missing labels file name!\n");
        exit(1);
    }

    input->ds = load_data(dsfilename, &input->N, &input->d);
    int nl, dl;
    input->labels = load_data(labelsfilename, &nl, &dl);

    if(nl != input->N || dl != 1){
        printf("Invalid size of labels file, should be %ix1!\n", input->N);
        exit(1);
    }

    if(input->k <= 0){
        printf("Invalid value of k parameter!\n");
        exit(1);
    }

    input->out = alloc_int_matrix(input->k, 1);

    t = clock();
    cfs(input);
    t = clock() - t;
    
    time = ((float)t)/CLOCKS_PER_SEC;

    if(!input->silent)
        printf("CFS time = %.3f secs\n", time);
    else
        printf("%.3f\n", time);

    sprintf(fname, "out32_%d_%d_%d.ds2", input->N, input->d, input->k);
    save_out(fname, input->sc, input->out, input->k);

    if(input->display){
        if(input->out == NULL)
            printf("out: NULL\n");
        else{
            int i,j;
            printf("sc: %f, out: [", input->sc);
            for(i=0; i<input->k; i++){
                printf("%i,", input->out[i]);
            }
            printf("]\n");
        }
    }

    if(!input->silent)
        printf("\nDone.\n");

    dealloc_matrix(input->ds);
    dealloc_matrix(input->labels);
    dealloc_matrix(input->out);
    free(input);

    return 0;
}

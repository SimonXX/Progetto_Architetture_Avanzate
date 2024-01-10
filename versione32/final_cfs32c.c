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

void stampaMatrice(float* matrix, int r, int c){
    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            printf("%.2f ", matrix[i * c + j]);
        }
        printf("\n");
    }
}

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

//extern float calculate_mean(float *data, int n); // Declaration of the assembly function
extern void calcola_cfs(float* ds, float* c, int k, int num_features, int N, int* out, float* score);

float calculate_mean(float *data, int n) {
    float sum = 0.0;
    for (int i = 0; i < n; ++i) {
        sum += data[i];
    }
    float mean = sum / n;
    return mean;
}

float calculate_std_dev(float *data, int n) {
    float mean = calculate_mean(data, n);
    float variance = 0.0;
    for (int i = 0; i < n; ++i) {
        variance += pow(data[i] - mean, 2);
    }
    float std_dev = sqrt(variance / (n - 1));
    return std_dev;
}

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

    float std_dev = calculate_std_dev(feature, n);

    float r_cf = ((mean_0 - mean_1) / std_dev) * sqrt((count_0 * count_1) / pow(n, 2));
    return r_cf;
}

float* getColumn(float* matrix, int rows, int cols, int col_index) {
    float* column = malloc(rows * sizeof(float));
    for (int i = 0; i < rows; ++i) {
        column[i] = matrix[i * cols + col_index];
    }
    return column;
}

float calculate_avg_cf_corr(float* ds, int* selected_features, int num_chosen_features, float* labels, int N, int d) {
    float total_cf_corr = 0.0;
    for (int a = 0; a < num_chosen_features; ++a) {
        int i=selected_features[a];
        float* column = getColumn(ds, N, d, i);
        float cf_corr = fabsf(calculate_cf_corr(column, labels, N));
        total_cf_corr += cf_corr;
    }
    float avg_cf_corr = total_cf_corr  / num_chosen_features;
    //float avg_cf_corr = total_cf_corr  ;
    return avg_cf_corr;
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

float calculate_avg_ff_corr(float *ds, int* selected_features, int num_chosen_features, int N, int d) {
    float total_ff_corr = 0.0f;
    int num_pairs = num_chosen_features * (num_chosen_features - 1) / 2;
    if(num_chosen_features==1) {
        return 1.0;
    }
    for(int a=0;a<num_chosen_features;a++){
        float* feature_a = getColumn(ds, N, d, selected_features[a]);
        for(int b=a+1;b<num_chosen_features;b++){
            float* feature_b = getColumn(ds, N, d, selected_features[b]);
            total_ff_corr += fabsf(calculate_ff_corr(feature_a, feature_b, N));
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
    return;
}

void print_merit_info(int current_size, int max_merit_feature_index, float max_merit) {
    printf("Current Size: %d, Max Merit Feature Index: %d, Max Merit: %f\n", current_size, max_merit_feature_index, max_merit);
}

void cfs(params* input){

    float* dataset = input->ds;
    float* c = input->labels;
    int k = input->k;
    int num_features = input->d;
    int N = input->N;
    int* s = malloc(k * sizeof(int));
    int current_size = 0;
    //float score;
    calcola_cfs(dataset, c, k, num_features, N, input->out, &(input->sc));
    //input->sc = score;
    //s[0]=0; non dovrebbe servire, era per debug.
    /*
    int max_merit_feature_index;
    float max_merit;


 
    while (current_size < k) {
        max_merit = -1.0;
        max_merit_feature_index = -1;

        for (int i = 0; i < num_features; i++) {
            int contains = 0;
            for (int j = 0; j < current_size; j++) {
                if (s[j] == i) {
                    contains = 1;
                    break;
                }
            }
            if (contains == 1) continue;
            s[current_size] = i;
            float current_merit = calculate_merit(calculate_avg_cf_corr(dataset, s, current_size + 1, c, N, num_features), calculate_avg_ff_corr(dataset, s, current_size + 1, N, num_features), current_size + 1);
            if (current_merit > max_merit) {
                max_merit = current_merit;
                max_merit_feature_index = i;
            }
        }
        if (max_merit_feature_index < 0) {
            printf("\n\nSomething went wrong\n\n");
            break; // Esci se c'è un problema nell'aggiornamento dell'indice
        }
        s[current_size] = max_merit_feature_index;
        current_size++;
    }

    input->out = s;
    input->sc = max_merit;
    */
}

int main(int argc, char** argv) {

    char fname[256];
    char* dsfilename = NULL;
    char* labelsfilename = NULL;
    clock_t t;
    float time;
    
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

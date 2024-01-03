//per compilare 
//gcc cfs32c_senza_commenti.c -o cfs32c_senza_commenti -lm

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
    int* out;
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


// Funzione per calcolare la media di un vettore di valori
float calculate_mean(float *data, int n) {
    float sum = 0.0;
    for (int i = 0; i < n; ++i) {
        sum += data[i];
    }
    float mean = sum / n;
    //printf("Mean: %f\n", mean);
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
    //printf("Standard Deviation: %f\n", std_dev);
    return std_dev;
}

// Funzione per calcolare la correlazione tra una feature e la variabile di classe
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
    //printf("Correlation: %f\n", r_cf);
    return r_cf;
}

float* getColumn(float* matrix, int rows, int cols, int col_index) {
    float* column = malloc(rows * sizeof(float));
    for (int i = 0; i < rows; ++i) {
        column[i] = matrix[i * cols + col_index];
    }
    return column;
}
/*
float* getColumn(float* matrix, int rows, int col_index){
    float* column = malloc(rows * sizeof(float));
    printf("Dimensione dataset: %ld\n",sizeof(matrix));
    printf("rows:%d\n",rows);
    for(int i=0; i<rows; i++){
        printf("i=%d, ",i);
        printf("col index=%d, ",col_index);
        printf("%d\n",i*rows+col_index);
        column[i] = matrix[i*rows + col_index];
    }
    printf("]\n");
    return column;
}
*/
float calculate_avg_cf_corr(float* ds, int* selected_features, int num_chosen_features, float* labels, int N, int d) {
    float total_cf_corr = 0.0;
    // Calcolo delle correlazioni tra feature e variabile di classe per ogni feature selezionata
    for (int a = 0; a < num_chosen_features; ++a) {
        int i=selected_features[a];
        float* column = getColumn(ds, N, d, i);
        float cf_corr = fabsf(calculate_cf_corr(column, labels, N));
        total_cf_corr += cf_corr;
    }
    // Calcolo del valore medio delle correlazioni tra feature selezionate e variabile di classe
    float avg_cf_corr = total_cf_corr / num_chosen_features;
    //printf("Average Correlation: %f\n", avg_cf_corr);
    return avg_cf_corr;
}

//-------------------------------------------------------------------------------------------------

// Funzione per calcolare la correlazione tra due feature
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
    //printf("Correlation between two features: %f\n", r_ff);
    return r_ff;
}

float calculate_avg_ff_corr(float *ds, int* selected_features, int num_chosen_features, int N, int d) {
    float total_ff_corr = 0.0f;
    int num_pairs = num_chosen_features * (num_chosen_features - 1) / 2;
    if(num_chosen_features==1) {
        //float* feature = getColumn(ds, N, d, selected_features[0]);
        //return fabsf(calculate_ff_corr(feature,feature,N));
        return 1.0;
    }
    // Calcolo delle correlazioni tra le feature stesse per ogni coppia di feature selezionate
    /*
    for (int i = 0; i < d; ++i) {
        for (int j = i + 1; j < d; ++j) {
            float ff_corr = calculate_ff_corr(ds + i * N, ds + j * N, N);
            total_ff_corr += ff_corr;
        }
    }*/
    for(int a=0;a<num_chosen_features;a++){
        float* feature_a = getColumn(ds, N, d, selected_features[a]);
        for(int b=a+1;b<num_chosen_features;b++){
            float* feature_b = getColumn(ds, N, d, selected_features[b]);
            total_ff_corr += fabsf(calculate_ff_corr(feature_a, feature_b, N));
        }
    }
    // Calcolo del valore medio delle correlazioni tra le feature stesse
    float avg_ff_corr = total_ff_corr / num_pairs;
    //printf("Average correlation between features: %f\n", avg_ff_corr);
    return avg_ff_corr;
}
//---------------------------------------------------------------

//Funzione per calcolare il merito
float calculate_merit(float avg_cf_corr, float avg_ff_corr, int k) {
    float merit = (k * avg_cf_corr) / sqrtf(k + k * (k - 1) * avg_ff_corr);
    //printf("Calculated merit: %f\n", merit);
    return merit;
}


/*
void cfs(params* input){
    //proviamo ad invocare la funzione da un altro file


    //esempio
    //calculate_correlation(input->ds, input->labels, input->N, input->d);

    printf("\nStampa del dataset\n");
    stampaMatrice(input->ds, input->N, input->d);

    printf("\nStampa labels\n");
    stampaMatrice(input->labels, input->N, 1);
    printf("\n");

    

    // Calcola le medie rfc e rff
    float avg_cf_corr = calculate_avg_cf_corr(input->ds, input->labels, input->N, input->d);

    
    float avg_ff_corr = calculate_avg_ff_corr(input->ds, input->N, input->d);

   


    
    // Calcola il merito
    int num_chosen_features = input->k; // Esempio di numero di feature scelte
    float merit = calculate_merit(avg_cf_corr, avg_ff_corr, num_chosen_features);


}
*/

void cfs(params* input){

    float* dataset = input->ds; //dataset definito sull'insieme di features F
    float* c = input->labels;   //vettore delle etichette
    int k = input->k;   //numero di features da estrarre
    int num_features = input->d;    //numero di colonne del dataset, nonché numero di features presenti
    int N = input->N;   //numero di righe del dataset, nonché dimensione delle singole colonne/features

    int* s = malloc(k * sizeof(int));   //vettore contenente l'insieme S degli indici delle features da estrarre (indici di colonna del dataset)
    int current_size = 0;   //numero di feature estratte ed inserite correntemente in s

    printf("Inizio della funzione cfs.\nk=%d,\n",k);
    printf("num_features=%d\n",num_features);
    printf("N=%d\n",N);

    s[0]=0;

    //stampaMatrice(dataset, N, num_features);
    printf("Valore iniziale per una colonna:%f\n", calculate_avg_ff_corr(dataset, s, current_size+1, N, num_features));

    float max_merit = -1.0;

    while(current_size<k){
        int max_merit_feature_index = -1;
        for(int i=0; i<num_features; i++){
            int contains = 0;
            for(int j=0; j<current_size; j++){
                if(s[j]==i) {
                    contains=1;
                    //printf("\nLa feature %d è già contenuta.\n\n",i);
                    break; //La feature selezionata è già presente nell'insieme S, non va considerata.
                }
            }
            if(contains==1) continue;
            printf("Considero la feature in posizione:%d\n",i);
            s[current_size] = i;
            float current_merit = calculate_merit( calculate_avg_cf_corr(dataset, s, current_size+1, c, N, num_features), calculate_avg_ff_corr(dataset, s, current_size+1, N, num_features), current_size+1 );
            printf("Merito corrente:%f\n",current_merit);
            if(current_merit>max_merit){
                max_merit=current_merit;
                max_merit_feature_index=i;
            }
            
        }
        
        if(max_merit_feature_index<0) printf("\n\nQualcosa è andato storto\n\n");   //debug

        s[current_size] = max_merit_feature_index;
        current_size++;
        printf("----------------------------------------\n");
    }

    input->out = s;
    input->sc = max_merit;

}

int main(int argc, char** argv) {

    printf("inizio del programma\n");
    
    
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
        printf("\tDS: il nome del file ds2 contenente il dataset\n");
        printf("\tLABELS: il nome del file ds2 contenente le etichette\n");
        printf("\tk: numero di features da estrarre\n");
        printf("\nOptions:\n");
        printf("\t-s: modo silenzioso, nessuna stampa, default 0 - false\n");
        printf("\t-d: stampa a video i risultati, default 0 - false\n");
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

    if(!input->silent){
        printf("Dataset file name: '%s'\n", dsfilename);
        printf("Labels file name: '%s'\n", labelsfilename);
        printf("Dataset row number: %d\n", input->N);
        printf("Dataset column number: %d\n", input->d);
        printf("Number of features to extract: %d\n", input->k);
    }

    //prova(input);

    t = clock();
    printf("\nprimo\n%f\n", (float)t);

    cfs(input);
    t = clock() - t;
    printf("\nsecondo\n%f\n", (float)t);
    
    time = ((float)t)/CLOCKS_PER_SEC;
    
    //time = ((float)t);

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

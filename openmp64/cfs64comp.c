/**************************************************************************************
* 
* CdL Magistrale in Ingegneria Informatica
* Corso di Architetture e Programmazione dei Sistemi di Elaborazione - a.a. 2020/21
* 
* Progetto dell'algoritmo Attention Mechanism 221 231 a
* in linguaggio assembly x86-64 + SSE
* 
* Fabrizio Angiulli, aprile 2019
* 
**************************************************************************************/

/*
* 
* Software necessario per l'esecuzione:
* 
*    NASM (www.nasm.us)
*    GCC (gcc.gnu.org)
* 
* entrambi sono disponibili come pacchetti software 
* installabili mediante il packaging tool del sistema 
* operativo; per esempio, su Ubuntu, mediante i comandi:
* 
*    sudo apt-get install nasm
*    sudo apt-get install gcc
* 
* potrebbe essere necessario installare le seguenti librerie:
* 
*    sudo apt-get install lib64gcc-4.8-dev (o altra versione)
*    sudo apt-get install libc6-dev-i386
* 
* Per generare il file eseguibile:
* 
* nasm -f elf64 fss64.nasm && gcc -m64 -msse -O0 -no-pie sseutils64.o fss64.o fss64c.c -o fss64c -lm && ./fss64c $pars
* 
* oppure
* 
* ./runfss64
* 
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <xmmintrin.h>

#define	type		double
#define	MATRIX		type*
#define	VECTOR		type*

typedef struct {
	MATRIX ds; 		// dataset
	VECTOR labels; 	// etichette
	int* out;		// vettore contenente risultato dim=k
	type sc;		// score dell'insieme di features risultato
	int k;			// numero di features da estrarre
	int N;			// numero di righe del dataset
	int d;			// numero di colonne/feature del dataset
	int display;
	int silent;
} params;

/*
* 
*	Le funzioni sono state scritte assumento che le matrici siano memorizzate 
* 	mediante un array (float*), in modo da occupare un unico blocco
* 	di memoria, ma a scelta del candidato possono essere 
* 	memorizzate mediante array di array (float**).
* 
* 	In entrambi i casi il candidato dovr� inoltre scegliere se memorizzare le
* 	matrici per righe (row-major order) o per colonne (column major-order).
*
* 	L'assunzione corrente � che le matrici siano in row-major order.
* 
*/

void* get_block(int size, int elements) { 
	return _mm_malloc(elements*size,32); 
}

void free_block(void* p) { 
	_mm_free(p);
}

MATRIX alloc_matrix(int rows, int cols) {
	return (MATRIX) get_block(sizeof(type),rows*cols);
}

int* alloc_int_matrix(int rows, int cols) {
	return (int*) get_block(sizeof(int),rows*cols);
}

void dealloc_matrix(void* mat) {
	free_block(mat);
}

/*
* 
* 	load_data
* 	=========
* 
*	Legge da file una matrice di N righe
* 	e M colonne e la memorizza in un array lineare in row-major order
* 
* 	Codifica del file:
* 	primi 4 byte: numero di colonne (N) --> numero intero
* 	successivi 4 byte: numero di righe (M) --> numero intero
* 	successivi N*M*8 byte: matrix data in row-major order --> numeri floating-point a precisione doppia
* 
*****************************************************************************
*	Se lo si ritiene opportuno, � possibile cambiare la codifica in memoria
* 	della matrice. 
*****************************************************************************
* 
*/
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
	
	MATRIX data = alloc_matrix(rows,cols);
	status = fread(data, sizeof(type), rows*cols, fp);
	fclose(fp);
	
	*n = rows;
	*k = cols;
	
	return data;
}

/*
* 	save_data
* 	=========
* 
*	Salva su file un array lineare in row-major order
*	come matrice di N righe e M colonne
* 
* 	Codifica del file:
* 	primi 4 byte: numero di colonne (N) --> numero intero a 32 bit
* 	successivi 4 byte: numero di righe (M) --> numero intero a 32 bit
* 	successivi N*M*8 byte: matrix data in row-major order --> numeri interi o floating-point a precisione doppia
*/
void save_data(char* filename, void* X, int n, int k) {
	FILE* fp;
	int i;
	fp = fopen(filename, "wb");
	if(X != NULL){
		fwrite(&k, 4, 1, fp);
		fwrite(&n, 4, 1, fp);
		for (i = 0; i < n; i++) {
			fwrite(X, sizeof(type), k, fp);
			//printf("%i %i\n", ((int*)X)[0], ((int*)X)[1]);
			X += sizeof(type)*k;
		}
	}
	else{
		int x = 0;
		fwrite(&x, 4, 1, fp);
		fwrite(&x, 4, 1, fp);
	}
	fclose(fp);
}

/*
* 	save_out
* 	=========
* 
*	Salva su file un array lineare composto da k+1 elementi.
* 
* 	Codifica del file:
* 	primi 4 byte: contenenti il numero di elementi (k+1)		--> numero intero a 32 bit
* 	successivi 4 byte: numero di righe (1) 						--> numero intero a 32 bit
* 	successivi byte: elementi del vettore 		--> 1 numero floating-point a precisione doppia e k interi
*/
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
		//printf("%i %i\n", ((int*)X)[0], ((int*)X)[1]);
	}
	fclose(fp);
}

extern double calcola_dev_std(double media, double* data, int n);
extern double calcola_rff(double* feature_x, double mean_x, double* feature_y, double mean_y, int n);
extern void get_column(double* risultato, double* dataset, int rows, int columns, int choosed_col);
extern double calcola_media(double* array, int n);

double calculate_cf_corr(double *feature, double *labels, int n) {
    //DEPRECATED
    double mean_0 = 0.0, mean_1 = 0.0;
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

    double std_dev;
    double media = calcola_media(feature, n);
    std_dev = calcola_dev_std(media, feature, n);

    double r_cf = ((mean_0 - mean_1) / std_dev) * sqrt((count_0 * count_1) / pow(n, 2));
    return r_cf;
}

double calculate_avg_cf_corr(double* ds, int* selected_features, int num_chosen_features, double* labels, int N, int d) {
    double total_cf_corr = 0.0;
	//#pragma omp parallel for -> segmentation fault.
    for (int a = 0; a < num_chosen_features; ++a) {
        int i=selected_features[a];
        double* column = get_block(N,sizeof(double));
        get_column(column, ds, N, d, i);
        double cf_corr = fabs(calculate_cf_corr(column, labels, N));
        free_block(column);
        total_cf_corr += cf_corr;
    }
    double avg_cf_corr = total_cf_corr  / num_chosen_features;
    return avg_cf_corr;
}

double calculate_avg_ff_corr(double *ds, int* selected_features, int num_chosen_features, int N, int d) {
    double total_ff_corr = 0.0f;
    int num_pairs = num_chosen_features * (num_chosen_features - 1) / 2;
    if(num_chosen_features==1) {
        return 1.0;
    }
	//#pragma omp parallel for -> segmentation fault.
    for(int a=0;a<num_chosen_features;a++){
        double* feature_a = get_block(N, sizeof(double));
        get_column(feature_a, ds, N, d, selected_features[a]);
		double media_a = calcola_media(feature_a, N);
		double* feature_b = get_block(N, sizeof(double));
        for(int b=a+1;b<num_chosen_features;b++){
            get_column(feature_b, ds, N, d, selected_features[b]);
            double rff;
            double media_b;
			media_b = calcola_media(feature_b, N);
            rff = calcola_rff(feature_a, media_a, feature_b, media_b, N);
            total_ff_corr += fabs(rff);
        }
        free_block(feature_a);
		free_block(feature_b);
    }
    double avg_ff_corr = total_ff_corr / num_pairs;
    return avg_ff_corr;
}

double calcola_merito(double* dataset, int* selected_features, int num_chosen_features,double* labels, int N, int d){
	double toRet= (num_chosen_features * calculate_avg_cf_corr(dataset, selected_features, num_chosen_features, labels, N, d)) / sqrt( num_chosen_features + num_chosen_features*(num_chosen_features-1) * calculate_avg_ff_corr(dataset, selected_features, num_chosen_features, N, d) );
	return toRet;
}
//extern void prova(params* input);

//extern double calcola_cfs(double* ds, double* labels, int k, int cols, int rows, int* out);

double calcola_cfs(double* dataset, double* c, int k, int num_features, int N, int* s){
	double max_merit;
	int current_size = 0;

    while(current_size<k){
		max_merit = -1.0;
        int max_merit_feature_index = -1;
		#pragma omp parallel for
        for(int i=0; i<num_features; i++){
            int contains = 0;
			//#pragma omp parallel for
            for(int j=0; j<current_size; j++){
                if(s[j]==i) {
                    contains=1;
                    //printf("\nLa feature %d è già contenuta.\n\n",i);
                    break; //La feature selezionata è già presente nell'insieme S, non va considerata.
                }
            }
            if(contains==1) continue;
            //printf("Considero la feature in posizione:%d\n",i);
            s[current_size] = i;
            double current_merit = calcola_merito(dataset, s, current_size+1, c, N, num_features);
            //printf("Merito corrente:%f\n",current_merit);
            if(current_merit>max_merit){
                max_merit=current_merit;
                max_merit_feature_index=i;
            }
            
        }
        
        if(max_merit_feature_index<0) printf("\n\nQualcosa è andato storto%d\n\n",current_size);   //debug

        s[current_size] = max_merit_feature_index;
        current_size++;
        //printf("----------------------------------------\n");
    }
	return max_merit;
}


void cfs(params* input){
	double* dataset = input->ds;
    double* c = input->labels;
    int k = input->k;
    int num_features = input->d;
    int N = input->N;
    input->sc = calcola_cfs(dataset, c, k, num_features, N, input->out);
}

int main(int argc, char** argv) {

	char fname[256];
	char* dsfilename = NULL;
	char* labelsfilename = NULL;
	clock_t t;
	float time;
	
	//
	// Imposta i valori di default dei parametri
	//

	params* input = malloc(sizeof(params));
	//params* input = get_block(1,sizeof(params));

	input->ds = NULL;
	input->labels = NULL;
	input->k = -1;
	input->sc = -1;

	input->silent = 0;
	input->display = 0;

	printf("%li\n", sizeof(int));

	//
	// Visualizza la sintassi del passaggio dei parametri da riga comandi
	//

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

	//
	// Legge i valori dei parametri da riga comandi
	//

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

	//
	// Legge i dati e verifica la correttezza dei parametri
	//

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
	//printf("indirizzo: %p\n",input->out);
	//printf("indirizzo ds: %p\n",input->ds);

	//
	// Visualizza il valore dei parametri
	//

	if(!input->silent){
		printf("Dataset file name: '%s'\n", dsfilename);
		printf("Labels file name: '%s'\n", labelsfilename);
		printf("Dataset row number: %d\n", input->N);
		printf("Dataset column number: %d\n", input->d);
		printf("Number of features to extract: %d\n", input->k);
	}

	// COMMENTARE QUESTA RIGA!
	//prova(input);
	//

	//
	// Correlation Features Selection
	//

	t = clock();
	cfs(input);
	t = clock() - t;
	time = ((float)t)/CLOCKS_PER_SEC;

	if(!input->silent)
		printf("CFS time = %.3f secs\n", time);
	else
		printf("%.3f\n", time);

	//
	// Salva il risultato
	//
	sprintf(fname, "out64_%d_%d_%d.ds2", input->N, input->d, input->k);
	save_out(fname, input->sc, input->out, input->k);
	if(input->display){
		if(input->out == NULL)
			printf("out: NULL\n");
		else{
			int i,j;
			printf("sc: %lf, out: [", input->sc);
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
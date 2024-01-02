// Funzione per calcolare la media di un vettore di valori
float calculate_mean(float *data, int n) {
    float sum = 0.0;
    for (int i = 0; i < n; ++i) {
        sum += data[i];
    }
    float mean = sum / n;
    printf("Mean: %f\n", mean);
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
    printf("Standard Deviation: %f\n", std_dev);
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
    printf("Correlation: %f\n", r_cf);
    return r_cf;
}

float* getColumn(float* matrix, int rows, int cols, int col_index) {
    float* column = malloc(rows * sizeof(float));
    for (int i = 0; i < rows; ++i) {
        column[i] = matrix[i * cols + col_index];
    }
    return column;
}

float calculate_avg_cf_corr(float* ds, float* labels, int N, int d) {
    float total_cf_corr = 0.0;
    // Calcolo delle correlazioni tra feature e variabile di classe per ogni feature
    for (int i = 0; i < d; ++i) {
        float* column = getColumn(ds, N, d, i);
        float cf_corr = calculate_cf_corr(column, labels, N);
        total_cf_corr += cf_corr;
    }
    // Calcolo del valore medio delle correlazioni tra feature e variabile di classe
    float avg_cf_corr = total_cf_corr / d;
    printf("Average Correlation: %f\n", avg_cf_corr);
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
    printf("Correlation between two features: %f\n", r_ff);
    return r_ff;
}

float calculate_avg_ff_corr(float *ds, int N, int d) {
    float total_ff_corr = 0.0f;
    int num_pairs = d * (d - 1) / 2;

    // Calcolo delle correlazioni tra le feature stesse per ogni coppia di feature
    for (int i = 0; i < d; ++i) {
        for (int j = i + 1; j < d; ++j) {
            float ff_corr = calculate_ff_corr(ds + i * N, ds + j * N, N);
            total_ff_corr += ff_corr;
        }
    }
    // Calcolo del valore medio delle correlazioni tra le feature stesse
    float avg_ff_corr = total_ff_corr / num_pairs;
    printf("Average correlation between features: %f\n", avg_ff_corr);
    return avg_ff_corr;
}
//---------------------------------------------------------------

//Funzione per calcolare il merito
float calculate_merit(float avg_cf_corr, float avg_ff_corr, int k) {
    float merit = (k * fabsf(avg_cf_corr)) / sqrtf(k + k * (k - 1) * fabsf(avg_ff_corr));
    printf("Calculated merit: %f\n", merit);
    return merit;
}


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
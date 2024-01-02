//usato per creare un dataset di prova (row-major-order)

#include <stdio.h>

void write_binary_file(const char* filename) {
    FILE* fp;
    int rows = 3;
    int cols = 4;
    float data[3][4] = {
        {1.1f, 2.2f, 3.3f, 4.4f},
        {5.5f, 6.6f, 7.7f, 8.8f},
        {9.9f, 10.0f, 11.1f, 12.2f}
    };

    // Apri il file in modalità binaria
    fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("Impossibile aprire il file %s per la scrittura.\n", filename);
        return;
    }

    // Scrivi il numero di colonne (int) nel file
    fwrite(&cols, sizeof(int), 1, fp);

    // Scrivi il numero di righe (int) nel file
    fwrite(&rows, sizeof(int), 1, fp);

    // Scrivi i dati nel file
    int i, j;
    for (i = 0; i < rows; ++i) {
        for (j = 0; j < cols; ++j) {
            fwrite(&data[i][j], sizeof(float), 1, fp);
        }
    }

    // Chiudi il file dopo aver scritto i dati
    fclose(fp);

    printf("File %s scritto correttamente.\n", filename);
}

int main() {
    const char* nome_file = "esempio_dataset.ds2";
    write_binary_file(nome_file);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>

extern void calcola_media(float* risultato, int n, float* array);

int main(){
    float v[] = {1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0};
    float risultato;// = malloc(sizeof(float));
    calcola_media(&risultato,8,v);
    printf("Il valore medio del vettore è %f\n", risultato);
}
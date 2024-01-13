#include <stdio.h>
#include <stdlib.h>

extern void calcola_media(float* risultato, float* array, int n);

int main(){
    float v[] = {1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,11.0};
    float risultato;// = malloc(sizeof(float));
    calcola_media(&risultato,v,11);
    printf("Il valore medio del vettore è %f\n", risultato);
}
#include "io_arquivos.h"
#include <stdio.h>
#include <stdlib.h>

int *ler_inteiros_arquivo(const char *caminho, size_t *out_n) {
    *out_n = 0;

    FILE *f = fopen(caminho, "r");
    if (!f) return NULL;

    // Conta quantos números existem no arquivo
    long long tmp;
    size_t n = 0;
    while (fscanf(f, "%lld", &tmp) == 1) n++;

    if (n == 0) {
        fclose(f);
        return NULL;
    }

    rewind(f);

    int *vetor = (int *)malloc(n * sizeof(int));
    if (!vetor) {
        fclose(f);
        return NULL;
    }

    for (size_t i = 0; i < n; i++) {
        if (fscanf(f, "%d", &vetor[i]) != 1) {
            free(vetor);
            fclose(f);
            return NULL;
        }
    }

    fclose(f);
    *out_n = n;
    return vetor;
}

int escrever_inteiros_arquivo(const char *caminho, const int *vetor, size_t n) {
    FILE *f = fopen(caminho, "w");
    if (!f) return -1;

    for (size_t i = 0; i < n; i++) {
        fprintf(f, "%d\n", vetor[i]);
    }

    fclose(f);
    return 0;
}
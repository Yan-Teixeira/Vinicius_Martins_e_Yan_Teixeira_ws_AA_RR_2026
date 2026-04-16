#include "counting_sort.h"
#include <stdlib.h>
#include <string.h>

int counting_sort(int *vetor, size_t n, int valor_maximo) {
    if (n == 0) return 0;
    if (valor_maximo < 0) return -1;

    size_t k = (size_t)valor_maximo + 1;

    int *contagem = (int *)malloc(k * sizeof(int));
    if (!contagem) return -1;
    memset(contagem, 0, k * sizeof(int));

    for (size_t i = 0; i < n; i++) {
        int v = vetor[i];
        contagem[v]++;
    }

    size_t pos = 0;
    for (int v = 0; v <= valor_maximo; v++) {
        int c = contagem[v];
        for (int j = 0; j < c; j++) {
            vetor[pos++] = v;
        }
    }

    free(contagem);
    return 0;
}
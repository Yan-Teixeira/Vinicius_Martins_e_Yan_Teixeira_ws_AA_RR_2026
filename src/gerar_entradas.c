#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

#include "config.h"

static void garantir_diretorio(const char *pasta) {
    mkdir(pasta, 0755); // se já existir, tudo bem
}

int main(void) {
    garantir_diretorio("input");

    srand(42);

    for (size_t t = 0; t < QTD_TAMANHOS; t++) {
        size_t n = TAMANHOS[t];

        char nome[256];
        snprintf(nome, sizeof(nome), "input/entrada_%zu.txt", n);

        FILE *f = fopen(nome, "w");
        if (!f) {
            perror("fopen");
            return 1;
        }

        for (size_t i = 0; i < n; i++) {
            int v = rand() % (VALOR_MAXIMO + 1); // 0..VALOR_MAXIMO
            fprintf(f, "%d\n", v);
        }

        fclose(f);
        printf("Gerado: %s (n=%zu, max=%d)\n", nome, n, VALOR_MAXIMO);
    }

    return 0;
}
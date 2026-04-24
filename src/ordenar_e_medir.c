#define _POSIX_C_SOURCE 200809L

#include "counting_sort.h"
#include "io_arquivos.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/stat.h>

static void garantir_diretorio(const char *pasta) {
    mkdir(pasta, 0755);
}

static double tempo_agora_s(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static long memoria_pico_kb(void) {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss; // KB no Linux
}

static int cmp_double(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    return (da > db) - (da < db);
}

static void imprimir_lista_tempos(const double tempos[], int n) {
    for (int i = 0; i < n; i++) {
        printf("    exec %2d: %.9f s\n", i + 1, tempos[i]);
    }
}

// Média aparada: ordena os 13 tempos, descarta menor e maior, média dos 11 restantes.
// Também retorna (via ponteiros) quais foram o min e o max descartados.
static double media_cortada_13(const double tempos_in[EXECUCOES], double *out_min, double *out_max) {
    double tempos[EXECUCOES];
    for (int i = 0; i < EXECUCOES; i++) tempos[i] = tempos_in[i];

    qsort(tempos, EXECUCOES, sizeof(double), cmp_double);

    if (out_min) *out_min = tempos[0];
    if (out_max) *out_max = tempos[EXECUCOES - 1];

    double soma = 0.0;
    for (int i = 1; i <= 11; i++) {
        soma += tempos[i];
    }
    return soma / 11.0;
}

// Mede o tempo de I/O da saída: fopen + fprintf (n linhas) + fclose.
// Retorna tempo em segundos; retorna <0 em caso de erro.
static double escrever_inteiros_arquivo_tempo(const char *caminho, const int *vetor, size_t n) {
    double t0 = tempo_agora_s();

    FILE *f = fopen(caminho, "w");
    if (!f) {
        perror("fopen");
        return -1.0;
    }

    for (size_t i = 0; i < n; i++) {
        fprintf(f, "%d\n", vetor[i]);
    }

    fclose(f);

    double t1 = tempo_agora_s();
    return (t1 - t0);
}

int main(void) {
    garantir_diretorio("output");
    garantir_diretorio("metrics");

    FILE *csv = fopen("metrics/metricas.csv", "w");
    if (!csv) {
        perror("fopen metrics/metricas.csv");
        return 1;
    }

    fprintf(csv, "n,tempo_medio_s,tempo_min_s,tempo_max_s,memoria_pico_kb,tempo_io_saida_s\n");

    for (size_t t = 0; t < QTD_TAMANHOS; t++) {
        size_t n_esperado = TAMANHOS[t];

        char caminho_entrada[256];
        snprintf(caminho_entrada, sizeof(caminho_entrada),
                 "input/entrada_%zu.txt", n_esperado);

        size_t n;
        int *original = ler_inteiros_arquivo(caminho_entrada, &n);
        if (!original) {
            fprintf(stderr, "Erro ao ler %s\n", caminho_entrada);
            fclose(csv);
            return 1;
        }

        printf("\n==============================\n");
        printf("Arquivo: %s\n", caminho_entrada);
        printf("n = %zu | valor_maximo = %d\n", n, VALOR_MAXIMO);
        printf("==============================\n");

        double tempos[EXECUCOES];

        // 13 execuções: mede apenas o tempo do counting_sort (sem I/O)
        for (int e = 0; e < EXECUCOES; e++) {
            int *copia = (int *)malloc(n * sizeof(int));
            if (!copia) {
                fprintf(stderr, "Sem memória para n=%zu\n", n);
                free(original);
                fclose(csv);
                return 1;
            }
            memcpy(copia, original, n * sizeof(int));

            double t0 = tempo_agora_s();
            int ok = counting_sort(copia, n, VALOR_MAXIMO);
            double t1 = tempo_agora_s();

            free(copia);

            if (ok != 0) {
                fprintf(stderr, "counting_sort falhou (n=%zu)\n", n);
                free(original);
                fclose(csv);
                return 1;
            }

            tempos[e] = (t1 - t0);
        }

        printf("Tempos coletados (%d execuções):\n", EXECUCOES);
        imprimir_lista_tempos(tempos, EXECUCOES);

        double tmin = 0.0, tmax = 0.0;
        double tempo_medio = media_cortada_13(tempos, &tmin, &tmax);

        printf("Descartados: min=%.9f s | max=%.9f s\n", tmin, tmax);
        printf("Tempo médio (11 valores): %.9f s\n", tempo_medio);

        long mem_kb = memoria_pico_kb();
        printf("Memória pico (processo): %ld KB\n", mem_kb);

        // Gera saída ordenada (artefato) uma vez
        int *ordenado = (int *)malloc(n * sizeof(int));
        if (!ordenado) {
            fprintf(stderr, "Sem memória para saída (n=%zu)\n", n);
            free(original);
            fclose(csv);
            return 1;
        }
        memcpy(ordenado, original, n * sizeof(int));

        if (counting_sort(ordenado, n, VALOR_MAXIMO) != 0) {
            fprintf(stderr, "counting_sort falhou na geração do arquivo de saída (n=%zu)\n", n);
            free(ordenado);
            free(original);
            fclose(csv);
            return 1;
        }

        char caminho_saida[256];
        snprintf(caminho_saida, sizeof(caminho_saida),
                 "output/saida_%zu_ordenado.txt", n);

        // Mede tempo de I/O da saída (abrir + escrever + fechar)
        double tempo_io_saida = escrever_inteiros_arquivo_tempo(caminho_saida, ordenado, n);
        if (tempo_io_saida < 0.0) {
            fprintf(stderr, "Erro ao escrever %s\n", caminho_saida);
            free(ordenado);
            free(original);
            fclose(csv);
            return 1;
        }

        printf("Saída: %s\n", caminho_saida);
        printf("Tempo I/O saída (abrir+escrever+fechar): %.6f s\n", tempo_io_saida);

        // Salva no CSV
        fprintf(csv, "%zu,%.12f,%.12f,%.12f,%ld,%.12f\n",
                n, tempo_medio, tmin, tmax, mem_kb, tempo_io_saida);

        free(ordenado);
        free(original);
    }

    fclose(csv);
    printf("\nMétricas salvas em: metrics/metricas.csv\n");
    return 0;
}
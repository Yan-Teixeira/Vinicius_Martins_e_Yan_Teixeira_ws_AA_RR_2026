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
    // "pico" aqui é o maior RSS (memória residente) que o processo atingiu até agora.
    // Atenção na portabilidade:
    // - Linux: ru_maxrss vem em KB
    // - macOS: ru_maxrss geralmente vem em BYTES
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);

#if defined(__APPLE__) && defined(__MACH__)
    // macOS: converte bytes -> KB
    return (long)(usage.ru_maxrss / 1024);
#else
    // Linux: já está em KB
    return (long)usage.ru_maxrss;
#endif
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

// média aparada: ordena os 13 tempos, descarta menor e maior, média dos 11 restantes.
// a ideia é reduzir o impacto de outliers (scheduler, cache frio, etc.).
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

// mede o tempo de I/O da saída: fopen + fprintf (n linhas) + fclose.
// retorna tempo em segundos; retorna <0 em caso de erro.
static double escrever_inteiros_arquivo_tempo(const char *caminho, const int *vetor, size_t n) {
    double t0 = tempo_agora_s();

    FILE *f = fopen(caminho, "w");
    if (!f) {
        perror("fopen");
        return -1.0;
    }

    // escrever em texto (fprintf) é bem mais caro do que escrever binário,
    // então em n grande esse passo costuma dominar.
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

    // CSV com todas as métricas que estamos acompanhando (inclui entrada/saída e tempo total).
    fprintf(csv,
            "n,tempo_medio_s,tempo_min_s,tempo_max_s,memoria_pico_kb,tempo_io_entrada_s,tempo_io_saida_s,tempo_total_arquivo_s\n");

    for (size_t t = 0; t < QTD_TAMANHOS; t++) {
        size_t n_esperado = TAMANHOS[t];

        char caminho_entrada[256];
        snprintf(caminho_entrada, sizeof(caminho_entrada),
                 "input/entrada_%zu.txt", n_esperado);

        // tempo total "end-to-end" por arquivo: do começo da leitura até terminar de escrever + liberar memória.
        double t0_total = tempo_agora_s();

        // mede I/O de entrada (abrir + ler + parse + fechar).
        // "parse" converte texto -> int enquanto lê o arquivo.
        double t0_in = tempo_agora_s();
        size_t n;
        int *original = ler_inteiros_arquivo(caminho_entrada, &n);
        double t1_in = tempo_agora_s();
        double tempo_io_entrada = (t1_in - t0_in);

        if (!original) {
            fprintf(stderr, "Erro ao ler %s\n", caminho_entrada);
            fclose(csv);
            return 1;
        }

        printf("\n==============================\n");
        printf("Arquivo: %s\n", caminho_entrada);
        printf("n = %zu | valor_maximo = %d\n", n, VALOR_MAXIMO);
        printf("==============================\n");
        printf("Tempo I/O entrada (abrir+ler+parse+fechar): %.6f s\n", tempo_io_entrada);

        double tempos[EXECUCOES];

        // benchmark do algoritmo: mede só o trecho do counting_sort (sem incluir I/O).
        // cada execução cria uma cópia do vetor original pra manter a mesma entrada.
        for (int e = 0; e < EXECUCOES; e++) {
            int *copia = (int *)malloc(n * sizeof(int));
            if (!copia) {
                fprintf(stderr, "Sem memória para n=%zu\n", n);
                free(original);
                fclose(csv);
                return 1;
            }
            memcpy(copia, original, n * sizeof(int));

            // aqui começa a contagem do "tempo do algoritmo"
            double t0 = tempo_agora_s();
            int ok = counting_sort(copia, n, VALOR_MAXIMO);
            double t1 = tempo_agora_s();
            // aqui termina o "tempo do algoritmo".

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

        // gera o arquivo de saída ordenado só uma vez (separado do benchmark das 13 execuções).
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

        // mede I/O de saída (abrir + escrever + fechar).
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

        free(ordenado);
        free(original);

        // fecha o tempo total do arquivo.
        double t1_total = tempo_agora_s();
        double tempo_total_arquivo = (t1_total - t0_total);
        printf("Tempo TOTAL (arquivo): %.6f s\n", tempo_total_arquivo);

        // salva no CSV
        fprintf(csv, "%zu,%.12f,%.12f,%.12f,%ld,%.12f,%.12f,%.12f\n",
                n, tempo_medio, tmin, tmax, mem_kb, tempo_io_entrada, tempo_io_saida, tempo_total_arquivo);
    }

    fclose(csv);
    printf("\nMétricas salvas em: metrics/metricas.csv\n");
    return 0;
}
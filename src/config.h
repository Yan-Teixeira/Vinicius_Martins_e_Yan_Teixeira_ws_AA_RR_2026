// src/config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>

/* Intervalo dos valores gerados: 0..VALOR_MAXIMO */
#define VALOR_MAXIMO 1000000

/* Tamanhos dos arquivos gerados */
static const size_t TAMANHOS[] = { 10000, 100000, 1000000, 10000000, 100000000 };
static const size_t QTD_TAMANHOS = sizeof(TAMANHOS) / sizeof(TAMANHOS[0]);

/* Protocolo do experimento */
#define EXECUCOES 13

#endif
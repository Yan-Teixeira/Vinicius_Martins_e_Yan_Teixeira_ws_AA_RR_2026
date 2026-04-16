#ifndef IO_ARQUIVOS_H
#define IO_ARQUIVOS_H

#include <stddef.h>

int *ler_inteiros_arquivo(const char *caminho, size_t *out_n);
int escrever_inteiros_arquivo(const char *caminho, const int *vetor, size_t n);

#endif
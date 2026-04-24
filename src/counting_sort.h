#ifndef COUNTING_SORT_H
#define COUNTING_SORT_H

#include <stddef.h>

/*
 * Counting Sort para inteiros naturais.
 * Pré-condição: todos os valores do vetor devem estar no intervalo [0..valor_maximo].
 * Retorna 0 se ok; -1 se ocorrer erro (ex.: falta de memória).
 */
int counting_sort(int *vetor, size_t n, int valor_maximo);

#endif
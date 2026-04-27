CC = gcc
CFLAGS = -O2 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  CFLAGS += -D_DARWIN_C_SOURCE
endif

BIN_DIR = bin
GERADOR = $(BIN_DIR)/gerar_entradas
BENCH   = $(BIN_DIR)/ordenar_e_medir

PYTHON  = python3
PLOT    = plotar_grafico.py

all: $(GERADOR) $(BENCH)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(GERADOR): src/gerar_entradas.c src/config.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ src/gerar_entradas.c

# config.h entra só como dependência do make (pra rebuild), não como argumento do gcc
$(BENCH): src/ordenar_e_medir.c src/counting_sort.c src/io_arquivos.c src/config.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ src/ordenar_e_medir.c src/counting_sort.c src/io_arquivos.c

gerar: $(GERADOR)
	./$(GERADOR)

medir: $(BENCH)
	./$(BENCH)

graficos:
	$(PYTHON) $(PLOT)

rodar: all gerar medir graficos

limpar:
	rm -rf $(BIN_DIR) input/* output/* metrics/*

.PHONY: all gerar medir graficos rodar limpar
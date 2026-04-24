# 📊 Análise de Complexidade e Performance de Algoritmos

Este repositório contém experimentos empíricos e uma análise matemática de algoritmos em linguagem C.  
O trabalho faz parte da disciplina **Análise de Algoritmos (DCC 606)** na **Universidade Federal de Roraima (UFRR)**.

O objetivo é:
- entender o impacto da **complexidade computacional** no tempo de execução;
- gerar um **gráfico de linha** (tempo de execução × tamanho da entrada);
- analisar a **tendência de comportamento assintótico**;
- apresentar um algoritmo eficiente (em termos de complexidade) e discutir os gargalos práticos (CPU vs I/O).

---

## 📌 1. Análise Teórica e Função de Custo

### Algoritmo analisado: **Counting Sort**

O Counting Sort é um algoritmo de ordenação **não-comparativo**, adequado quando os valores estão em um intervalo conhecido **0..k**.

Ele funciona em três etapas principais:
1. cria um vetor `contagem[0..k]` inicializado com zero;
2. percorre o vetor de entrada e incrementa `contagem[valor]`;
3. reconstrói o vetor ordenado percorrendo `contagem`.

### Função de custo (pior caso)

Seja:
- `n` = quantidade de elementos a ordenar
- `k` = valor máximo possível (range de valores)

No pior caso:

- Inicialização do vetor de contagem: **O(k)**
- Contagem das ocorrências (varre entrada): **O(n)**
- Reconstrução do vetor ordenado (varre `contagem`): **O(n + k)** (na prática, depende do método de reconstrução; aqui o custo é dominado por percorrer `k` e preencher `n` elementos)

✅ **Tempo:** **T(n, k) = O(n + k)**  
✅ **Espaço extra:** **S(k) = O(k)** (vetor de contagem)

> Quando `k` é constante (fixo no experimento), o comportamento esperado fica próximo de **O(n)**.

---

## 💻 2. Algoritmo Proposto (Código em C)

A implementação está em:
- `src/counting_sort.c` / `src/counting_sort.h`

A ideia central:
- usa um vetor auxiliar de contagem para registrar frequências;
- reescreve a saída em ordem crescente com base nessas frequências.

---

## 📈 3. Experimentação e Tempo de Execução

### Entrada dos testes
Os arquivos de entrada são gerados automaticamente em `input/entrada_<n>.txt` com valores aleatórios no intervalo `0..VALOR_MAXIMO`, definido em `src/config.h`.

### Metodologia de medição (robustez)
Para cada tamanho `n`:
- o algoritmo é executado **13 vezes**;
- os tempos são ordenados;
- o menor e o maior tempo são descartados;
- calcula-se a **média dos 11 valores restantes** (média aparada), reduzindo influência de outliers (cache, escalonamento do SO, page faults etc.).

### O que é medido?
O programa `ordenar_e_medir` registra:

- **Tempo de ordenação (CPU/RAM):** mede *somente* o trecho do `counting_sort`  
  (`clock_gettime(CLOCK_MONOTONIC)`).

- **Tempo de I/O da saída:** mede **abrir + escrever + fechar** o arquivo ordenado final  
  (isso é separado porque para entradas grandes o gargalo costuma ser disco, não CPU).

- **Memória pico:** `getrusage(ru_maxrss)` (KB).

Os resultados são salvos em:
- `metrics/metricas.csv`

E os gráficos são gerados em:
- `metrics/grafico_tempo.png`
- `metrics/grafico_io_saida.png`
- `metrics/grafico_memoria.png`

> Observação importante: o tempo total de execução do programa inclui leitura/escrita de arquivos (I/O), que pode ser muito maior do que o tempo de ordenação em si.

---

## 🚀 4. Proposta de Otimização

Como o Counting Sort já é **O(n + k)**, a maior otimização prática observada no experimento é reduzir gargalos de **I/O**.

Sugestões:
- Evitar `fprintf`/`fscanf` linha por linha para arquivos gigantes (muito overhead).
- Usar escrita/leituras em buffer (`fwrite`/`fread`) ou parsing otimizado.
- (Avançado) usar `mmap` para leitura rápida.
- Em cenários reais, armazenar dados em formato binário pode reduzir drasticamente o custo de I/O.

> Ou seja: para `n` muito grande, frequentemente o tempo de execução é dominado por disco/rede, não pelo algoritmo.

---

## 🛠️ 5. Como Compilar e Executar

### Requisitos
- Linux (recomendado) **ou Windows com WSL**
- `gcc` e `make`
- `python3` + `matplotlib` (para gerar os gráficos)

No Ubuntu/Debian:
```bash
sudo apt update
sudo apt install -y build-essential python3 python3-matplotlib
```

### Compilar
```bash
make
```

### Limpar artefatos gerados
```bash
make limpar
```

### Gerar entradas
```bash
make gerar
```

### Rodar medições
```bash
make medir
```

### Gerar gráficos
```bash
make graficos
```

### Rodar tudo (pipeline completo)
```bash
make rodar
```

ou:
```bash
./scripts/rodar_tudo.sh
```

---

## 📂 Estrutura do projeto

- `src/` — código-fonte C
- `input/` — entradas geradas automaticamente
- `output/` — saídas ordenadas
- `metrics/` — CSV e gráficos
- `scripts/` — scripts auxiliares
- `plotar_grafico.py` — gera gráficos a partir do CSV
- `Makefile` — automação de build e execução

---

## ✅ Observação sobre compatibilidade
Este projeto utiliza APIs POSIX (ex.: `clock_gettime`, `getrusage`), então é voltado para:
- **Linux**
- **WSL (Windows Subsystem for Linux)**

Rodar em Windows sem WSL pode exigir adaptações.

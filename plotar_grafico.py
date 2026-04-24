import matplotlib
matplotlib.use("Agg")

import csv
import matplotlib.pyplot as plt

CAMINHO_CSV = "metrics/metricas.csv"

ns = []
tempo_medio = []
tempo_min = []
tempo_max = []
mem_kb = []
tempo_io_saida = []

with open(CAMINHO_CSV, "r", newline="") as f:
    leitor = csv.DictReader(f)
    for row in leitor:
        ns.append(int(row["n"]))
        tempo_medio.append(float(row["tempo_medio_s"]))
        tempo_min.append(float(row["tempo_min_s"]))
        tempo_max.append(float(row["tempo_max_s"]))
        mem_kb.append(int(row["memoria_pico_kb"]))
        # Nova coluna (I/O da escrita do arquivo ordenado)
        tempo_io_saida.append(float(row["tempo_io_saida_s"]))

# --- Gráfico 1: Tempo de ordenação (média) + faixa min/max ---
plt.figure(figsize=(8, 5))
plt.plot(ns, tempo_medio, marker="o", linewidth=2, label="tempo médio (11 exec.)")
plt.fill_between(ns, tempo_min, tempo_max, alpha=0.2, label="faixa (min..max)")
plt.title("Counting Sort - Tempo de ordenação vs n")
plt.xlabel("n (quantidade de números)")
plt.ylabel("Tempo (s)")
plt.grid(True, alpha=0.3)
plt.xscale("log")
plt.yscale("log")
plt.legend()
plt.tight_layout()
plt.savefig("metrics/grafico_tempo.png", dpi=200)
plt.close()

# --- Gráfico 2: Tempo de I/O da saída ---
plt.figure(figsize=(8, 5))
plt.plot(ns, tempo_io_saida, marker="o", linewidth=2, color="darkorange")
plt.title("Counting Sort - Tempo de escrita do arquivo de saída vs n")
plt.xlabel("n (quantidade de números)")
plt.ylabel("Tempo I/O saída (s) [fopen + escrever + fclose]")
plt.grid(True, alpha=0.3)
plt.xscale("log")
plt.yscale("log")
plt.tight_layout()
plt.savefig("metrics/grafico_io_saida.png", dpi=200)
plt.close()

# --- Gráfico 3: Memória ---
plt.figure(figsize=(8, 5))
plt.plot(ns, mem_kb, marker="o", linewidth=2, color="purple")
plt.title("Counting Sort - Memória pico vs n")
plt.xlabel("n (quantidade de números)")
plt.ylabel("Memória pico (KB)")
plt.grid(True, alpha=0.3)
plt.xscale("log")
plt.yscale("log")
plt.tight_layout()
plt.savefig("metrics/grafico_memoria.png", dpi=200)
plt.close()

print("Gerados:")
print(" - metrics/grafico_tempo.png")
print(" - metrics/grafico_io_saida.png")
print(" - metrics/grafico_memoria.png")
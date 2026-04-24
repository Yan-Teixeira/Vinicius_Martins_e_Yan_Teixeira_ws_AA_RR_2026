#!/usr/bin/env bash
set -euo pipefail

# roda sempre a partir da raiz do projeto
cd "$(dirname "$0")/.."

make limpar
make
make gerar
make medir
make graficos

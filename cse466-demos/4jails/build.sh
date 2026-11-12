#!/bin/bash
set -e
cd "$(dirname "$0")"

for n in 1 2 3 4; do
    gcc -w -O0 -o "jail$n" "jail$n.c"
done
#!/usr/bin/env bash

cd "$(dirname "${BASH_SOURCE[0]}")"
set -euxo pipefail

clang -g3 -o miner miner.c sha-256.c ring-buffer.c

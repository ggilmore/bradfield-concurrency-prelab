#!/usr/bin/env bash

cd "$(dirname "${BASH_SOURCE[0]}")"
set -euxo pipefail

clang -o miner miner.c sha-256.c

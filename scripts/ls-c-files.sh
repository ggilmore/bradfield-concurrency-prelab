#!/usr/bin/env bash

set -eu pipefail

IGNORE_DIRS=(

)

REPOSITORY_ROOT_RELATIVE_PATH="$(realpath --relative-to="$(pwd)" "$(dirname "${BASH_SOURCE[0]}")"/..)"

fd --extension c --extension h --exclude "${IGNORE_DIRS[@]}" . "${REPOSITORY_ROOT_RELATIVE_PATH}"

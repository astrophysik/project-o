#!/usr/bin/env bash

set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "Usage: $0 <executable> <directory>"
    exit 1
fi

EXECUTABLE="$1"
DIR="$2"

if [[ ! -x "$EXECUTABLE" ]]; then
    echo "Error: '$EXECUTABLE' is not executable or does not exist"
    exit 1
fi

if [[ ! -d "$DIR" ]]; then
    echo "Error: '$DIR' is not a directory or does not exist"
    exit 1
fi

for filepath in "$DIR"/*; do
    [[ -f "$filepath" ]] || continue
    filename=$(basename "$filepath")
    echo ">>> $filename"
    "$EXECUTABLE" "$filepath"
done

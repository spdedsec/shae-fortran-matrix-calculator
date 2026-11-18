#!/usr/bin/env bash
set -euo pipefail
BINARY="bin/shae-fortran-matrix-calculator"
mkdir -p ~/.local/bin
cp "$BINARY" ~/.local/bin/
echo "Installed shae-fortran-matrix-calculator to ~/.local/bin"
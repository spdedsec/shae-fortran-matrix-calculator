# shae-fortran-matrix-calculator

A CLI matrix calculator conforming to the SHAE Field Manual design system.

## Installation

```bash
make
make install
```

This copies `~/.local/bin/shae-fortran-matrix-calculator`.

CLI usage:

```
shae-fortran-matrix-calculator add A.txt B.txt   # Matrix addition
shae-fortran-matrix-calculator sub A.txt B.txt   # Matrix subtraction
shae-fortran-matrix-calculator mul A.txt B.txt   # Matrix multiplication
shae-fortran-matrix-calculator transpose A.txt   # Transpose
shae-fortran-matrix-calculator det A.txt          # Determinant
shae-fortran-matrix-calculator inverse A.txt      # Matrix inverse
```

Matrix file format: whitespace-separated numbers, one row per line.
Invalid input returns exit code 1 with a clear error message to stderr.

## Design

- No gradients, glass, shadows, or pills
- Output uses plain text with monotone formatting
- Error messages routed to stderr in oxide-red console styling
- All numeric output avoids ornamental formatting; uses %g precision
- Focus rings and motion guidelines apply only to the reference web UI

## Release workflow

Run `make release` to build the binary and publish a GitHub release with the asset.
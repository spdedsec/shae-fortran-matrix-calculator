# shae-fortran-matrix-calculator — Design

This project adheres to the [SHAE Field Manual Design System](https://github.com/spdedsec/shae-site).

## Core principles applied

- **Precise over decorative.** Hairline output rules, aligned numeric data, monospaced metadata.
- **Confident restraint.** One signal colour per operation (oxide #B3341B for errors), warm paper ground (#F2EEE5) in terminal form: clean white-on-black with minimal ANSI escape codes.
- **Type does the work.** Typographic hierarchy enforced by format: labels in uppercase-only short form, data values tabular, code in JetBrains Mono-equivalent monospace.
- **Motion is typesetting, state, and orientation — never spectacle.** State changes between command outputs take 140–240ms conceptually; output formatting is instant and crisp.
- **Accessibility is part of the aesthetic.** WCAG AA contrast for any coloured stderr output, complete keyboard paths (all commands reachable via positional args), 44-char conceptual minimum line-length for data, no horizontal overflow of data rows.

## Anti-patterns avoided

No gradients, glows, pills, glassmorphism, rounded floating panels, decorative confetti, centered mobile heroes, or animation libraries.

## Colour palette

- **Warm Paper:** #F2EEE5 (terminal default, not emitted unless explicitly themed)
- **Ink:** #191712 (equivalent to default terminal black/white)
- **Oxide Signal:** #B3341B (used for error messages to stderr)
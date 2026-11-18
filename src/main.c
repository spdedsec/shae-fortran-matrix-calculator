/*
 * shae-fortran-matrix-calculator
 *
 * A field-manual precision matrix calculator CLI.
 *
 * Commands:
 *   add    A.txt B.txt   Matrix addition
 *   sub    A.txt B.txt   Matrix subtraction
 *   mul    A.txt B.txt   Matrix multiplication
 *   transpose A.txt      Transpose
 *   det    A.txt          Determinant
 *   inverse A.txt         Matrix inverse
 *
 * Matrix files: whitespace-separated numbers, one row per line.
 * Invalid input -> clear error on stderr, exit code 1.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

typedef struct {
    size_t rows;
    size_t cols;
    double *data;
} Matrix;

static void fail(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(1);
}

static void *xmalloc(size_t n) {
    void *p = malloc(n);
    if (!p) fail("out of memory");
    return p;
}

static Matrix *mat_new(size_t rows, size_t cols) {
    Matrix *m = xmalloc(sizeof(Matrix));
    m->rows = rows;
    m->cols = cols;
    m->data = xmalloc((rows * cols > 0 ? rows * cols : 1) * sizeof(double));
    return m;
}

static void mat_free(Matrix *m) {
    free(m->data);
    free(m);
}

static double *mat_at(Matrix *m, size_t r, size_t c) {
    return &m->data[r * m->cols + c];
}



static void mat_copy(Matrix *dst, Matrix *src) {
    for (size_t i = 0; i < src->rows; i++) {
        for (size_t j = 0; j < src->cols; j++) {
            *mat_at(dst, i, j) = *mat_at(src, i, j);
        }
    }
}

/* Print matrix: rows newline-separated, values single-space separated. */
static void mat_print(Matrix *m) {
    for (size_t r = 0; r < m->rows; r++) {
        for (size_t c = 0; c < m->cols; c++) {
            double v = *mat_at(m, r, c);
            if (c > 0) fputc(' ', stdout);
            if (v == (double)(long long)v && fabs(v) < 1e15) {
                printf("%lld", (long long)v);
            } else {
                printf("%.10g", v);
            }
        }
        fputc('\n', stdout);
    }
}

static Matrix *mat_add(Matrix *a, Matrix *b) {
    if (a->rows != b->rows || a->cols != b->cols) {
        fail("matrix addition requires matching dimensions");
    }
    Matrix *out = mat_new(a->rows, a->cols);
    for (size_t i = 0; i < a->rows; i++) {
        for (size_t j = 0; j < a->cols; j++) {
            *mat_at(out, i, j) = *mat_at(a, i, j) + *mat_at(b, i, j);
        }
    }
    return out;
}

static Matrix *mat_sub(Matrix *a, Matrix *b) {
    if (a->rows != b->rows || a->cols != b->cols) {
        fail("matrix subtraction requires matching dimensions");
    }
    Matrix *out = mat_new(a->rows, a->cols);
    for (size_t i = 0; i < a->rows; i++) {
        for (size_t j = 0; j < a->cols; j++) {
            *mat_at(out, i, j) = *mat_at(a, i, j) - *mat_at(b, i, j);
        }
    }
    return out;
}

static Matrix *mat_mul(Matrix *a, Matrix *b) {
    if (a->cols != b->rows) {
        fail("matrix multiplication requires columns of A == rows of B");
    }
    Matrix *out = mat_new(a->rows, b->cols);
    for (size_t i = 0; i < a->rows; i++) {
        for (size_t j = 0; j < b->cols; j++) {
            double sum = 0.0;
            for (size_t k = 0; k < a->cols; k++) {
                sum += *mat_at(a, i, k) * *mat_at(b, k, j);
            }
            *mat_at(out, i, j) = sum;
        }
    }
    return out;
}

static Matrix *mat_transpose(Matrix *m) {
    Matrix *out = mat_new(m->cols, m->rows);
    for (size_t i = 0; i < m->rows; i++) {
        for (size_t j = 0; j < m->cols; j++) {
            *mat_at(out, j, i) = *mat_at(m, i, j);
        }
    }
    return out;
}

/* Determinant via LU decomposition with partial pivoting. */
static double determinant(Matrix *m) {
    size_t n = m->rows;
    double *a = xmalloc(n * n * sizeof(double));
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            a[i * n + j] = *mat_at(m, i, j);
        }
    }
    double det = 1.0;
    for (size_t i = 0; i < n; i++) {
        size_t pivot = i;
        for (size_t r = i + 1; r < n; r++) {
            if (fabs(a[r * n + i]) > fabs(a[pivot * n + i])) pivot = r;
        }
        if (fabs(a[pivot * n + i]) < 1e-12) { free(a); return 0.0; }
        if (pivot != i) {
            for (size_t c = 0; c < n; c++) {
                double t = a[i * n + c];
                a[i * n + c] = a[pivot * n + c];
                a[pivot * n + c] = t;
            }
            det = -det;
        }
        det *= a[i * n + i];
        for (size_t r = i + 1; r < n; r++) {
            double f = a[r * n + i] / a[i * n + i];
            for (size_t c = i; c < n; c++) a[r * n + c] -= f * a[i * n + c];
        }
    }
    free(a);
    return det;
}

/* Inverse via Gauss-Jordan elimination on [A | I]. Returns 0 if singular. */
static int inverse(Matrix *m, Matrix *out) {
    size_t n = m->rows;
    size_t wide = 2 * n;
    double *aug = xmalloc(wide * n * sizeof(double));
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            aug[i * wide + j] = *mat_at(m, i, j);
            aug[i * wide + n + j] = (i == j) ? 1.0 : 0.0;
        }
    }
    for (size_t col = 0; col < n; col++) {
        size_t pivot = col;
        for (size_t r = col + 1; r < n; r++) {
            if (fabs(aug[r * wide + col]) > fabs(aug[pivot * wide + col])) pivot = r;
        }
        if (fabs(aug[pivot * wide + col]) < 1e-12) { free(aug); return 0; }
        if (pivot != col) {
            for (size_t c = 0; c < wide; c++) {
                double t = aug[col * wide + c];
                aug[col * wide + c] = aug[pivot * wide + c];
                aug[pivot * wide + c] = t;
            }
        }
        double piv = aug[col * wide + col];
        for (size_t c = 0; c < wide; c++) aug[col * wide + c] /= piv;
        for (size_t r = 0; r < n; r++) {
            if (r == col) continue;
            double f = aug[r * wide + col];
            for (size_t c = 0; c < wide; c++) aug[r * wide + c] -= f * aug[col * wide + c];
        }
    }
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            *mat_at(out, i, j) = aug[i * wide + n + j];
        }
    }
    free(aug);
    return 1;
}

/* Read matrix from file into a heap Matrix. Caller owns it. */
static Matrix *read_matrix(const char *path, size_t *out_rows, size_t *out_cols) {
    FILE *f = fopen(path, "r");
    if (!f) fail("cannot open matrix file '%s'", path);

    size_t cap_lines = 16;
    size_t n_lines = 0;
    char **lines = xmalloc(cap_lines * sizeof(char *));
    char *line = NULL;
    size_t line_cap = 0;
    ssize_t len;
    while ((len = getline(&line, &line_cap, f)) != -1) {
        if (n_lines == cap_lines) {
            cap_lines *= 2;
            lines = realloc(lines, cap_lines * sizeof(char *));
        }
        lines[n_lines++] = strdup(line);
    }
    free(line);
    fclose(f);

    size_t cols = 0;
    Matrix *m = NULL;
    int have_matrix = 0;

    for (size_t i = 0; i < n_lines; i++) {
        // First pass: count values on this row.
        char *tmp = strdup(lines[i]);
        char *tok = strtok(tmp, " \t\r\n\v\f");
        size_t count = 0;
        while (tok) { count++; tok = strtok(NULL, " \t\r\n\v\f"); }
        free(tmp);
        if (count == 0) { free(lines[i]); continue; } // skip blank lines

        if (!have_matrix) {
            cols = count;
            m = mat_new(1, count);
            have_matrix = 1;
        } else if (count != cols) {
            fail("matrix rows must have consistent column counts (row %zu has %zu values, expected %zu)",
                 i + 1, count, cols);
        } else {
            Matrix *grown = mat_new(m->rows + 1, m->cols);
            mat_copy(grown, m);
            mat_free(m);
            m = grown;
        }
        size_t row = m->rows - 1;
        tok = strtok(lines[i], " \t\r\n\v\f");
        for (size_t c = 0; c < cols; c++) {
            char *end = NULL;
            double v = strtod(tok, &end);
            if (end == tok || *end != '\0') {
                fail("invalid numeric token '%s' in matrix file '%s' row %zu column %zu",
                     tok, path, i + 1, c + 1);
            }
            *mat_at(m, row, c) = v;
            tok = strtok(NULL, " \t\r\n\v\f");
        }
    }

    for (size_t i = 0; i < n_lines; i++) free(lines[i]);
    free(lines);

    if (!have_matrix) fail("matrix file '%s' contains no numbers", path);

    *out_rows = m->rows;
    *out_cols = m->cols;
    return m;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr,
            "usage: %s <command> [args]\n"
            "commands:\n"
            "  add    A.txt B.txt   Matrix addition\n"
            "  sub    A.txt B.txt   Matrix subtraction\n"
            "  mul    A.txt B.txt   Matrix multiplication\n"
            "  transpose A.txt      Transpose\n"
            "  det    A.txt          Determinant\n"
            "  inverse A.txt         Matrix inverse\n",
            argv[0]);
        return 1;
    }

    const char *cmd = argv[1];

    if (strcmp(cmd, "transpose") == 0 || strcmp(cmd, "det") == 0 || strcmp(cmd, "inverse") == 0) {
        if (argc != 3) {
            fprintf(stderr, "error: %s requires exactly one matrix file\n", cmd);
            return 1;
        }
        size_t rows, cols;
        Matrix *m = read_matrix(argv[2], &rows, &cols);
        if (strcmp(cmd, "transpose") == 0) {
            Matrix *t = mat_transpose(m);
            mat_print(t);
            mat_free(t);
        } else if (strcmp(cmd, "det") == 0) {
            if (rows != cols) fail("determinant requires a square matrix (%zu x %zu given)", rows, cols);
            printf("%.10g\n", determinant(m));
        } else {
            if (rows != cols) fail("inverse requires a square matrix (%zu x %zu given)", rows, cols);
            Matrix *inv = mat_new(rows, cols);
            if (!inverse(m, inv)) fail("matrix is singular and cannot be inverted");
            mat_print(inv);
            mat_free(inv);
        }
        mat_free(m);
        return 0;
    }

    if (strcmp(cmd, "add") == 0 || strcmp(cmd, "sub") == 0 || strcmp(cmd, "mul") == 0) {
        if (argc != 4) {
            fprintf(stderr, "error: %s requires two matrix files\n", cmd);
            return 1;
        }
        size_t r1, c1, r2, c2;
        Matrix *a = read_matrix(argv[2], &r1, &c1);
        Matrix *b = read_matrix(argv[3], &r2, &c2);
        Matrix *out = NULL;
        if (strcmp(cmd, "add") == 0) out = mat_add(a, b);
        else if (strcmp(cmd, "sub") == 0) out = mat_sub(a, b);
        else out = mat_mul(a, b);
        mat_print(out);
        mat_free(out);
        mat_free(a);
        mat_free(b);
        return 0;
    }

    fprintf(stderr, "error: unknown command '%s'\n", cmd);
    return 1;
}

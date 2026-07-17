#include <stdio.h>
#include <stdlib.h>

void
main(int argc, char **argv) {
    char c;
    FILE *f;

    if ( argc != 2 ) {
        fprintf(stderr, "useage: frommac <filename>\n");
        exit(1);
    }

    if ( ( f = fopen(argv[1], "r")) == NULL ) {
        fprintf(stderr, "can't open file %s\n", argv[1]);
        exit(1);
    }

    while (!feof(f)) {
        c = getc(f);
        putchar ( c == '\r' ? '\n' : c );
    }
    putchar ( '\n');
    fflush(stdout);
}

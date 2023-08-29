//#include "lib/sstring.h"
//#include "token.h"

typedef struct {
    char *file;
    char *src;

    int cur;
    int start;
    int line;

    char ch;
} Scanner;

void scanner_make(Scanner *s, char *filepath);
void scanner_scan(Scanner *s, Tokens *toks);

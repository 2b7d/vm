typedef struct {
    char *ptr;
    int len;
} string;

void string_make(string *s, char *ptr, int len);
int string_cmp(string *s1, string *s2);

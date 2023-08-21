typedef struct {
    char *ptr;
    int len;
} string;

void string_make(string *s, char *ptr, int len);
void string_fromc(string *s, char *cstr);
void string_init(string *s, int len);
void string_init_fromc(string *s, char *cstr);

void string_dup(string *dst, string *src);
void string_cpy(string *dst, string *src);

int string_cmp(string *s1, string *s2);
int string_cmpc(string *s, char *cstr);

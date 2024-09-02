#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../lib/sstring.h"

#include "../vm.h"
#include "../linker/ln.h"

char *sec_to_str(enum vm_section kind)
{
    switch (kind) {
    case SECTION_DATA:
        return "DATA";
    case SECTION_TEXT:
        return "TEXT";
    default:
        assert(0 && "unreachable");
    }
}

char *symkind_to_str(enum ln_symkind kind)
{
    switch (kind) {
    case SYM_LOCAL:
        return "LOCAL";
    case SYM_GLOBAL:
        return "GLOBAL";
    case SYM_EXTERN:
        return "EXTERN";
    default:
        assert(0 && "unreachable");
    }
}

int main(int argc, char **argv)
{
    string data, text;
    struct ln_header header;
    FILE *in;

    argc--;
    argv++;

    if (argc < 1) {
        fprintf(stderr, "Provide file to disassemble\n");
        return 1;
    }

    in = fopen(*argv, "r");
    if (in == NULL) {
        perror(NULL);
        return 1;
    }

    memset(&header, 0, sizeof(struct ln_header));
    fread(&header.nsyms, 2, 1, in);
    fread(&header.nrels, 2, 1, in);

    printf("Header\n"
            "    nsyms: %d\n"
            "    nrels: %d\n", header.nsyms, header.nrels);


    data.len = 0;
    fread(&data.len, 2, 1, in);
    if (data.len > 0) {
        string_init(&data, data.len);
        fread(data.ptr, 1, data.len, in);
    }

    text.len = 0;
    fread(&text.len, 2, 1, in);
    if (text.len > 0) {
        string_init(&text, text.len);
        fread(text.ptr, 1, text.len, in);
    }

    if (header.nsyms > 0) {
        printf("Symbols\n");
    }

    for (int i = 0; i < header.nsyms; ++i) {
        struct ln_symbol s;

        memset(&s, 0, sizeof(struct ln_symbol));

        fread(&s.kind, 1, 1, in);
        fread(&s.sec, 1, 1, in);
        fread(&s.idx, 2, 1, in);
        fread(&s.addr, 2, 1, in);

        fread(&s.label.len, 2, 1, in);
        string_init(&s.label, s.label.len);
        fread(s.label.ptr, 1, s.label.len, in);

        printf("    kind(%6s) section(%4s) idx(%05d) addr(%05d) label(%.*s)\n", symkind_to_str(s.kind), sec_to_str(s.sec), s.idx, s.addr, s.label.len, s.label.ptr);
    }

    if (header.nrels > 0) {
        printf("Relocations\n");
    }

    for (int i = 0; i < header.nrels; ++i) {
        struct ln_relocation r;

        memset(&r, 0, sizeof(struct ln_relocation));

        fread(&r.loc, 2, 1, in);
        fread(&r.symidx, 2, 1, in);

        printf("    loc(%05d) symidx(%05d)\n", r.loc, r.symidx);
    }

    if (data.len > 0) {
        char ch;
        int i, col;

        printf("Data(%d)\n", data.len);

        i = 0;
        col = 0;
        while (i < data.len) {
            ch = data.ptr[i];
            printf("    %05d: %5d ", i, ch);
            col++;
            if (col == 5) {
                col = 0;
                printf("\n");
            }
            i++;
        }
        printf("\n");
    }

    if (text.len > 0) {
        enum vm_opcode op;
        char ch;
        int i, word;

        printf("Text(%d)\n", text.len);

        i = 0;
        while (i < text.len) {
            ch = text.ptr[i];
            op = ch;

            printf("    %05d: ", i);

            i++;

            switch (op) {
            case OP_HALT:
                printf("halt\n");
                break;

            case OP_PUSH:
                word = text.ptr[i];
                i += 2;
                printf("push %d\n", word);
                break;
            case OP_PUSHB:
                word = text.ptr[i++];
                printf("pushb %d\n", word);
                break;
            case OP_DROP:
                printf("drop\n");
                break;
            case OP_DROPB:
                printf("dropb\n");
                break;

            case OP_LD:
                printf("ld\n");
                break;
            case OP_LDB:
                printf("ldb\n");
                break;
            case OP_ST:
                printf("st\n");
                break;
            case OP_STB:
                printf("stb\n");
                break;

            case OP_CTW:
                printf("ctw\n");
                break;
            case OP_CTB:
                printf("ctb\n");
                break;

            case OP_ADD:
                printf("add\n");
                break;
            case OP_ADDB:
                printf("addb\n");
                break;
            case OP_SUB:
                printf("sub\n");
                break;
            case OP_SUBB:
                printf("subb\n");
                break;
            case OP_NEG:
                printf("neg\n");
                break;
            case OP_NEGB:
                printf("negb\n");
                break;

            case OP_EQ:
                printf("eq\n");
                break;
            case OP_EQB:
                printf("eqb\n");
                break;
            case OP_LT:
                printf("lt\n");
                break;
            case OP_LTB:
                printf("ltb\n");
                break;
            case OP_GT:
                printf("gt\n");
                break;
            case OP_GTB:
                printf("gtb\n");
                break;

            case OP_JMP:
                printf("jmp\n");
                break;
            case OP_CJMP:
                printf("cjmp\n");
                break;

            case OP_CALL:
                word = text.ptr[i];
                i += 2;
                printf("call %d\n", word);
                break;
            case OP_RET:
                printf("ret\n");
                break;

            case OP_SYSCALL:
                printf("syscall\n");
                break;

            default:
                assert(0 && "unreachable");
            }
        }
    }

    return 0;
}

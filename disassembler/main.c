#include <stdio.h>
#include <assert.h>

#include "../vm.h"

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

int main(int argc, char **argv)
{
    int _start_addr, nsecs;
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

    fread(&_start_addr, 2, 1, in);
    fread(&nsecs, 1, 1, in);

    printf("_start addr:        %d\n"
           "number of sections: %d\n\n",
           _start_addr, nsecs);

    for (int i = 0; i < nsecs; ++i) {
        enum vm_section kind;
        int len, op, word, cur;

        fread(&kind, 1, 1, in);
        fread(&len, 2, 1, in);

        printf("Section: %s\n"
               "Size:    %d\n"
               "----------------------\n",
               sec_to_str(kind), len);

        if (kind == SECTION_DATA) {
            char ch;
            int j, col;

            j = 0;
            col = 0;
            while (j < len) {
                fread(&ch, 1, 1, in);
                printf("%5d: %5d ", j + 6, ch); // TODO(art): magic number
                col++;
                if (col == 3) {
                    col = 0;
                    printf("\n");
                }
                j++;
            }
            printf("\n\n");
            continue;
        }

        cur = 0;
        while (cur < len) {
            fread(&op, 1, 1, in);

            switch (op) {
            case OP_HALT:
                printf("%5d: halt\n", cur);
                break;

            case OP_PUSH:
                fread(&word, 2, 1, in);
                printf("%5d: push %d\n", cur, word);
                cur += 2;
                break;
            case OP_PUSHB:
                fread(&word, 1, 1, in);
                printf("%5d: pushb %d\n", cur, word);
                cur++;
                break;
            case OP_DROP:
                printf("%5d: drop\n", cur);
                break;
            case OP_DROPB:
                printf("%5d: dropb\n", cur);
                break;

            case OP_LD:
                printf("%5d: ld\n", cur);
                break;
            case OP_LDB:
                printf("%5d: ldb\n", cur);
                break;
            case OP_ST:
                printf("%5d: st\n", cur);
                break;
            case OP_STB:
                printf("%5d: stb\n", cur);
                break;

            case OP_CTW:
                printf("%5d: ctw\n", cur);
                break;
            case OP_CTB:
                printf("%5d: ctb\n", cur);
                break;

            case OP_ADD:
                printf("%5d: add\n", cur);
                break;
            case OP_ADDB:
                printf("%5d: addb\n", cur);
                break;
            case OP_SUB:
                printf("%5d: sub\n", cur);
                break;
            case OP_SUBB:
                printf("%5d: subb\n", cur);
                break;
            case OP_NEG:
                printf("%5d: neg\n", cur);
                break;
            case OP_NEGB:
                printf("%5d: negb\n", cur);
                break;

            case OP_EQ:
                printf("%5d: eq\n", cur);
                break;
            case OP_EQB:
                printf("%5d: eqb\n", cur);
                break;
            case OP_LT:
                printf("%5d: lt\n", cur);
                break;
            case OP_LTB:
                printf("%5d: ltb\n", cur);
                break;
            case OP_GT:
                printf("%5d: gt\n", cur);
                break;
            case OP_GTB:
                printf("%5d: gtb\n", cur);
                break;

            case OP_JMP:
                printf("%5d: jmp\n", cur);
                break;
            case OP_CJMP:
                printf("%5d: cjmp\n", cur);
                break;

            case OP_CALL:
                fread(&word, 2, 1, in);
                printf("%5d: call %d\n", cur, word);
                cur += 2;
                break;
            case OP_RET:
                printf("%5d: ret\n", cur);
                break;

            case OP_SYSCALL:
                printf("%5d: syscall\n", cur);
                break;

            default:
                assert(0 && "unreachable");
            }

            cur++;
        }

        printf("\n");
    }

    return 0;
}

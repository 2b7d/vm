#include <assert.h>

void reset_vm()
{
    memset(ram, 0, sizeof(ram));
    memset(regfile, 0, sizeof(regfile));
    memset(&regflags, 0, sizeof(regflags));
    pc = 0;
}

#define arrlen(arr) (sizeof((arr)) / sizeof(*(arr)))
#define encode_registers(r1, r2) ((r1) << 4) | ((r2) & 0x0f)

#define halt() write_byte(HALT, pc++)

#define mov(r1, r2) write_byte(MOV, pc++), write_byte(encode_registers((r1), (r2)), pc++)
#define movi(imm, r) write_byte(MOVI, pc++), write_word((imm), pc++), pc++, write_byte((r), pc++)
#define movb(r1, r2) write_byte(MOVB, pc++), write_byte(encode_registers((r1), (r2)), pc++)
#define movbe(r1, r2) write_byte(MOVBE, pc++), write_byte(encode_registers((r1), (r2)), pc++)

#define st(r1, r2) write_byte(ST, pc++), write_byte(encode_registers((r1), (r2)), pc++)
#define sti(r, imm) write_byte(STI, pc++), write_byte((r), pc++), write_word((imm), pc++), pc++
#define stb(r1, r2) write_byte(STB, pc++), write_byte(encode_registers((r1), (r2)), pc++)
#define stbi(r, imm) write_byte(STBI, pc++), write_byte((r), pc++), write_word((imm), pc++), pc++

#define ld(r1, r2) write_byte(LD, pc++), write_byte(encode_registers((r1), (r2)), pc++)
#define ldi(imm, r) write_byte(LDI, pc++), write_word((imm), pc++), pc++, write_byte((r), pc++)
#define ldb(r1, r2) write_byte(LDB, pc++), write_byte(encode_registers((r1), (r2)), pc++)
#define ldbi(imm, r) write_byte(LDBI, pc++), write_word((imm), pc++), pc++, write_byte((r), pc++)

#define add(r1, r2) write_byte(ADD, pc++), write_byte(encode_registers((r1), (r2)), pc++)
#define addi(imm, r) write_byte(ADDI, pc++), write_word((imm), pc++), pc++, write_byte((r), pc++)
#define addb(r1, r2) write_byte(ADDB, pc++), write_byte(encode_registers((r1), (r2)), pc++)
#define addbi(imm, r) write_byte(ADDBI, pc++), write_byte((imm), pc++), write_byte((r), pc++)
#define inc(r) write_byte(INC, pc++), write_byte((r), pc++)
#define incb(r) write_byte(INCB, pc++), write_byte((r), pc++)

#define sub(r1, r2) write_byte(SUB, pc++), write_byte(encode_registers((r1), (r2)), pc++)
#define subi(imm, r) write_byte(SUBI, pc++), write_word((imm), pc++), pc++, write_byte((r), pc++)
#define subb(r1, r2) write_byte(SUBB, pc++), write_byte(encode_registers((r1), (r2)), pc++)
#define subbi(imm, r) write_byte(SUBBI, pc++), write_byte((imm), pc++), write_byte((r), pc++)
#define dec(r) write_byte(DEC, pc++), write_byte((r), pc++)
#define decb(r) write_byte(DECB, pc++), write_byte((r), pc++)

#define cmp(r1, r2) write_byte(CMP, pc++), write_byte(encode_registers((r1), (r2)), pc++)
#define cmpi(imm, r) write_byte(CMPI, pc++), write_word((imm), pc++), pc++, write_byte((r), pc++)
#define cmpb(r1, r2) write_byte(CMPB, pc++), write_byte(encode_registers((r1), (r2)), pc++)
#define cmpbi(imm, r) write_byte(CMPBI, pc++), write_byte((imm), pc++), write_byte((r), pc++)

#define jabs(imm) write_byte(JABS, pc++), write_word((imm), pc++), pc++
#define je(imm) write_byte(JE, pc++), write_word((imm), pc++), pc++
#define jne(imm) write_byte(JNE, pc++), write_word((imm), pc++), pc++
#define jg(imm) write_byte(JG, pc++), write_word((imm), pc++), pc++
#define jge(imm) write_byte(JGE, pc++), write_word((imm), pc++), pc++
#define jl(imm) write_byte(JL, pc++), write_word((imm), pc++), pc++
#define jle(imm) write_byte(JLE, pc++), write_word((imm), pc++), pc++
#define ja(imm) write_byte(JA, pc++), write_word((imm), pc++), pc++
#define jae(imm) write_byte(JAE, pc++), write_word((imm), pc++), pc++
#define jb(imm) write_byte(JB, pc++), write_word((imm), pc++), pc++
#define jbe(imm) write_byte(JBE, pc++), write_word((imm), pc++), pc++

#define push(r) write_byte(PUSH, pc++), write_byte((r), pc++)
#define pushi(imm) write_byte(PUSHI, pc++), write_word((imm), pc++), pc++
#define pop(r) write_byte(POP, pc++), write_byte((r), pc++)

#define call(imm) write_byte(CALL, pc++), write_word((imm), pc++), pc++
#define callr(r) write_byte(CALLR, pc++), write_byte((r), pc++)
#define ret() write_byte(RET, pc++)

#include "mov.c"
#include "movi.c"
#include "movb.c"
#include "movbe.c"

#include "st.c"
#include "sti.c"
#include "stb.c"
#include "stbi.c"

#include "ld.c"
#include "ldi.c"
#include "ldb.c"
#include "ldbi.c"

#include "add.c"
#include "addi.c"
#include "addb.c"
#include "addbi.c"
#include "inc.c"
#include "incb.c"

#include "sub.c"
#include "subi.c"
#include "subb.c"
#include "subbi.c"
#include "dec.c"
#include "decb.c"

#include "cmp.c"
#include "cmpi.c"
#include "cmpb.c"
#include "cmpbi.c"

#include "jabs.c"
#include "je.c"
#include "jne.c"
#include "jg.c"
#include "jge.c"
#include "jl.c"
#include "jle.c"
#include "ja.c"
#include "jae.c"
#include "jb.c"
#include "jbe.c"

#include "push.c"
#include "pushi.c"
#include "pop.c"

#include "call.c"
#include "callr.c"
#include "ret.c"

int main(void)
{
    test_mov();
    test_movi();
    test_movb();
    test_movbe();

    test_st();
    test_sti();
    test_stb();
    test_stbi();

    test_ld();
    test_ldi();
    test_ldb();
    test_ldbi();

    test_add();
    test_addi();
    test_addb();
    test_addbi();
    test_inc();
    test_incb();

    test_sub();
    test_subi();
    test_subb();
    test_subbi();
    test_dec();
    test_decb();

    test_cmp();
    test_cmpi();
    test_cmpb();
    test_cmpbi();

    test_jabs();
    test_je();
    test_jne();
    test_jg();
    test_jge();
    test_jl();
    test_jle();
    test_ja();
    test_jae();
    test_jb();
    test_jbe();

    test_push();
    test_pushi();
    test_pop();

    test_call();
    test_callr();
    test_ret();

    return 0;
}

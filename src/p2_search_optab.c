#include <string.h>
#include <ctype.h>

#include "optab.h"

/* 
 * SIC/XE까지 포함된 OPTAB 예시.
 * 과제는 SIC만 써도 되지만, XE 코드가 더 있어도 상관없으니 그냥 두자.
 * (실제 PASS-2에서 쓰는 건 필요한 것들만 쓰면 됨)
 */
static const OpTabEntry optab[] = {
    { "ADD",   0x18 }, { "ADDF",  0x58 }, { "ADDR",  0x90 }, { "AND",   0x40 },
    { "CLEAR", 0xB4 }, { "COMP",  0x28 }, { "COMPF", 0x88 }, { "COMPR", 0xA0 },
    { "DIV",   0x24 }, { "DIVF",  0x64 }, { "DIVR",  0x9C }, { "FIX",   0xC4 },
    { "FLOAT", 0xC0 }, { "HIO",   0xF4 }, { "J",     0x3C }, { "JEQ",   0x30 },
    { "JGT",   0x34 }, { "JLT",   0x38 }, { "JSUB",  0x48 }, { "LDA",   0x00 },
    { "LDB",   0x68 }, { "LDCH",  0x50 }, { "LDF",   0x70 }, { "LDL",   0x08 },
    { "LDS",   0x6C }, { "LDT",   0x74 }, { "LDX",   0x04 }, { "LPS",   0xD0 },
    { "MUL",   0x20 }, { "MULF",  0x60 }, { "MULR",  0x98 }, { "NORM",  0xC8 },
    { "OR",    0x44 }, { "RD",    0xD8 }, { "RMO",   0xAC }, { "RSUB",  0x4C },
    { "SHIFTL",0xA4 }, { "SHIFTR",0xA8 }, { "SIO",   0xF0 }, { "SSK",   0xEC },
    { "STA",   0x0C }, { "STB",   0x78 }, { "STCH",  0x54 }, { "STF",   0x80 },
    { "STI",   0xD4 }, { "STL",   0x14 }, { "STS",   0x7C }, { "STSW",  0xE8 },
    { "STT",   0x84 }, { "STX",   0x10 }, { "SUB",   0x1C }, { "SUBF",  0x5C },
    { "SUBR",  0x94 }, { "SVC",   0xB0 }, { "TD",    0xE0 }, { "TIO",   0xF8 },
    { "TIX",   0x2C }, { "TIXR",  0xB8 }, { "WD",    0xDC },

    /* assembler directive 들 (코드 값은 의미 없음) */
    { "START", 0 }, { "END", 0 }, { "BYTE", 0 }, { "RESW", 0 },
    { "RESB", 0 }, { "WORD", 0 },
};

static void upper_copy(char *dst, const char *src, int maxlen) {
    int i;
    for (i = 0; i < maxlen - 1 && src[i]; ++i) {
        dst[i] = (char)toupper((unsigned char)src[i]);
    }
    dst[i] = '\0';
}

int optab_lookup(const char *mnemonic, int *opcode_out) {
    char key[8];
    upper_copy(key, mnemonic, sizeof(key));

    int n = (int)(sizeof(optab) / sizeof(optab[0]));
    for (int i = 0; i < n; ++i) {
        if (strcmp(key, optab[i].name) == 0) {
            if (opcode_out) {
                *opcode_out = optab[i].code;
            }
            return 1;
        }
    }
    return 0;
}

int optab_is_directive(const char *mnemonic) {
    char key[8];
    upper_copy(key, mnemonic, sizeof(key));

    return (strcmp(key, "BYTE")  == 0 ||
            strcmp(key, "WORD")  == 0 ||
            strcmp(key, "RESB")  == 0 ||
            strcmp(key, "RESW")  == 0 ||
            strcmp(key, "START") == 0 ||
            strcmp(key, "END")   == 0);
}

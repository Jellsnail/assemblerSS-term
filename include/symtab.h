#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdio.h>
#include "assembler.h"

#define MAX_SYMBOLS 500

typedef struct {
    char name[MAX_LABEL_LEN];
    int  address;
    int  line_no;
} Symbol;

typedef struct {
    Symbol table[MAX_SYMBOLS];
    int count;
} SymTab;

/* 전역 심볼 테이블 (단순 과제용) */
extern SymTab g_symtab;

void symtab_init(void);
int  symtab_insert(const char *name, int address, int line_no, int *is_dup);
int  symtab_search(const char *name, int *out_addr);
void symtab_write_to_file(const char *path);

#endif


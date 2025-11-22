#include <stdio.h>
#include <string.h>
#include "symtab.h"

SymTab g_symtab;

void symtab_init(void) {
    g_symtab.count = 0;
}

static int symtab_find_index(const char *name) {
    for (int i = 0; i < g_symtab.count; ++i) {
        if (strcmp(g_symtab.table[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int symtab_insert(const char *name, int address, int line_no, int *is_dup) {
    if (is_dup) *is_dup = 0;

    if (g_symtab.count >= MAX_SYMBOLS) {
        fprintf(stderr, "SYMTAB overflow: symbols > %d\n", MAX_SYMBOLS);
        return 0;
    }

    int idx = symtab_find_index(name);
    if (idx >= 0) {
        if (is_dup) *is_dup = 1;
        return 0;   // 중복
    }

    Symbol *s = &g_symtab.table[g_symtab.count++];
    strncpy(s->name, name, MAX_LABEL_LEN - 1);
    s->name[MAX_LABEL_LEN - 1] = '\0';
    s->address = address;
    s->line_no = line_no;
    return 1;
}

int symtab_search(const char *name, int *out_addr) {
    int idx = symtab_find_index(name);
    if (idx < 0) return 0;
    if (out_addr) *out_addr = g_symtab.table[idx].address;
    return 1;
}

void symtab_write_to_file(const char *path) {
    FILE *fp = fopen(path, "w");
    if (!fp) {
        perror("failed to write symtab file");
        return;
    }

    fprintf(fp, "SYMBOL  ADDRESS  LINE\n");
    for (int i = 0; i < g_symtab.count; ++i) {
        Symbol *s = &g_symtab.table[i];
        fprintf(fp, "%-6s  %04X     %d\n",
                s->name, s->address & 0xFFFF, s->line_no);
    }

    fclose(fp);
}

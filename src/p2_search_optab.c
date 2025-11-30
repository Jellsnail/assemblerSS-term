// p2_search_optab.c
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "optab.h"

static OpTabEntry optab[OPTAB_MAX_ENTRIES];
static int optab_count   = 0;
static int optab_loaded  = 0;

static void upper_copy(char *dst, const char *src, int maxlen) {
    int i;
    for (i = 0; i < maxlen - 1 && src[i]; ++i) {
        dst[i] = (char)toupper((unsigned char)src[i]);
    }
    dst[i] = '\0';
}

int p2_optab_init(const char *path)
{
    FILE *fp;
    char line[128];

    optab_count  = 0;
    optab_loaded = 0;

    fp = fopen(path, "r");
    if (!fp) {
        perror("p2_optab_init: fopen");
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        char *p = line;
        char name[OPTAB_MNEMONIC_LEN];
        unsigned int code_hex;

        // 앞 공백 스킵
        while (*p && isspace((unsigned char)*p)) p++;

        // 빈 줄 / 주석(".","#")은 무시
        if (*p == '\0' || *p == '\n' || *p == '.' || *p == '#')
            continue;

        // "<MNEMONIC> <HEX>" 형식
        if (sscanf(p, "%15s %x", name, &code_hex) != 2) {
            fprintf(stderr, "p2_optab_init: invalid line: %s", line);
            continue;
        }

        if (optab_count >= OPTAB_MAX_ENTRIES) {
            fprintf(stderr, "p2_optab_init: table full (max=%d)\n", OPTAB_MAX_ENTRIES);
            break;
        }

        upper_copy(optab[optab_count].name, name, OPTAB_MNEMONIC_LEN);
        optab[optab_count].code = (int)code_hex;
        optab_count++;
    }

    fclose(fp);
    optab_loaded = 1;
    return 1;
}

int p2_search_optab(const char *mnemonic, int *opcode_out)
{
    char key[OPTAB_MNEMONIC_LEN];
    int i;

    if (!optab_loaded) {
        fprintf(stderr, "p2_search_optab: optab not initialized\n");
        return 0;
    }

    if (!mnemonic) return 0;

    upper_copy(key, mnemonic, sizeof(key));

    for (i = 0; i < optab_count; ++i) {
        if (strcmp(key, optab[i].name) == 0) {
            if (opcode_out) {
                *opcode_out = optab[i].code;
            }
            return 1;
        }
    }
    return 0;
}

void p2_optab_finalize(void)
{
    optab_count  = 0;
    optab_loaded = 0;
}

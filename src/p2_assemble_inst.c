// p2_assemble_inst.c
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "intfile.h"
#include "symtab.h"
#include "optab.h"

// 대소문자 무시 비교
static int eq_icase(const char *a, const char *b) {
    while (*a && *b) {
        if (toupper((unsigned char)*a) != toupper((unsigned char)*b))
            return 0;
        a++; b++;
    }
    return *a == '\0' && *b == '\0';
}

// BYTE C'EOF' / X'F1' 처리
static int assemble_byte(const char *operand, char *objcode, int bufsize, int *objlen_out) {
    if (!operand || operand[0] == '\0') return -1;
    if (toupper((unsigned char)operand[0]) != 'C' &&
        toupper((unsigned char)operand[0]) != 'X')
        return -1;

    if (operand[1] != '\'') return -1;
    const char *p = operand + 2;
    const char *end = strchr(p, '\'');
    if (!end) return -1;

    int len = (int)(end - p);
    objcode[0] = '\0';

    if (toupper((unsigned char)operand[0]) == 'C') {
        // 문자 상수: 각 문자 → 1바이트
        *objlen_out = len;
        char tmp[8];
        for (int i = 0; i < len; i++) {
            snprintf(tmp, sizeof(tmp), "%02X", (unsigned char)p[i]);
            if ((int)strlen(objcode) + 2 >= bufsize) return -1;
            strcat(objcode, tmp);
        }
        return 1;
    } else {
        // 16진 상수: 그대로 사용 (공백 제거)
        char hexbuf[128];
        if (len + 1 > (int)sizeof(hexbuf)) return -1;
        int j = 0;
        for (int i = 0; i < len; i++) {
            if (!isspace((unsigned char)p[i]))
                hexbuf[j++] = (char)toupper((unsigned char)p[i]);
        }
        hexbuf[j] = '\0';

        if (j == 0 || (j % 2) != 0) return -1; // 홀수 개면 오류

        *objlen_out = j / 2;
        if (j + 1 > bufsize) return -1;
        strcpy(objcode, hexbuf);
        return 1;
    }
}

// WORD 상수 처리
static int assemble_word(const char *operand, char *objcode, int bufsize, int *objlen_out) {
    if (!operand || operand[0] == '\0') return -1;

    char *endp;
    long val = strtol(operand, &endp, 10);
    if (*endp != '\0' && !isspace((unsigned char)*endp)) {
        return -1;
    }

    int word = (int)(val & 0xFFFFFF);  // 24bit
    snprintf(objcode, bufsize, "%06X", word);
    *objlen_out = 3;
    return 1;
}

static int assemble_format3(const IntRecord *rec,
                            char *objcode, int bufsize, int *objlen_out) {
    int opcode_val;
    if (!p2_search_optab(rec->opcode, &opcode_val))
        return 0; // OPTAB에 없으면 여기서 처리 안 함 (pseudo-op일 가능성)

    // 피연산자 분해 (LABEL 또는 LABEL,X)
    char symbol[OPERAND_LEN];
    int x_flag = 0;

    if (rec->operand[0] == '\0') {
        symbol[0] = '\0';
    } else {
        const char *comma = strchr(rec->operand, ',');
        if (comma) {
            size_t n = (size_t)(comma - rec->operand);
            if (n >= sizeof(symbol)) n = sizeof(symbol) - 1;
            memcpy(symbol, rec->operand, n);
            symbol[n] = '\0';

            const char *p = comma + 1;
            while (*p && isspace((unsigned char)*p)) p++;
            if (*p == 'X' || *p == 'x') x_flag = 1;
        } else {
            strncpy(symbol, rec->operand, sizeof(symbol) - 1);
            symbol[sizeof(symbol) - 1] = '\0';
        }
    }

    int addr = 0;
    if (symbol[0] != '\0') {
        int found = symtab_search(symbol, &addr);
        if (!found) {
            // SYMTAB에 없는 심볼 → 에러로 보고 상위에 알림
            fprintf(stderr, "PASS2: undefined symbol '%s'\n", symbol);
            return -1;
        }
    }

    if (x_flag) {
        addr |= 0x8000; // 상위 비트(X비트) 세팅
    }

    int obj = ((opcode_val & 0xFF) << 16) | (addr & 0xFFFF);
    snprintf(objcode, bufsize, "%06X", obj);
    *objlen_out = 3;
    return 1;
}

int p2_assemble_inst(const IntRecord *rec,
                     char *objcode, int bufsize, int *objlen) {
    objcode[0] = '\0';
    if (objlen) *objlen = 0;
    if (!rec || !objcode || !objlen) return -1;

    // 주석 라인, PASS-1에서 이미 에러난 라인
    if (rec->is_comment || rec->errflag != 0)
        return 0;

    // 디렉티브 분기
    if (eq_icase(rec->opcode, "START") ||
        eq_icase(rec->opcode, "END")   ||
        eq_icase(rec->opcode, "BASE")  ||
        eq_icase(rec->opcode, "NOBASE")) {
        return 0; // 오브젝트 코드 없음
    }

    if (eq_icase(rec->opcode, "BYTE")) {
        return assemble_byte(rec->operand, objcode, bufsize, objlen);
    }

    if (eq_icase(rec->opcode, "WORD")) {
        return assemble_word(rec->operand, objcode, bufsize, objlen);
    }

    if (eq_icase(rec->opcode, "RESB") ||
        eq_icase(rec->opcode, "RESW")) {
        // 메모리만 건너뛰는 예약 → 오브젝트 코드 없음, T 레코드 끊는 용도
        return 0;
    }

    // 나머지는 전부 머신 명령어로 보고 처리
    return assemble_format3(rec, objcode, bufsize, objlen);
}

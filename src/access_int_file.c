// access_int_file.c
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "intfile.h"

static FILE *int_fp = NULL;
static int   int_mode = 0; // 0: none, 1: write, 2: read

// 내부용 공백 트림 함수
static void rstrip_newline(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')) {
        s[--len] = '\0';
    }
}

// ---------- PASS-1: write ----------

int int_open_for_write(const char *path)
{
    if (int_fp != NULL) {
        fclose(int_fp);
        int_fp = NULL;
    }

    int_fp = fopen(path, "w");
    if (!int_fp) {
        perror("int_open_for_write: fopen");
        int_mode = 0;
        return 0;
    }

    int_mode = 1;
    return 1;
}

void int_write_record(const IntRecord *rec)
{
    if (!int_fp || int_mode != 1) {
        fprintf(stderr, "int_write_record: file not opened for write\n");
        return;
    }

    if (rec->is_comment) {
        // 주석 라인: ". ..." 형태로 그대로 기록
        if (rec->raw_line[0] != '\0') {
            // raw_line이 이미 ". ..." 형태로 들어있다고 가정
            fprintf(int_fp, "%s\n", rec->raw_line);
        } else {
            // 안전장치: 내용 없으면 최소 '.'만 찍기
            fprintf(int_fp, ".\n");
        }
        return;
    }

    // 일반 라인: "LOC_HEX LABEL OPCODE OPERAND ERRFLAG"
    // LOC는 4자리 16진수(필요시 5자리로 늘려도 됨)
    fprintf(int_fp,
            "%04X %s %s %s %d\n",
            rec->loc,
            rec->label[0]   ? rec->label   : "-",
            rec->opcode[0]  ? rec->opcode  : "-",
            rec->operand[0] ? rec->operand : "-",
            rec->errflag);
}

void int_close_write(void)
{
    if (int_fp && int_mode == 1) {
        fclose(int_fp);
    }
    int_fp   = NULL;
    int_mode = 0;
}

// ---------- PASS-2: read ----------

int int_open_for_read(const char *path)
{
    if (int_fp != NULL) {
        fclose(int_fp);
        int_fp = NULL;
    }

    int_fp = fopen(path, "r");
    if (!int_fp) {
        perror("int_open_for_read: fopen");
        int_mode = 0;
        return 0;
    }

    int_mode = 2;
    return 1;
}

int int_read_record(IntRecord *out)
{
    char line[LINEBUF_LEN];
    char *p;

    if (!int_fp || int_mode != 2) {
        fprintf(stderr, "int_read_record: file not opened for read\n");
        return 0;
    }

    if (fgets(line, sizeof(line), int_fp) == NULL) {
        // EOF
        return 0;
    }

    rstrip_newline(line);

    // raw_line에 통째로 저장 (리스트 파일용)
    strncpy(out->raw_line, line, LINEBUF_LEN - 1);
    out->raw_line[LINEBUF_LEN - 1] = '\0';

    // 앞쪽 공백 스킵
    p = line;
    while (*p && isspace((unsigned char)*p)) p++;

    // 주석 라인인가?
    if (*p == '.') {
        out->is_comment = 1;
        out->loc        = 0;
        out->label[0]   = '\0';
        out->opcode[0]  = '\0';
        out->operand[0] = '\0';
        out->errflag    = 0;
        return 1;
    }

    // 일반 라인: "LOC_HEX LABEL OPCODE OPERAND ERRFLAG"
    {
        unsigned int loc_hex = 0;
        char label[LABEL_LEN];
        char opcode[OPCODE_LEN];
        char operand[OPERAND_LEN];
        int errflag = 0;

        // 기본 형태: 5개 토큰이 나와야 정상
        int n = sscanf(p,
                       "%x %31s %15s %63s %d",
                       &loc_hex,
                       label,
                       opcode,
                       operand,
                       &errflag);

        if (n != 5) {
            fprintf(stderr, "int_read_record: parse error -> '%s'\n", line);
            return -1;
        }

        out->is_comment = 0;
        out->loc        = (int)loc_hex;
        out->errflag    = errflag;

        // "-" 처리: 심볼/오퍼랜드 없는 경우
        if (strcmp(label, "-") == 0) {
            out->label[0] = '\0';
        } else {
            strncpy(out->label, label, LABEL_LEN - 1);
            out->label[LABEL_LEN - 1] = '\0';
        }

        if (strcmp(opcode, "-") == 0) {
            out->opcode[0] = '\0';
        } else {
            strncpy(out->opcode, opcode, OPCODE_LEN - 1);
            out->opcode[OPCODE_LEN - 1] = '\0';
        }

        if (strcmp(operand, "-") == 0) {
            out->operand[0] = '\0';
        } else {
            strncpy(out->operand, operand, OPERAND_LEN - 1);
            out->operand[OPERAND_LEN - 1] = '\0';
        }
    }

    return 1;
}

void int_close_read(void)
{
    if (int_fp && int_mode == 2) {
        fclose(int_fp);
    }
    int_fp   = NULL;
    int_mode = 0;
}

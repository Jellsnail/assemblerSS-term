/*지금은 모든 알 수 없는 opcode를 “3바이트짜리 기계 명령”이라고 가정했다.
나중에 OPTAB 붙이면 여기에서 진짜로 검사하게 바꾸면 된다.*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "assembler.h"

// 16진수 문자열을 int로 파싱 (성공:1, 실패:0)
static int parse_hex(const char *s, int *out) {
    if (!s || !*s) return 0;
    char *endptr = NULL;
    long v = strtol(s, &endptr, 16);
    if (*endptr != '\0') return 0;
    if (v < 0 || v > 0xFFFF) return 0;
    *out = (int)v;
    return 1;
}

// 10진수 정수 파싱 (성공:1, 실패:0)
static int parse_dec(const char *s, int *out) {
    if (!s || !*s) return 0;
    char *endptr = NULL;
    long v = strtol(s, &endptr, 10);
    if (*endptr != '\0') return 0;
    if (v < 0 || v > 0x7FFFFFFF) return 0;
    *out = (int)v;
    return 1;
}

// BYTE X'..' 길이 계산 (byte 수, 실패 시 -1)
static int byte_length(const char *operand) {
    if (!operand || operand[0] == '\0') return -1;

    // 예: X'1F2A'
    if (toupper((unsigned char)operand[0]) == 'X' && operand[1] == '\'') {
        const char *p = operand + 2;
        const char *end = strchr(p, '\'');
        if (!end) return -1;

        int hex_count = 0;
        for (const char *q = p; q < end; ++q) {
            char c = (char)toupper((unsigned char)*q);
            if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) {
                return -1;
            }
            hex_count++;
        }
        if (hex_count == 0 || (hex_count % 2) != 0) return -1;
        if (hex_count > 32) return -1; // 명세: 최대 32 hex digits
        return hex_count / 2;
    }

    // (필요하면 나중에 C'..' 처리 추가)
    return -1;
}

int p1_assign_loc(SourceLine *stmt, int *locctr, int *started) {
    stmt->has_loc = 0;

    if (stmt->type != LINE_STATEMENT) {
        return 0; // 빈 줄/코멘트는 LOC 없음
    }

    // opcode 대문자로 정규화
    for (int i = 0; stmt->opcode[i]; ++i) {
        stmt->opcode[i] = (char)toupper((unsigned char)stmt->opcode[i]);
    }

    // 아직 START를 못 만난 상태
    if (!*started) {
        if (strcmp(stmt->opcode, "START") == 0) {
            int start_addr = 0;
            if (stmt->operand[0] != '\0') {
                if (!parse_hex(stmt->operand, &start_addr)) {
                    fprintf(stderr, "라인 %d: START 주소가 잘못됨 [%s]\n",
                            stmt->line_no, stmt->operand);
                    start_addr = 0;
                }
            }
            *locctr = start_addr;
            stmt->loc = *locctr;
            stmt->has_loc = 1;
            *started = 1;

            // ★ 전역 시작 주소/길이 초기화
            g_start_addr  = start_addr;
            g_prog_length = 0;

            return 0;
        } else {
            // START 없이 시작하면 0부터
            *locctr = 0;
            *started = 1;
            // ★ START 없을 때도 기준값 초기화
            g_start_addr  = 0;
            g_prog_length = 0;
        }
    }

    // END 문장은 LOC 부여 x
    if (strcmp(stmt->opcode, "END") == 0) {
        stmt->has_loc = 0;
        return 0;
    }

    // 현재 LOC를 이 줄에 부여
    stmt->loc = *locctr;
    stmt->has_loc = 1;

    int inc = 0;

    if (strcmp(stmt->opcode, "WORD") == 0) {
        inc = 3;
    } else if (strcmp(stmt->opcode, "RESW") == 0) {
        int n = 0;
        if (!parse_dec(stmt->operand, &n)) {
            fprintf(stderr, "라인 %d: RESW 피연산자 오류 [%s]\n",
                    stmt->line_no, stmt->operand);
            n = 0;
        }
        inc = 3 * n;
    } else if (strcmp(stmt->opcode, "RESB") == 0) {
        int n = 0;
        if (!parse_dec(stmt->operand, &n)) {
            fprintf(stderr, "라인 %d: RESB 피연산자 오류 [%s]\n",
                    stmt->line_no, stmt->operand);
                    n = 0;
        }
        inc = n;
    } else if (strcmp(stmt->opcode, "BYTE") == 0) {
        int len = byte_length(stmt->operand);
        if (len < 0) {
            fprintf(stderr, "라인 %d: BYTE 피연산자(hex string) 오류 [%s]\n",
                    stmt->line_no, stmt->operand);
            len = 0;
        }
        inc = len;
    } else if (strcmp(stmt->opcode, "START") == 0) {
        inc = 0; // 이 경우는 사실상 안 옴
    } else {
        // 나머지는 전부 SIC 명령어라고 가정 (3바이트)
        inc = 3;
    }

    int new_loc = *locctr + inc;
    if (new_loc > 0x10000) { // 주소가 FFFF를 넘으면 경고
        fprintf(stderr, "라인 %d: 주소가 FFFF를 초과함 (LOC=%04X -> %04X)\n",
                stmt->line_no, *locctr, new_loc);
    }

    // ★ 프로그램 길이 갱신 (END는 위에서 미리 return해서 여기 안 옴)
    if (*started) {
        g_prog_length = new_loc - g_start_addr;
        if (g_prog_length < 0) g_prog_length = 0;   // 안전장치
    }

    *locctr = new_loc;
    return 0;
}

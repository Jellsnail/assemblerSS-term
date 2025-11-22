/* 지금 단계에선:

label 규칙(1~6, 첫 글자 A~Z, 나머지 A~Z/0~9) 적용

첫 단어가 label 규칙을 만족하면 label + opcode + operand, 아니면 opcode + operand

줄 맨 앞이 . 이면 코멘트 라인

operand는 opcode 뒤의 남은 부분 전체(앞뒤 공백 제거)로 취급
→ BYTE X'0F' 같은 것도 그대로 operand에 들어간다. */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "assembler.h"

/* label 규칙: 1~6글자, 첫 글자 A~Z, 나머지 A~Z/0~9 */
static int is_valid_label(const char *s) {
    int len = (int)strlen(s);
    if (len < 1 || len > 6) return 0;
    if (!isalpha((unsigned char)s[0]) || !isupper((unsigned char)s[0])) return 0;

    for (int i = 1; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        if (!isupper(c) && !isdigit(c)) return 0;
    }
    return 1;
}

/* 현재는 간단히 SIC opcode + directive만 넣어둔 테이블 */
//>>>>>임시 테이블<<<<
static int is_mnemonic(const char *s) {
    static const char *table[] = {
        // SIC 명령어 (교과서 기준 26개)
        "ADD","AND","COMP","DIV","J","JEQ","JGT","JLT","JSUB",
        "LDA","LDCH","LDL","LDX","MUL","OR","RD","RSUB","STA",
        "STCH","STL","STSW","STX","SUB","TD","TIX","WD",
        // assembler directive
        "BYTE","WORD","RESB","RESW","START","END"
    };
    const int n = sizeof(table)/sizeof(table[0]);
    for (int i = 0; i < n; ++i) {
        if (strcmp(s, table[i]) == 0) return 1;
    }
    return 0;
}

static void rstrip_newline(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[--len] = '\0';
    }
}

static void trim_spaces(char *s) {
    char *p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    memmove(s, p, strlen(p) + 1);

    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[--len] = '\0';
    }
}

/* 한 줄을 규칙에 따라 파싱 */
static void parse_line(const char *line, SourceLine *out) {
    char buf[MAX_LINE_LEN];

    // ⚠️ 여기서는 out을 memset으로 지우지 않는다.
    // (line_no 등은 호출자가 채워 둔 값을 유지해야 함)

    strncpy(out->original, line, MAX_LINE_LEN - 1);
    out->original[MAX_LINE_LEN - 1] = '\0';

    strncpy(buf, line, MAX_LINE_LEN - 1);
    buf[MAX_LINE_LEN - 1] = '\0';
    rstrip_newline(buf);

    // 공백/탭만 있는지
    int only_ws = 1;
    for (const char *p = buf; *p; ++p) {
        if (!isspace((unsigned char)*p)) {
            only_ws = 0;
            break;
        }
    }
    if (only_ws) {
        out->type = LINE_EMPTY;
        return;
    }

    char *p = buf;
    while (*p && isspace((unsigned char)*p)) p++;

    // '.'로 시작하면 코멘트 라인
    if (*p == '.') {
        out->type = LINE_COMMENT;
        strncpy(out->comment, p, MAX_COMMENT_LEN - 1);
        out->comment[MAX_COMMENT_LEN - 1] = '\0';
        trim_spaces(out->comment);
        return;
    }

    out->type = LINE_STATEMENT;

    // 첫 단어 분리
    char first[32] = {0};
    char *f = first;
    while (*p && !isspace((unsigned char)*p)) {
        if ((f - first) < (int)sizeof(first) - 1) {
            *f++ = *p;
        }
        p++;
    }
    *f = '\0';

    // 첫 단어 대문자
    for (int i = 0; first[i]; ++i) {
        first[i] = (char)toupper((unsigned char)first[i]);
    }

    while (*p && isspace((unsigned char)*p)) p++;

    // 1순위: 이것이 opcode/directive 이름이면 => label 없음
    if (is_mnemonic(first)) {
        out->label[0] = '\0';
        strncpy(out->opcode, first, MAX_OPCODE_LEN - 1);
        out->opcode[MAX_OPCODE_LEN - 1] = '\0';

        char operand_buf[MAX_OPERAND_LEN];
        strncpy(operand_buf, p, MAX_OPERAND_LEN - 1);
        operand_buf[MAX_OPERAND_LEN - 1] = '\0';
        trim_spaces(operand_buf);
        strncpy(out->operand, operand_buf, MAX_OPERAND_LEN - 1);
        out->operand[MAX_OPERAND_LEN - 1] = '\0';
        return;
    }

    // 2순위: label 규칙 만족하면 label 로 취급
    if (is_valid_label(first)) {
        strncpy(out->label, first, MAX_LABEL_LEN - 1);
        out->label[MAX_LABEL_LEN - 1] = '\0';

        // 두 번째 단어 = opcode
        char op[32] = {0};
        char *q = op;
        while (*p && !isspace((unsigned char)*p)) {
            if ((q - op) < (int)sizeof(op) - 1) {
                *q++ = *p;
            }
            p++;
        }
        *q = '\0';

        for (int i = 0; op[i]; ++i) {
            op[i] = (char)toupper((unsigned char)op[i]);
        }
        strncpy(out->opcode, op, MAX_OPCODE_LEN - 1);
        out->opcode[MAX_OPCODE_LEN - 1] = '\0';

        while (*p && isspace((unsigned char)*p)) p++;

        char operand_buf[MAX_OPERAND_LEN];
        strncpy(operand_buf, p, MAX_OPERAND_LEN - 1);
        operand_buf[MAX_OPERAND_LEN - 1] = '\0';
        trim_spaces(operand_buf);
        strncpy(out->operand, operand_buf, MAX_OPERAND_LEN - 1);
        out->operand[MAX_OPERAND_LEN - 1] = '\0';
        return;
    }

    // 3순위: 그냥 opcode로 취급 (이후 단계에서 에러 처리)
    out->label[0] = '\0';
    strncpy(out->opcode, first, MAX_OPCODE_LEN - 1);
    out->opcode[MAX_OPCODE_LEN - 1] = '\0';

    char operand_buf[MAX_OPERAND_LEN];
    strncpy(operand_buf, p, MAX_OPERAND_LEN - 1);
    operand_buf[MAX_OPERAND_LEN - 1] = '\0';
    trim_spaces(operand_buf);
    strncpy(out->operand, operand_buf, MAX_OPERAND_LEN - 1);
    out->operand[MAX_OPERAND_LEN - 1] = '\0';
}

int p1_read_source(const char *src_path, FILE *interm_fp) {
    FILE *fp = fopen(src_path, "r");
    if (!fp) {
        perror("failed to open source file");
        return 1;
    }

    char line[MAX_LINE_LEN];
    int line_no = 0;
    SourceLine stmt;
    int locctr = 0;
    int started = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        line_no++;

        // ⚠️ 여기서 한 번만 전체 초기화
        memset(&stmt, 0, sizeof(stmt));
        stmt.line_no = line_no;

        parse_line(line, &stmt);

        // LOC 계산
        p1_assign_loc(&stmt, &locctr, &started);

        // SYMTAB에 label 등록
        p1_assign_sym(&stmt);

        // 중간파일 기록
        switch (stmt.type) {
        case LINE_EMPTY:
            fprintf(interm_fp, "%4d |          | (blank)\n", stmt.line_no);
            break;
        case LINE_COMMENT:
            fprintf(interm_fp, "%4d |          | COMMENT | %s\n",
                    stmt.line_no, stmt.comment);
            break;
        case LINE_STATEMENT:
            if (stmt.has_loc) {
                fprintf(interm_fp,
                        "%4d | %04X | L:%-6s | OP:%-8s | OPR:%s\n",
                        stmt.line_no,
                        stmt.loc & 0xFFFF,
                        stmt.label[0]   ? stmt.label   : "-",
                        stmt.opcode[0]  ? stmt.opcode  : "-",
                        stmt.operand[0] ? stmt.operand : "-");
            } else {
                fprintf(interm_fp,
                        "%4d |      | L:%-6s | OP:%-8s | OPR:%s\n",
                        stmt.line_no,
                        stmt.label[0]   ? stmt.label   : "-",
                        stmt.opcode[0]  ? stmt.opcode  : "-",
                        stmt.operand[0] ? stmt.operand : "-");
            }
            break;
        }
    }

    fclose(fp);
    return 0;
}

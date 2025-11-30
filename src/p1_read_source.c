#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "assembler.h"
#include "intfile.h"   // IntRecord, int_write_record 사용

// 전역 프로그램 정보 (정의)
int g_start_addr  = 0;
int g_prog_length = 0;
char g_progname[MAX_LABEL_LEN] = "NONAME";

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

/*
 * PASS-1 전체 흐름:
 *  - 소스 한 줄 읽기
 *  - parse_line → SourceLine 채우기
 *  - p1_assign_loc → LOC 부여
 *  - p1_assign_sym → SYMTAB 등록
 *  - IntRecord 구성 → int_write_record() 호출
 */
int p1_read_source(const char *src_path) {
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

        memset(&stmt, 0, sizeof(stmt));
        stmt.line_no = line_no;

        parse_line(line, &stmt);

        // 빈 줄은 INTFILE에 기록하지 않아도 됨
        if (stmt.type == LINE_EMPTY) {
            continue;
        }

        // START 라인에서 프로그램 이름 설정
        if (stmt.type == LINE_STATEMENT &&
            strcmp(stmt.opcode, "START") == 0 &&
            stmt.label[0] != '\0') {
            strncpy(g_progname, stmt.label, MAX_LABEL_LEN - 1);
            g_progname[MAX_LABEL_LEN - 1] = '\0';
        }

        int errflag = 0;

        // LOC 계산
        p1_assign_loc(&stmt, &locctr, &started);
        // (필요하면 나중에 p1_assign_loc의 에러를 errflag에 반영 가능)

        // SYMTAB에 label 등록 (중복 라벨 등 에러 체크)
        if (p1_assign_sym(&stmt) != 0) {
            errflag = 1;   // 예: 1 = SYMTAB 관련 에러 (중복 라벨 등)
        }

        // INTFILE에 기록할 레코드 구성
        IntRecord rec;
        memset(&rec, 0, sizeof(rec));

        // 공통: 원본 줄 보관 (리스트 파일에서 사용)
        strncpy(rec.raw_line, stmt.original, LINEBUF_LEN - 1);
        rec.raw_line[LINEBUF_LEN - 1] = '\0';

        if (stmt.type == LINE_COMMENT) {
            // 주석 라인
            rec.is_comment = 1;
            // access_int_file.c의 int_write_record는
            // rec->raw_line만 보고 ". ..." 형태로 저장/판단한다.
            int_write_record(&rec);
        } else if (stmt.type == LINE_STATEMENT) {
            rec.is_comment = 0;
            rec.loc        = stmt.has_loc ? stmt.loc : 0;
            rec.errflag    = errflag;

            // label/opcode/operand가 없으면 "-"로 대체
            if (stmt.label[0] != '\0') {
                strncpy(rec.label, stmt.label, LABEL_LEN - 1);
                rec.label[LABEL_LEN - 1] = '\0';
            } else {
                strcpy(rec.label, "-");
            }

            if (stmt.opcode[0] != '\0') {
                strncpy(rec.opcode, stmt.opcode, OPCODE_LEN - 1);
                rec.opcode[OPCODE_LEN - 1] = '\0';
            } else {
                strcpy(rec.opcode, "-");
            }

            if (stmt.operand[0] != '\0') {
                strncpy(rec.operand, stmt.operand, OPERAND_LEN - 1);
                rec.operand[OPERAND_LEN - 1] = '\0';
            } else {
                strcpy(rec.operand, "-");
            }

            int_write_record(&rec);
        }
    }

    fclose(fp);
    return 0;
}

// p2_write_list.c
#include <stdio.h>
#include <string.h>
#include "intfile.h"
#include "assembler.h"

void p2_write_list_header(FILE *fp)
{
    fprintf(fp, "===== ASSEMBLY LISTING =====\n");
    fprintf(fp, "LOC   OBJCODE     SOURCE\n");
    fprintf(fp, "---------------------------------------------\n");
}

/*
 * listing 출력
 * rec    : INTFILE에서 읽은 한 줄
 * objcode: assemble_inst 결과 (없으면 "")
 * objlen : 객체코드 길이(바이트)
 */
void p2_write_list_line(FILE *fp, const IntRecord *rec,
                        const char *objcode, int objlen)
{
    // 주석 라인은 OBJCODE, LOC 없이 raw_line 그대로 출력
    if (rec->is_comment) {
        fprintf(fp, "           %s\n", rec->raw_line);
        return;
    }

    // SOURCE 컬럼용 문자열 구성: "LABEL OPCODE OPERAND" 형식
    char src[LINEBUF_LEN];
    char tmp[LINEBUF_LEN];

    if (rec->label[0] != '\0') {
        if (rec->operand[0] != '\0') {
            snprintf(tmp, sizeof(tmp), "%-6s %-8s %s",
                     rec->label, rec->opcode, rec->operand);
        } else {
            snprintf(tmp, sizeof(tmp), "%-6s %s",
                     rec->label, rec->opcode);
        }
    } else {
        if (rec->operand[0] != '\0') {
            snprintf(tmp, sizeof(tmp), "       %-8s %s",
                     rec->opcode, rec->operand);
        } else {
            snprintf(tmp, sizeof(tmp), "       %s",
                     rec->opcode);
        }
    }

    strncpy(src, tmp, sizeof(src) - 1);
    src[sizeof(src) - 1] = '\0';

    // PASS-1 에러 플래그가 있다면 끝에 표시 (지금은 거의 0일 것)
    if (rec->errflag != 0) {
        size_t len = strlen(src);
        snprintf(src + len, sizeof(src) - len,
                 "   *** ERROR %d ***", rec->errflag);
    }

    // 에러나 OBJCODE 없는 경우: OBJCODE 칸 비우기
    if (rec->errflag != 0 || objlen <= 0) {
        fprintf(fp, "%04X              %s\n", rec->loc, src);
    } else {
        fprintf(fp, "%04X  %-10s  %s\n", rec->loc, objcode, src);
    }
}

void p2_write_list_footer(FILE *fp)
{
    fprintf(fp, "---------------------------------------------\n");
    fprintf(fp, "===== END OF LISTING =====\n");
}

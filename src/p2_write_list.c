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

    // PASS-1 에러는 LOC/OBJ 없이 에러 메시지 출력
    if (rec->errflag != 0) {
        fprintf(fp, "%04X            %-20s   *** ERROR %d ***\n",
                rec->loc, rec->raw_line, rec->errflag);
        return;
    }

    // 일반 라인
    if (objlen > 0) {
        // 오브젝트 코드 있는 경우
        fprintf(fp, "%04X  %-10s  %s\n",
                rec->loc, objcode, rec->raw_line);
    } else {
        // OBJ 없음 (RESB/RESW/START/END 등)
        fprintf(fp, "%04X              %s\n",
                rec->loc, rec->raw_line);
    }
}

void p2_write_list_footer(FILE *fp)
{
    fprintf(fp, "---------------------------------------------\n");
    fprintf(fp, "===== END OF LISTING =====\n");
}

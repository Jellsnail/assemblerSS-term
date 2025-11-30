// p2_write_obj.c
#include <stdio.h>
#include <string.h>
#include "intfile.h"
#include "assembler.h"

static char t_record[1024];
static int  t_start_addr = 0;
static int  t_length = 0;
static int  t_active = 0;

static void flush_t_record(FILE *fp)
{
    if (!t_active) return;

    fprintf(fp, "T%06X%02X%s\n", t_start_addr, t_length, t_record);

    t_record[0] = '\0';
    t_active = 0;
    t_length = 0;
}

static void start_t_record(int loc)
{
    t_active = 1;
    t_start_addr = loc;
    t_record[0] = '\0';
    t_length = 0;
}


void p2_write_obj_add(FILE *fp, int loc, const char *objcode, int objlen)
{
    // 객체코드 없으면 (RESB/RESW/START/END) → 현재 T 레코드 강제 종료
    if (objlen == 0) {
        flush_t_record(fp);
        return;
    }

    if (!t_active) {
        start_t_record(loc);
    }

    // 30바이트 초과하면 새 T record 시작
    if (t_length + objlen > 30) {
        flush_t_record(fp);
        start_t_record(loc);
    }

    // T record에 HEX 문자열 추가
    strcat(t_record, objcode);
    t_length += objlen;
}


void p2_write_obj_header(FILE *fp, const char *progname, int start_addr, int length)
{
    char namebuf[7];
    snprintf(namebuf, sizeof(namebuf), "%-6s", progname); // 6 chars
    fprintf(fp, "H%s%06X%06X\n", namebuf, start_addr, length);
}


void p2_write_obj_end(FILE *fp, int first_exec_addr)
{
    // 남아있는 마지막 T record flush
    flush_t_record(fp);

    fprintf(fp, "E%06X\n", first_exec_addr);
}

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include "intfile.h"

#define MAX_LABEL_LEN    8      // 1~6이지만 여유
#define MAX_OPCODE_LEN   16
#define MAX_OPERAND_LEN  80
#define MAX_COMMENT_LEN  80
#define MAX_LINE_LEN     120

typedef enum {
    LINE_EMPTY,
    LINE_COMMENT,
    LINE_STATEMENT
} LineType;

typedef struct {
    int  line_no;
    LineType type;

    char original[MAX_LINE_LEN];         // 원본 한 줄 (리스트 출력용)

    int  loc;           // 이 줄의 LOC 값
    int  has_loc;       // LOC가 유효한 줄인지 여부 (0/1)

    char label[MAX_LABEL_LEN];
    char opcode[MAX_OPCODE_LEN];
    char operand[MAX_OPERAND_LEN];
    char comment[MAX_COMMENT_LEN];       // 코멘트 라인 내용 등
} SourceLine;

/* PASS1 - Step1 */
int p1_read_source(const char *src_path, FILE *interm_fp);

/* PASS1 - Step2: LOC 계산 */
int p1_assign_loc(SourceLine *stmt, int *locctr, int *started);

/* PASS1 - Step3: SYMTAB에 label 등록 */
int p1_assign_sym(SourceLine *stmt);

// PASS-2
int p2_assemble_inst(const IntRecord *rec,
                     char *objcode, int bufsize, int *objlen);

void p2_write_obj_header(FILE *fp, const char *progname, int start_addr, int length);
void p2_write_obj_add(FILE *fp, int loc, const char *objcode, int objlen);
void p2_write_obj_end(FILE *fp, int first_exec_addr);

void p2_write_list_header(FILE *fp);
void p2_write_list_line(FILE *fp, const IntRecord *rec,
                        const char *objcode, int objlen);
void p2_write_list_footer(FILE *fp);


#endif


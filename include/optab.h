#ifndef OPTAB_H
#define OPTAB_H

#include "assembler.h"

/* OPTAB 엔트리 */
typedef struct {
    char name[8];   // mnemonic (대문자)
    int  code;      // 기계 opcode (상위 8비트 값)
} OpTabEntry;

/* 
 * mnemonic 을 OPTAB에서 검색
 *  - 찾으면 1, 못 찾으면 0
 *  - opcode_out != NULL 이면 opcode(hex) 저장
 */
int optab_lookup(const char *mnemonic, int *opcode_out);

/*
 * assembler directive 인지 여부 (BYTE, WORD, RESB, RESW, START, END)
 *  - 맞으면 1, 아니면 0
 */
int optab_is_directive(const char *mnemonic);

#endif

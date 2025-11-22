// optab.h
#ifndef OPTAB_H
#define OPTAB_H

#define OPTAB_MNEMONIC_LEN  16
#define OPTAB_MAX_ENTRIES   128   // 넉넉하게

typedef struct {
    char name[OPTAB_MNEMONIC_LEN];  // 대문자 mnemonic
    int  code;                      // opcode (0x00~0xFF)
} OpTabEntry;

/* 
 * optab.txt 로딩
 *  path: "optab.txt" 같은 경로
 *  반환값: 1 = 성공, 0 = 실패
 */
int p2_optab_init(const char *path);

/*
 * mnemonic으로 opcode 검색
 *  mnemonic: "LDA", "JSUB" 등
 *  opcode_out: 찾으면 opcode 저장
 *  반환값: 1 = 찾음, 0 = 없음
 */
int p2_search_optab(const char *mnemonic, int *opcode_out);

/*
 * 필요하면 해제용. 지금 구현에선 정적 배열이라 하는 일은 없다.
 */
void p2_optab_finalize(void);

#endif // P2_SEARCH_OPTAB_H
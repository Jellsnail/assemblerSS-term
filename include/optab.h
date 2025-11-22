// optab.h
#ifndef OPTAB_H
#define OPTAB_H

#define OPTAB_MNEMONIC_LEN  16
#define OPTAB_MAX_ENTRIES   128   // 넉넉하게

typedef struct {
    char name[OPTAB_MNEMONIC_LEN];  // 대문자 mnemonic
    int  code;                      // opcode (0x00~0xFF)
} OpTabEntry;

/**
 * optab.txt를 읽어 메모리에 적재.
 * path: optab.txt 경로
 * 반환: 1 = 성공, 0 = 실패
 */
int optab_init(const char *path);

/**
 * 메모리 해제용 (현재 구현은 정적 배열이라 할 일 없지만, 형식상 둔다)
 */
void optab_free(void);

/**
 * mnemonic으로 opcode 조회
 * 반환: 1 = 찾음, 0 = 없음
 */
int optab_lookup(const char *mnemonic, int *opcode_out);

#endif // OPTAB_H

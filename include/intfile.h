// intfile.h
#ifndef INTFILE_H
#define INTFILE_H

#define LABEL_LEN    32
#define OPCODE_LEN   16
#define OPERAND_LEN  64
#define LINEBUF_LEN  256

typedef struct {
    int  is_comment;                   // 1: 주석 라인, 0: 일반 라인
    int  loc;                          // 주소 (정수, 내부에서 16진수로 포맷)
    char label[LABEL_LEN];             // 라벨 없으면 "-"
    char opcode[OPCODE_LEN];           // START / LDA / BYTE / ...
    char operand[OPERAND_LEN];         // 없으면 "-"
    int  errflag;                      // PASS-1 에러 여부 (0 정상, 그 외 에러 코드)

    char raw_line[LINEBUF_LEN];        // 필요 시 원본(또는 INTFILE 라인) 저장용
} IntRecord;

// PASS-1: INTFILE 쓰기용
int  int_open_for_write(const char *path);
void int_write_record(const IntRecord *rec);
void int_close_write(void);

// PASS-2: INTFILE 읽기용
int  int_open_for_read(const char *path);
/**
 * 반환값: 1 = 정상 한 줄 읽음
 *         0 = EOF
 *        -1 = 파싱 에러(라인 형식 이상)
 */
int  int_read_record(IntRecord *out);
void int_close_read(void);

#endif // INTFILE_H


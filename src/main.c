#include <stdio.h>
#include <stdlib.h>
#include "assembler.h"
#include "symtab.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "사용법: %s sourcefile\n", argv[0]);
        return 1;
    }

    const char *src_path = argv[1];

    symtab_init();  // SYMTAB 초기화

    FILE *interm_fp = fopen("intermediate.txt", "w");
    if (!interm_fp) {
        perror("failed to open intermediate file");
        return 1;
    }

    if (p1_read_source(src_path, interm_fp) != 0) {
        fprintf(stderr, "PASS1: p1_read_source 단계에서 오류 발생\n");
        fclose(interm_fp);
        return 1;
    }

    fclose(interm_fp);

    // PASS1 끝난 뒤 심볼 테이블 파일로 덤프
    symtab_write_to_file("symtab.txt");

    printf("PASS1 완료: intermediate.txt, symtab.txt 생성.\n");
    return 0;
}


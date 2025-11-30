#include <stdio.h>
#include <stdlib.h>
#include "assembler.h"
#include "symtab.h"
#include "optab.h"
#include "intfile.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "사용법: %s sourcefile\n", argv[0]);
        return 1;
    }

    const char *src_path = argv[1];

    /* ==========================================================
     * PASS 1
     * ========================================================== */
    symtab_init();  // SYMTAB 초기화

    if (!int_open_for_write("intermediate.txt")) {
        fprintf(stderr, "PASS1: intermediate.txt 생성 실패\n");
        return 1;
    }

    if (p1_read_source(src_path) != 0) {
        fprintf(stderr, "PASS1: p1_read_source 단계에서 오류 발생\n");
        int_close_write();
        return 1;
    }

    int_close_write();

    // PASS1 끝난 뒤 SYMTAB 파일 출력
    symtab_write_to_file("symtab.txt");

    printf("PASS1 완료: intermediate.txt, symtab.txt 생성.\n");


    /* ==========================================================
     * PASS 2 초기화
     * ========================================================== */

    if (!p2_optab_init("optab.txt")) {
        fprintf(stderr, "PASS2: OPTAB 로딩 실패\n");
        return 1;
    }

    if (!int_open_for_read("intermediate.txt")) {
        fprintf(stderr, "PASS2: intermediate.txt 열기 실패\n");
        return 1;
    }

    FILE *obj_fp = fopen("out.obj", "w");
    if (!obj_fp) {
        perror("PASS2: out.obj 생성 실패");
        int_close_read();
        return 1;
    }

    FILE *lst_fp = fopen("out.lst", "w");
    if (!lst_fp) {
        perror("PASS2: out.lst 생성 실패");
        int_close_read();
        return 1;
    }

    p2_write_list_header(lst_fp);

    int start_addr = g_start_addr;
    int program_length = g_prog_length;

    p2_write_obj_header(obj_fp, g_progname, start_addr, program_length);


    /* ==========================================================
     * PASS 2 메인 루프
     * ========================================================== */

    IntRecord rec;
    char objcode[64];
    int objlen = 0;
    int pass2_error_count = 0;   // PASS2 에러 개수 카운트

    while (int_read_record(&rec) == 1) {
        int st = p2_assemble_inst(&rec, objcode, sizeof(objcode), &objlen);

        if (st == -1) {
        // PASS2 조립 중 에러 발생
        pass2_error_count++;
        objcode[0] = '\0';
        objlen = 0;
    }
    // st == 0 → 원래부터 오브젝트 없는 줄 (RES/START/END 등)
    // st == 1 → 정상 조립

        // 리스트 파일 출력
        p2_write_list_line(lst_fp, &rec, objcode, objlen);

        // 오브젝트 프로그램에 추가 (T record 처리)
        p2_write_obj_add(obj_fp, rec.loc, objcode, objlen);
    }


    /* ==========================================================
     * PASS 2 종료 처리
     * ========================================================== */

    p2_write_obj_end(obj_fp, start_addr);
    p2_write_list_footer(lst_fp);

    fclose(obj_fp);
    fclose(lst_fp);
    int_close_read();

    printf("PASS2 완료: out.obj, out.lst 생성.\n");

    return 0;
}



#include <stdio.h>
#include <string.h>

#include "assembler.h"
#include "symtab.h"

int p1_assign_sym(SourceLine *stmt) {
    if (stmt->type != LINE_STATEMENT) return 0;
    if (!stmt->has_loc) return 0;           // LOC 없는 줄은 등록 안 함
    if (stmt->label[0] == '\0') return 0;   // label 없으면 패스

    int is_dup = 0;
    if (!symtab_insert(stmt->label, stmt->loc, stmt->line_no, &is_dup)) {
        if (is_dup) {
            fprintf(stderr,
                    "라인 %d: 중복 라벨 '%s'\n",
                    stmt->line_no, stmt->label);
        }
        return 1;   // 에러 발생
    }

    return 0;       // 정상
}

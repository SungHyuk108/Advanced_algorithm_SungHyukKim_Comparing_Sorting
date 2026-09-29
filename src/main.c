/* 정렬 비교 — 실행: make run
 *
 * 등록된 정렬을 훑으며 이론값 표와 강의자료 예제 계수를 찍는다.
 * 어떤 정렬인지는 여기에 적지 않는다. sort.c의 구현 표를 훑을 뿐이다.
 */
#include <stdio.h>

#include "sort.h"

#define RULE "--------------------------------------------------------------------------\n"

/* 이론값 표 — 구현 표에 적어 둔 복잡도를 그대로 옮긴다. */
static void theoryTable(void) {
    printf("알고리즘별 이론값\n");
    printf(RULE);
    printf("%-8s %12s %12s %12s %12s %8s\n", "algo", "best", "average", "worst", "space",
           "stable");
    printf(RULE);
    for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
        const SortAlgorithm *s = &SORT_ALGORITHMS[k];
        printf("%-8s %12s %12s %12s %12s %8s\n", s->name, s->best, s->average, s->worst,
               s->space, s->stable ? "yes" : "no");
    }
}

/* 강의자료가 주제 02~04 내내 쓰는 배열. 병합 22/68, 퀵 25/21이 찍혀 있다.
 * 같은 숫자가 나오면 우리 구현이 강의 구현과 같다는 뜻이다. */
static void lectureCheck(void) {
    static const int SAMPLE[] = {2, 8, 5, 9, 1, 10, 7, 6, 4, 3};
    const size_t n = sizeof(SAMPLE) / sizeof(SAMPLE[0]);

    printf("강의자료 예제 배열: 2 8 5 9 1 10 7 6 4 3\n");
    printf(RULE);
    printf("%-8s %14s %14s %10s\n", "algo", "compares", "moves", "depth");
    printf(RULE);

    for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
        Record a[10];
        for (size_t i = 0; i < n; i++) {
            a[i].key = SAMPLE[i];
            a[i].tag = (int)i;
        }

        SortStats st;
        sortStatsReset(&st);
        SORT_ALGORITHMS[k].sort(a, n, &st);

        printf("%-8s %14zu %14zu %10zu\n", SORT_ALGORITHMS[k].name, st.compares, st.moves,
               st.maxDepth);
    }
    printf("\n강의자료 기댓값: merge 22/68 · quick 25/21\n");
}

int main(void) {
    printf("정렬 비교\n\n");
    theoryTable();
    printf("\n");
    lectureCheck();
    return 0;
}

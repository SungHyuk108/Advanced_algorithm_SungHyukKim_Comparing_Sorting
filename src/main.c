/* 정렬 비교 — 병합 / 퀵 / 힙.
 *
 *   make run                    사람이 읽는 비교 표
 *   ./src/main.out --csv        같은 측정을 CSV로 (tools/plot.py가 쓴다)
 *   ./src/main.out --lecture    강의자료 예제 배열로 계수를 맞춰 본다
 *   ./src/main.out --stability  시드를 바꿔 가며 안정성을 판정한다
 *
 * 무엇을 잴지는 아래 SPECS 한 곳에만 적는다. 어떤 정렬을 잴지는 아예
 * 적지 않는다 — sort.c의 구현 표를 훑을 뿐이다.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "sort.h"

#define RULE "--------------------------------------------------------------------------\n"

/* --- 무엇을 잴 것인가 -------------------------------------------------- */

typedef struct Spec {
    const char *scope; /* kinds: 입력 모양별 · growth: n을 키우며 */
    InputKind kind;
    size_t n;
    int reps;
} Spec;

/* kinds — n을 고정하고 입력 모양만 바꾼다. 이론표의 최선/평균/최악을 만든다.
 * growth — 입력을 무작위로 고정하고 n을 두 배씩 키운다. 증가 차수를 본다.
 *
 * n을 2의 거듭제곱으로 잡은 이유.
 * 병합은 구간을 반으로 가르는데, n이 2의 거듭제곱이 아니면 홀수 구간이 생겨
 * 조각 크기가 들쭉날쭉해진다. 그러면 실측이 이론값과 어긋난다. n = 2^13에서는
 * 바닥까지 완전히 반씩 갈려 이동 횟수가 2n*log2(n) = 212992와 정확히 같아진다.
 * 퀵의 최악도 8192*8191/2 = 33550336과 정확히 맞는다. 이론과 실측을 나란히
 * 놓고 검증하려면 이 성질이 필요하다.
 *
 * 8192로 끊은 이유. 맨 앞 피벗 퀵은 정렬된 입력에서 재귀 깊이가 n까지 자란다.
 * n = 8192면 호출 스택이 약 512KB로 기본 한도(8MB)에 한참 못 미치고, 최악
 * 한 회차가 십여 ms라 반복 측정에도 부담이 없다. */
static const Spec SPECS[] = {
    {"kinds", INPUT_RANDOM, 8192, 15},
    {"kinds", INPUT_SORTED, 8192, 15},
    {"kinds", INPUT_REVERSED, 8192, 15},
    {"kinds", INPUT_FEW_UNIQUE, 8192, 15},
    {"growth", INPUT_RANDOM, 2048, 15},
    {"growth", INPUT_RANDOM, 4096, 15},
    {"growth", INPUT_RANDOM, 8192, 15},
    {"growth", INPUT_RANDOM, 16384, 15},
    {"growth", INPUT_RANDOM, 32768, 15},
    {"growth", INPUT_RANDOM, 65536, 15},
};

static const size_t SPEC_COUNT = sizeof(SPECS) / sizeof(SPECS[0]);

/* 시드를 고정한다. 같은 입력이라야 다른 기계에서도 같은 비교·이동 횟수가 나온다. */
#define INPUT_SEED 20260927u

/* 중복 key가 없는 입력에서는 안정성을 가릴 수 없다. yes로 찍으면
 * "안정 정렬임이 확인됐다"로 오해되므로 따로 표시한다. */
static const char *stableText(const BenchResult *r, const char *unjudgeable) {
    if (!r->judgeable) {
        return unjudgeable;
    }
    return r->stable ? "yes" : "no";
}

/* SPECS를 전부 돌며 측정한다. asCsv가 참이면 CSV 한 줄씩, 거짓이면 표로 찍는다.
 * 재는 순서와 조건은 어느 쪽이든 완전히 같다. */
static void measureAll(int asCsv) {
    for (size_t s = 0; s < SPEC_COUNT; s++) {
        const Spec *spec = &SPECS[s];

        Record *input = (Record *)malloc(spec->n * sizeof(Record));
        if (input == NULL) {
            fprintf(stderr, "입력 배열을 잡지 못했다 (n = %zu)\n", spec->n);
            return;
        }
        makeInput(input, spec->n, spec->kind, INPUT_SEED);

        if (!asCsv) {
            printf("\n[%s] %s · n = %zu\n", spec->scope, inputKindName(spec->kind), spec->n);
            printf(RULE);
            printf("%-8s %10s %14s %14s %9s %7s %7s\n", "algo", "time(ms)", "compares",
                   "moves", "depth", "sorted", "stable");
            printf(RULE);
        }

        for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
            BenchResult r = benchRun(&SORT_ALGORITHMS[k], input, spec->n, spec->reps);

            if (asCsv) {
                printf("%s,%s,%zu,%s,%.4f,%zu,%zu,%zu,%d,%s\n", spec->scope,
                       inputKindName(spec->kind), r.n, r.algo->name, r.millis,
                       r.stats.compares, r.stats.moves, r.stats.maxDepth, r.sorted,
                       stableText(&r, "n/a"));
            } else {
                printf("%-8s %10.3f %14zu %14zu %9zu %7s %7s\n", r.algo->name, r.millis,
                       r.stats.compares, r.stats.moves, r.stats.maxDepth,
                       r.sorted ? "yes" : "NO!", stableText(&r, "-"));
            }
        }

        free(input);
    }
}

/* --- 이론값 표 --------------------------------------------------------- */

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

/* --- 강의자료 예제 배열로 계수 맞춰 보기 --------------------------------- */

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

/* --- 안정성 판정 ------------------------------------------------------- */

/* 안정성은 한 번 돌려서는 결론 낼 수 없다. 두 가지 함정이 있다.
 *   1. 중복 key가 없는 입력이면 어떤 정렬이든 결과가 같아 판정 자체가 불가능하다.
 *   2. 한 번의 시도에서 불안정 정렬이 우연히 원래 순서를 지킬 수 있다.
 * 그래서 중복이 많은 입력을 시드를 바꿔 가며 여러 번 돌리고, 한 번이라도
 * 순서가 깨지면 불안정으로 판정한다. */
#define STABILITY_TRIALS 200
#define STABILITY_N 1024

static void stabilityCheck(void) {
    printf("\n안정성 판정 — 중복 많은 입력(값 8종) · n = %d · 시드 %d개\n", STABILITY_N,
           STABILITY_TRIALS);
    printf(RULE);
    printf("%-8s %12s %22s %10s\n", "algo", "표의 주장", "안정으로 나온 시도", "판정");
    printf(RULE);

    Record *a = (Record *)malloc(STABILITY_N * sizeof(Record));
    if (a == NULL) {
        fprintf(stderr, "안정성 판정용 배열을 잡지 못했다\n");
        return;
    }

    for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
        const SortAlgorithm *algo = &SORT_ALGORITHMS[k];
        int stableCount = 0;

        for (int t = 0; t < STABILITY_TRIALS; t++) {
            makeInput(a, STABILITY_N, INPUT_FEW_UNIQUE, (unsigned)(INPUT_SEED + (unsigned)t));

            SortStats st;
            sortStatsReset(&st);
            algo->sort(a, STABILITY_N, &st);

            if (recordsStable(a, STABILITY_N)) {
                stableCount++;
            }
        }

        printf("%-8s %12s %18d/%-3d %10s\n", algo->name, algo->stable ? "안정" : "불안정",
               stableCount, STABILITY_TRIALS,
               stableCount == STABILITY_TRIALS ? "안정" : "불안정");
    }

    free(a);
}

/* --- 진입점 ------------------------------------------------------------ */

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--csv") == 0) {
        printf("scope,kind,n,algo,millis,compares,moves,depth,sorted,stable\n");
        measureAll(1);
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--lecture") == 0) {
        lectureCheck();
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--stability") == 0) {
        stabilityCheck();
        return 0;
    }

    printf("정렬 비교 — 병합 / 퀵 / 힙\n\n");
    theoryTable();
    printf("\n");
    lectureCheck();
    stabilityCheck();
    measureAll(0);
    printf("\n");
    return 0;
}

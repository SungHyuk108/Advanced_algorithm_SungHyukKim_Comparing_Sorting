/* 등록된 정렬 전부에 같은 시험을 친다.
 *
 * 테스트도 정렬 이름을 적지 않는다. 구현 표를 훑으며 모든 정렬에 같은 것을
 * 물어본다. 새 정렬을 표에 넣으면 시험도 저절로 늘어난다.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "sort.h"

static int failures = 0;

static void check(int ok, const char *what) {
    if (ok) {
        printf("  ok    %s\n", what);
    } else {
        printf("  FAIL  %s\n", what);
        failures++;
    }
}

/* 검증용 오라클. 테스트가 자기 손으로 정렬해 답을 만든다 — 검사받는 코드에
 * 기대지 않아야 "원소를 잃지 않았는가"를 따질 수 있다. */
static void bubbleSortKeys(int *k, size_t n) {
    for (size_t i = 0; i + 1 < n; i++) {
        for (size_t j = 0; j + 1 < n - i; j++) {
            if (k[j] > k[j + 1]) {
                int t = k[j];
                k[j] = k[j + 1];
                k[j + 1] = t;
            }
        }
    }
}

/* 배열 하나를 모든 정렬에 넣어 보고, 결과가 오라클과 같은지 본다. */
static void runCase(const int *keys, size_t n, const char *label) {
    for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
        const SortAlgorithm *algo = &SORT_ALGORITHMS[k];

        Record a[32];
        int expect[32];
        for (size_t i = 0; i < n; i++) {
            a[i].key = keys[i];
            a[i].tag = (int)i;
            expect[i] = keys[i];
        }
        bubbleSortKeys(expect, n);

        SortStats st;
        sortStatsReset(&st);
        algo->sort(a, n, &st);

        int same = 1;
        for (size_t i = 0; i < n; i++) {
            if (a[i].key != expect[i]) {
                same = 0;
            }
        }

        char what[128];
        snprintf(what, sizeof(what), "%s: %s", algo->name, label);
        check(recordsSorted(a, n) && same, what);
    }
}

static void testSortsCorrectly(void) {
    static const int MIXED[] = {2, 8, 5, 9, 1, 10, 7, 6, 4, 3};
    static const int SORTED[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    static const int REVERSED[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    static const int DUPS[] = {3, 1, 3, 2, 1, 2, 3, 1};
    static const int ONE[] = {42};
    static const int TWO[] = {2, 1};

    runCase(MIXED, 10, "섞인 배열");
    runCase(SORTED, 10, "이미 정렬된 배열");
    runCase(REVERSED, 10, "역순 배열");
    runCase(DUPS, 8, "중복이 있는 배열");
    runCase(TWO, 2, "원소 둘");
    runCase(ONE, 1, "원소 하나");
    runCase(ONE, 0, "빈 배열");
}

/* 생성한 입력으로 크기를 바꿔 가며 본다.
 * 손으로 쓴 배열은 경계(0, 1, 홀수 개)를 놓치기 쉽고, 큰 입력에서만 드러나는
 * 실수도 있다. bench의 입력 생성기를 그대로 써서 측정과 같은 입력으로 시험한다. */
static void testGeneratedInputs(void) {
    static const size_t SIZES[] = {0, 1, 2, 3, 7, 64, 1000};
    const size_t sizeCount = sizeof(SIZES) / sizeof(SIZES[0]);
    int allOk = 1;

    for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
        const SortAlgorithm *algo = &SORT_ALGORITHMS[k];

        for (size_t s = 0; s < sizeCount; s++) {
            size_t n = SIZES[s];
            for (int kind = 0; kind < INPUT_KIND_COUNT; kind++) {
                Record *a = (Record *)malloc((n + 1) * sizeof(Record));
                int *expect = (int *)malloc((n + 1) * sizeof(int));
                if (a == NULL || expect == NULL) {
                    free(a);
                    free(expect);
                    check(0, "메모리 할당");
                    return;
                }

                makeInput(a, n, (InputKind)kind, 12345u);
                for (size_t i = 0; i < n; i++) {
                    expect[i] = a[i].key;
                }
                bubbleSortKeys(expect, n);

                SortStats st;
                sortStatsReset(&st);
                algo->sort(a, n, &st);

                int ok = recordsSorted(a, n);
                for (size_t i = 0; i < n; i++) {
                    if (a[i].key != expect[i]) {
                        ok = 0;
                    }
                }
                if (!ok) {
                    char what[160];
                    snprintf(what, sizeof(what), "%s: %s n=%zu 에서 결과가 틀림", algo->name,
                             inputKindName((InputKind)kind), n);
                    check(0, what);
                    allOk = 0;
                }

                free(a);
                free(expect);
            }
        }
    }

    if (allOk) {
        char what[128];
        snprintf(what, sizeof(what), "생성 입력 %zu크기 x %d모양 x %zu정렬 모두 정확",
                 sizeCount, (int)INPUT_KIND_COUNT, SORT_ALGORITHM_COUNT);
        check(1, what);
    }
}

/* 재귀 깊이가 이론과 맞는가.
 * 병합은 입력과 무관하게 log n 급이다 — 정렬된 입력을 넣어도 변하지 않는다. */
static void testDepth(void) {
    /* log2(1000)은 10 남짓이므로 병합의 깊이는 11 정도다. 12로 여유를 둔다. */
    const size_t n = 1000;
    Record *a = (Record *)malloc(n * sizeof(Record));
    SortStats st;
    if (a == NULL) {
        check(0, "메모리 할당");
        return;
    }

    makeInput(a, n, INPUT_SORTED, 1u);
    sortStatsReset(&st);
    mergeSort(a, n, &st);
    {
        char what[128];
        snprintf(what, sizeof(what), "merge: 정렬된 입력에서도 깊이가 log n 급 (측정 %zu)",
                 st.maxDepth);
        check(st.maxDepth <= 12, what);
    }

    /* 퀵은 반대다. 맨 앞 피벗이 정렬된 입력에서 매번 최솟값을 고르므로
     * 구간이 한 칸씩만 줄고, 깊이가 n까지 자란다. */
    makeInput(a, n, INPUT_SORTED, 1u);
    sortStatsReset(&st);
    quickSort(a, n, &st);
    {
        char what[128];
        snprintf(what, sizeof(what), "quick: 정렬된 입력에서 깊이가 n까지 자람 (측정 %zu)",
                 st.maxDepth);
        check(st.maxDepth >= n / 2, what);
    }

    /* 힙은 재귀를 아예 쓰지 않는다. siftDown이 아래로만 내려가는 반복문이라
     * 되돌아올 자리를 기억할 필요가 없다. */
    makeInput(a, n, INPUT_SORTED, 1u);
    sortStatsReset(&st);
    heapSort(a, n, &st);
    {
        char what[128];
        snprintf(what, sizeof(what), "heap: 반복문이라 깊이 1 (측정 %zu)", st.maxDepth);
        check(st.maxDepth == 1, what);
    }

    free(a);
}

/* 구현 표가 주장한 안정성이 실제와 맞는가.
 * 중복이 많은 입력이라야 차이가 드러난다. */
static void testStabilityClaim(void) {
    static const int DUPS[] = {3, 1, 3, 2, 1, 2, 3, 1, 2, 3, 1, 2};
    const size_t n = sizeof(DUPS) / sizeof(DUPS[0]);

    for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
        const SortAlgorithm *algo = &SORT_ALGORITHMS[k];

        Record a[32];
        for (size_t i = 0; i < n; i++) {
            a[i].key = DUPS[i];
            a[i].tag = (int)i;
        }

        SortStats st;
        sortStatsReset(&st);
        algo->sort(a, n, &st);

        char what[128];
        snprintf(what, sizeof(what), "%s: 안정성 주장과 실측이 어긋나지 않음", algo->name);
        /* 안정하다고 주장했으면 반드시 안정해야 한다.
         * 불안정하다고 주장한 쪽이 우연히 안정한 것은 실패가 아니다. */
        check(!algo->stable || recordsStable(a, n), what);
    }
}

/* 강의자료(주제 03·04)에 찍힌 계수와 맞는가.
 * 이게 맞으면 우리 구현이 강의 구현과 같은 알고리즘·같은 계수 규약이라는 뜻이다. */
static void testLectureCounts(void) {
    static const int SAMPLE[] = {2, 8, 5, 9, 1, 10, 7, 6, 4, 3};
    const size_t n = sizeof(SAMPLE) / sizeof(SAMPLE[0]);

    static const struct {
        const char *name;
        size_t compares;
        size_t moves;
    } EXPECTED[] = {
        {"merge", 22, 68},
        {"quick", 25, 21},
    };
    const size_t count = sizeof(EXPECTED) / sizeof(EXPECTED[0]);

    for (size_t e = 0; e < count; e++) {
        const SortAlgorithm *algo = NULL;
        for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
            if (strcmp(SORT_ALGORITHMS[k].name, EXPECTED[e].name) == 0) {
                algo = &SORT_ALGORITHMS[k];
            }
        }

        char what[160];
        if (algo == NULL) {
            snprintf(what, sizeof(what), "%s: 구현 표에 등록됨", EXPECTED[e].name);
            check(0, what);
            continue;
        }

        Record a[10];
        for (size_t i = 0; i < n; i++) {
            a[i].key = SAMPLE[i];
            a[i].tag = (int)i;
        }

        SortStats st;
        sortStatsReset(&st);
        algo->sort(a, n, &st);

        snprintf(what, sizeof(what), "%s: 강의자료 계수 비교 %zu·이동 %zu (측정 %zu·%zu)",
                 algo->name, EXPECTED[e].compares, EXPECTED[e].moves, st.compares, st.moves);
        check(st.compares == EXPECTED[e].compares && st.moves == EXPECTED[e].moves, what);
    }
}

int main(void) {
    printf("정렬 테스트\n");
    testSortsCorrectly();
    testGeneratedInputs();
    testDepth();
    testStabilityClaim();
    testLectureCounts();

    if (failures == 0) {
        printf("\n모두 통과\n");
        return 0;
    }
    printf("\n실패 %d건\n", failures);
    return 1;
}

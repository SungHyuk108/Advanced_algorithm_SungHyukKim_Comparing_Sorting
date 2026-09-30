/* clock_gettime은 POSIX라 -std=c17(순수 ISO)에서는 기본으로 보이지 않는다.
 * 이 매크로를 헤더보다 먼저 정의해야 선언이 드러난다. */
#define _POSIX_C_SOURCE 199309L

#include "bench.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

/* 라이브러리 rand() 대신 같은 종류의 생성기(minstd)를 직접 둔다.
 * 강의자료 주제 04의 nextRandom과 같은 식이다. 시드가 같으면 수열이 같으므로
 * 어느 기계에서 돌려도 같은 입력이 만들어진다 — 비교·이동 횟수가 재현되려면
 * 입력부터 같아야 한다. */
static long long randomState = 1;

static void setSeed(unsigned seed) {
    randomState = (long long)seed;
    if (randomState == 0) {
        randomState = 1;
    }
}

static long long nextRandom(void) {
    randomState = (randomState * 16807) % 2147483647;
    return randomState;
}

const char *inputKindName(InputKind kind) {
    switch (kind) {
        case INPUT_RANDOM: return "random";
        case INPUT_SORTED: return "sorted";
        case INPUT_REVERSED: return "reversed";
        case INPUT_FEW_UNIQUE: return "few-unique";
        default: return "?";
    }
}

void makeInput(Record *a, size_t n, InputKind kind, unsigned seed) {
    setSeed(seed);

    for (size_t i = 0; i < n; i++) {
        a[i].tag = (int)i; /* 입력 순서. 안정성 판정에 쓴다 */
        switch (kind) {
            case INPUT_SORTED:
                a[i].key = (int)i;
                break;
            case INPUT_REVERSED:
                a[i].key = (int)(n - i);
                break;
            case INPUT_FEW_UNIQUE:
                /* 값의 종류를 8개로 줄인다. 같은 key가 잔뜩 생기므로
                 * 안정성 차이가 드러나고, 퀵의 파티션도 한쪽으로 쏠린다. */
                a[i].key = (int)(nextRandom() % 8);
                break;
            case INPUT_RANDOM:
            default:
                a[i].key = (int)(nextRandom() % 1000000);
                break;
        }
    }
}

static double nowMillis(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

BenchResult benchRun(const SortAlgorithm *algo, const Record *input, size_t n, int reps) {
    BenchResult r;
    r.algo = algo;
    r.n = n;
    r.millis = 0.0;
    r.sorted = 0;
    r.stable = 0;
    r.judgeable = 0;
    sortStatsReset(&r.stats);

    Record *work = (Record *)malloc(n * sizeof(Record));
    if (work == NULL) {
        return r;
    }

    /* 워밍업 한 회. 첫 회차는 배열이 아직 캐시에 올라오지 않았고 컨테이너도
     * 덥혀지지 않은 상태에서 돌아 유독 느리다. 재지 않고 버린다. */
    memcpy(work, input, n * sizeof(Record));
    sortStatsReset(&r.stats);
    algo->sort(work, n, &r.stats);

    /* 평균이 아니라 가장 빠른 회차를 남긴다.
     *
     * 측정을 방해하는 것(다른 프로세스, OS의 스케줄링, 공유 vCPU를 나눠 쓰는
     * 이웃)은 시간을 늘리기만 하지 줄이지 못한다. 그래서 최솟값이 이 기계에서
     * 이 정렬이 낼 수 있는 실력에 가장 가깝고, 다시 재도 같은 값이 나온다.
     * 평균을 내면 그때 옆에서 무슨 일이 있었는지까지 같이 재게 된다. */
    double best = -1.0;
    for (int t = 0; t < reps; t++) {
        /* 복사는 시계 밖에서 한다. 복사 비용이 정렬 시간에 섞이면
         * 세 정렬을 같은 조건에서 쟀다고 할 수 없다. */
        memcpy(work, input, n * sizeof(Record));
        sortStatsReset(&r.stats);

        double start = nowMillis();
        algo->sort(work, n, &r.stats);
        double elapsed = nowMillis() - start;

        if (best < 0.0 || elapsed < best) {
            best = elapsed;
        }
    }

    r.millis = (best < 0.0) ? 0.0 : best;
    r.sorted = recordsSorted(work, n);
    r.stable = recordsStable(work, n);
    /* 정렬 뒤 배열에서 본다. 같은 key가 붙어 있으므로 한 번 훑으면 된다. */
    r.judgeable = recordsHaveDuplicates(work, n);

    free(work);
    return r;
}

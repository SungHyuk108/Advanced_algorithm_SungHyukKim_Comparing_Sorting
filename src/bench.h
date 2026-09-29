/* 정렬들을 같은 잣대로 재는 도구.
 *
 * 알고리즘(sort.c와 각 구현)과 측정(bench.c)을 나눠 둔다. 정렬은 자기가
 * 측정당하는 줄 모르고, 측정은 어떤 정렬인지 모른다. 둘을 잇는 것은
 * SortAlgorithm 구조체뿐이다.
 */
#ifndef BENCH_H
#define BENCH_H

#include <stddef.h>

#include "sort.h"

/* 입력 모양. 정렬은 입력에 따라 성능이 크게 달라진다 —
 * 이 네 가지가 이론표의 최선·평균·최악 칸을 실험으로 만들어 내는 장치다. */
typedef enum InputKind {
    INPUT_RANDOM,     /* 무작위 — 평균에 해당 */
    INPUT_SORTED,     /* 이미 정렬됨 — 맨 앞 피벗 퀵의 최악 */
    INPUT_REVERSED,   /* 역순 */
    INPUT_FEW_UNIQUE, /* 중복 많음 — 안정성 차이가 드러나는 유일한 입력 */
    INPUT_KIND_COUNT
} InputKind;

const char *inputKindName(InputKind kind);

/* a[0..n-1]을 kind 모양으로 채운다. tag에는 입력 순서를 넣는다.
 * seed를 고정하면 매번 같은 입력이 나온다 — 그래야 결과가 재현된다. */
void makeInput(Record *a, size_t n, InputKind kind, unsigned seed);

typedef struct BenchResult {
    const SortAlgorithm *algo;
    size_t n;
    double millis;   /* 한 번 도는 데 걸린 시간 (reps회 평균) */
    SortStats stats; /* 마지막 회차의 측정값 */
    int sorted;      /* 결과가 정렬됐는가 — 측정값을 보기 전에 이것부터 본다 */
    int stable;      /* 실제로 안정했는가 (구현 표의 주장이 아니라 실측) */
    int judgeable;   /* 입력에 중복 key가 있어 안정성을 가릴 수 있는가 */
} BenchResult;

/* input을 복사해 reps번 정렬하고 평균 시간을 남긴다. 복사 시간은 빼고 잰다. */
BenchResult benchRun(const SortAlgorithm *algo, const Record *input, size_t n, int reps);

#endif /* BENCH_H */

/* 정렬 비교 과제 — 여러 정렬을 하나의 공통 인터페이스로 묶는다.
 *
 * 부르는 쪽(main.c, 테스트)은 정렬 이름을 적지 않는다. 아래 SORT_ALGORITHMS
 * 표만 훑으면 된다. 정렬을 하나 더 만들면 그 표에 한 줄 넣는 것으로 끝난다.
 */
#ifndef SORT_H
#define SORT_H

#include <stddef.h>

/* 측정에 쓰는 원소.
 * key로 정렬하고 tag에는 입력 순서를 담아 둔다. 정렬 뒤에도 같은 key끼리
 * tag가 오름차순이면 안정 정렬이다. int 배열만으로는 안정성을 볼 수 없어서
 * 원소를 이렇게 잡았다. */
typedef struct Record {
    int key;
    int tag;
} Record;

/* 한 번 정렬하는 동안 모인 측정값.
 * 시간은 바깥에서 재고, 여기에는 시계와 무관하게 몇 번을 돌려도 똑같이
 * 재현되는 값만 담는다. */
typedef struct SortStats {
    size_t compares; /* 원소의 key를 맞대어 본 횟수 */
    size_t moves;    /* 원소를 옮긴 횟수. 대입 한 번이 1, 교환 한 번은 3 */
    size_t maxDepth; /* 재귀 깊이의 최댓값. 반복문만 쓰면 1 */
} SortStats;

/* 정렬 한 가지. 이 구조체가 이 과제의 "인터페이스"다. */
typedef struct SortAlgorithm {
    const char *name;
    const char *best;    /* 최선 시간복잡도 */
    const char *average; /* 평균 */
    const char *worst;   /* 최악 */
    const char *space;   /* 추가 공간 */
    int stable;          /* 안정 정렬이라고 주장하는 값. 테스트가 실측과 맞춰 본다 */
    /* a[0..n-1]을 key 오름차순으로 정렬한다. st가 NULL이면 안 된다. */
    void (*sort)(Record *a, size_t n, SortStats *st);
} SortAlgorithm;

void mergeSort(Record *a, size_t n, SortStats *st);
void quickSort(Record *a, size_t n, SortStats *st);
void heapSort(Record *a, size_t n, SortStats *st);

extern const SortAlgorithm SORT_ALGORITHMS[];
extern const size_t SORT_ALGORITHM_COUNT;

void sortStatsReset(SortStats *st);

/* 결과를 살펴보는 함수들. 정렬 자체와 무관하지만 Record를 다루므로 여기 둔다. */
int recordsSorted(const Record *a, size_t n); /* key가 오름차순인가 */
int recordsStable(const Record *a, size_t n); /* 같은 key끼리 tag 순서가 남았는가 */

/* 같은 key가 하나라도 있는가.
 * 중복이 없으면 안정·불안정을 가릴 근거 자체가 없다 — 어떤 정렬이든 결과가
 * 똑같아지므로, 그때의 "안정함"은 판정이 아니라 우연이다. */
int recordsHaveDuplicates(const Record *a, size_t n);

#endif /* SORT_H */

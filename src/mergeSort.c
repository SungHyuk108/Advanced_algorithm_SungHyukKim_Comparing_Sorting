/* 병합 정렬 — 위치로 나누고, 합칠 때 일한다.
 *
 * 나누기는 공짜다. mid를 구하는 산술 한 줄이라 비교도 이동도 0회다.
 * 대신 결합(merge)에서 전부 일한다. 정렬된 두 절반의 맨 앞끼리만 견주어
 * 작은 쪽을 꺼내므로, 구간 하나를 합치는 비교는 구간 크기를 넘지 않는다.
 *
 * 계수 규약은 강의자료(주제 03)와 맞췄다.
 *   비교 — 두 절반이 모두 남아 있을 때의 맞대보기 한 번
 *   이동 — temp에 쓴 횟수 + temp에서 되돌린 횟수 (그래서 구간마다 2배)
 * 배열 2 8 5 9 1 10 7 6 4 3 에서 비교 22 · 이동 68 이 나오면 강의 구현과 같다.
 */
#include <stdlib.h>

#include "sort.h"

/* 정렬된 a[lo..mid] 와 a[mid+1..hi] 를 하나로 합친다. */
static void merge(Record *a, long lo, long mid, long hi, Record *temp, SortStats *st) {
    long i = lo;      /* 앞 절반에서 아직 안 꺼낸 자리 */
    long j = mid + 1; /* 뒤 절반에서 아직 안 꺼낸 자리 */
    long k = lo;      /* temp에 쓸 자리 */

    /* 양쪽 다 남아 있는 동안만 비교가 일어난다. */
    while (i <= mid && j <= hi) {
        st->compares++;
        if (a[i].key <= a[j].key) { /* <= 라서 같은 값이면 왼쪽이 먼저: 안정 */
            temp[k++] = a[i++];
        } else {
            temp[k++] = a[j++];
        }
        st->moves++;
    }

    /* 한쪽이 바닥나면 남은 쪽은 비교 없이 부어 넣는다. */
    while (i <= mid) {
        temp[k++] = a[i++];
        st->moves++;
    }
    while (j <= hi) {
        temp[k++] = a[j++];
        st->moves++;
    }

    /* 합친 결과를 제자리로 되돌린다. 이 왕복 때문에 이동이 구간 크기의 2배가 된다. */
    for (k = lo; k <= hi; k++) {
        a[k] = temp[k];
        st->moves++;
    }
}

static void mergeSortRec(Record *a, long lo, long hi, Record *temp, SortStats *st,
                         size_t depth) {
    if (depth > st->maxDepth) {
        st->maxDepth = depth;
    }
    if (lo >= hi) { /* 원소 하나면 이미 정렬 */
        return;
    }

    long mid = lo + (hi - lo) / 2; /* 나누기는 여기까지. 비교도 이동도 없다 */
    mergeSortRec(a, lo, mid, temp, st, depth + 1);
    mergeSortRec(a, mid + 1, hi, temp, st, depth + 1);
    merge(a, lo, mid, hi, temp, st);
}

void mergeSort(Record *a, size_t n, SortStats *st) {
    if (n < 2) {
        return;
    }
    /* temp는 처음 한 번만 잡아 재귀 전체가 돌려쓴다. 그래서 깊이가 얼마든
     * 추가 공간은 n개로 고정이다 — 재귀 깊이와 무관한 비용이다. */
    Record *temp = (Record *)malloc(n * sizeof(Record));
    if (temp == NULL) {
        return;
    }
    mergeSortRec(a, 0, (long)n - 1, temp, st, 1);
    free(temp);
}

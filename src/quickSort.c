/* 퀵 정렬 — 값으로 나누고, 나눌 때 일한다.
 *
 * 병합과 뼈대가 같은 재귀인데 일하는 자리가 반대다. 파티션이 피벗의 자리를
 * 확정하면 왼쪽은 모두 피벗보다 작고 오른쪽은 모두 크거나 같다. 두 블록이
 * 최종 배열에서 차지할 구역이 이미 갈렸으므로 결합은 할 일이 없다.
 *
 * 피벗은 강의자료와 같이 구간의 맨 앞 원소를 쓴다. 이 선택 때문에
 * 이미 정렬된 입력이 최악(O(n^2))이 되고, 재귀 깊이가 n까지 자란다.
 * 그 최악을 실험으로 재는 것이 이 과제의 목적 중 하나라 일부러 이대로 둔다.
 * (랜덤 피벗이나 median-of-3로 고치면 최악이 사라져 볼 것이 없어진다)
 *
 * 계수 규약은 강의자료(주제 04)와 맞췄다.
 *   비교 — 피벗과 맞대보기 한 번
 *   이동 — 대입 한 번이 1회. 교환은 세 번의 대입이므로 3회
 * 배열 2 8 5 9 1 10 7 6 4 3 에서 비교 25 · 이동 21 이 나오면 강의 구현과 같다.
 */
#include "sort.h"

/* 교환은 tmp에 담고, 덮어쓰고, 되돌리는 세 번의 대입이다. 그래서 이동 3회로 센다.
 * 병합의 "원소 하나 쓰기 = 1회"와 단위가 같아야 두 정렬의 이동을 견줄 수 있다.
 * 제자리 교환(i==j)은 아무것도 옮기지 않으므로 세지 않는다 — 이것이 없으면
 * "퀵은 정렬된 입력에서 이동 0회"라는 강의자료의 결과가 재현되지 않는다. */
static void swap(Record *a, long i, long j, SortStats *st) {
    if (i == j) {
        return;
    }
    Record t = a[i];
    a[i] = a[j];
    a[j] = t;
    st->moves += 3;
}

/* a[lo]를 피벗으로 삼아 작은 것을 왼쪽으로 모으고, 피벗의 최종 자리를 돌려준다. */
static long partition(Record *a, long lo, long hi, SortStats *st) {
    int pivot = a[lo].key;
    long i = lo; /* 지금까지 확인한 "피벗보다 작은 것"의 마지막 자리 */

    for (long j = lo + 1; j <= hi; j++) {
        st->compares++;
        if (a[j].key < pivot) {
            i++;
            swap(a, i, j, st);
        }
    }
    swap(a, lo, i, st); /* 피벗을 그 경계로 데려온다 */
    return i;
}

static void quickSortRec(Record *a, long lo, long hi, SortStats *st, size_t depth) {
    if (depth > st->maxDepth) {
        st->maxDepth = depth;
    }
    if (lo >= hi) { /* 원소 하나면 이미 정렬 */
        return;
    }

    long p = partition(a, lo, hi, st); /* 일은 여기서 다 한다 */
    quickSortRec(a, lo, p - 1, st, depth + 1);
    quickSortRec(a, p + 1, hi, st, depth + 1);
    /* 결합 없음 — 돌아와서 할 일이 없다 */
}

void quickSort(Record *a, size_t n, SortStats *st) {
    if (n < 2) {
        return;
    }
    quickSortRec(a, 0, (long)n - 1, st, 1);
}

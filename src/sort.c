/* 구현을 담은 표와 공용 헬퍼. 이 파일에는 알고리즘이 없다. */
#include "sort.h"

const SortAlgorithm SORT_ALGORITHMS[] = {
    {"merge", "O(n log n)", "O(n log n)", "O(n log n)", "O(n)", 1, mergeSort},
    {"quick", "O(n log n)", "O(n log n)", "O(n^2)", "O(log n)", 0, quickSort},
};

const size_t SORT_ALGORITHM_COUNT = sizeof(SORT_ALGORITHMS) / sizeof(SORT_ALGORITHMS[0]);

void sortStatsReset(SortStats *st) {
    st->compares = 0;
    st->moves = 0;
    st->maxDepth = 0;
}

int recordsSorted(const Record *a, size_t n) {
    for (size_t i = 1; i < n; i++) {
        if (a[i - 1].key > a[i].key) {
            return 0;
        }
    }
    return 1;
}

int recordsStable(const Record *a, size_t n) {
    for (size_t i = 1; i < n; i++) {
        if (a[i - 1].key == a[i].key && a[i - 1].tag > a[i].tag) {
            return 0;
        }
    }
    return 1;
}

int recordsHaveDuplicates(const Record *a, size_t n) {
    for (size_t i = 1; i < n; i++) {
        if (a[i - 1].key == a[i].key) {
            return 1;
        }
    }
    return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>

#define MAX 10000
#define NUM_BUCKETS 10
#define NUM_THREADS 4

typedef struct {
    int *array;
    int size;
    int capacity;
} Bucket;

int compareInt(const void *a, const void *b) {
    return ((int *)a - ((int *)b));
}

int main() {
    int n = 1000;
    int arr[n];
    Bucket buckets[NUM_BUCKETS * NUM_THREADS]; // Local buckets for each thread


    for (int i = 0; i < NUM_BUCKETS * NUM_THREADS; i++) {
        buckets[i].array = (int *)malloc(sizeof(int) * (n / NUM_BUCKETS));
        buckets[i].size = 0;
        buckets[i].capacity = n / NUM_BUCKETS;
    }


    for (int i = 0; i < n; i++) {
        arr[i] = rand() % MAX;
    }

    double start, end;
   start = omp_get_wtime();


    #pragma omp parallel for num_threads(NUM_THREADS) schedule(static)
    for (int i = 0; i < n; i++) {
        int thread_id = omp_get_thread_num();
        int index = arr[i] / (MAX / NUM_BUCKETS);
        int b_index = thread_id * NUM_BUCKETS + index; // Compute bucket index based on thread ID
        Bucket *b = &buckets[b_index];

        b->array[b->size++] = arr[i];
    }


    #pragma omp parallel for num_threads(NUM_THREADS) schedule(static)
    for (int i = 0; i < NUM_BUCKETS * NUM_THREADS; i++) {
        if (buckets[i].size > 0) {
            qsort(buckets[i].array, buckets[i].size, sizeof(int), compareInt);
        }
    }


    int k = 0;
    for (int b = 0; b < NUM_BUCKETS; b++) {
        for (int t = 0; t < NUM_THREADS; t++) {
            int b_index = t * NUM_BUCKETS + b;
            for (int j = 0; j < buckets[b_index].size; j++) {
                arr[k++] = buckets[b_index].array[j];
            }
        }
    }


    for (int i = 0; i < NUM_BUCKETS * NUM_THREADS; i++) {
        free(buckets[i].array);
    }
 
  end = omp_get_wtime();

    printf("Sorted array: \n");
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n...");

    printf("Time taken using OpenMP with %d threads: %.5f milliseconds\n", NUM_THREADS, (end - start) * 1000);

    return 0;
}

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MAX 10000
#define NUM_BUCKETS 10
#define NUM_THREADS 10

typedef struct {
    int *array;
    int size;
} Bucket;

typedef struct {
    int *array;
    int start;
    int end;
    Bucket *buckets;
} ThreadData;

int compareInt(const void *a, const void *b) {
    return ((int *)a - ((int *)b));
}

void *threadSort(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int range = MAX / NUM_BUCKETS;
    int start = data->start;
    int end = data->end;
    Bucket *buckets = data->buckets;


    for (int i = start; i < end; i++) {
        int index = data->array[i] / range;
        buckets[index].array[buckets[index].size++] = data->array[i];
    }

    for (int i = 0; i < NUM_BUCKETS; i++) {
        qsort(buckets[i].array, buckets[i].size, sizeof(int), compareInt);
    }

    pthread_exit(NULL);
}

int main() {
    int n = 10000;
    int arr[n];
    pthread_t threads[NUM_THREADS];
    ThreadData threadData[NUM_THREADS];
    Bucket buckets[NUM_BUCKETS * NUM_THREADS]; // Separate buckets for each thread
    int len = n / NUM_THREADS;


    for (int i = 0; i < NUM_BUCKETS * NUM_THREADS; i++) {
        buckets[i].array = (int *)malloc(sizeof(int) * (len + 1));
        buckets[i].size = 0;
    }


    for (int i = 0; i < n; i++) {
        arr[i] = rand() % MAX;
    }

    clock_t start, end;
    start = clock();


    for (int i = 0; i < NUM_THREADS; i++) {
        threadData[i].array = arr;
        threadData[i].start = i * len;
        threadData[i].end = (i == NUM_THREADS - 1) ? n : (i + 1) * len;
        threadData[i].buckets = &buckets[i * NUM_BUCKETS];
        pthread_create(&threads[i], NULL, threadSort, (void *)&threadData[i]);

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }


    int k = 0;
    for (int i = 0; i < NUM_BUCKETS * NUM_THREADS; i++) {
        for (int j = 0; j < buckets[i].size; j++) {
            arr[k++] = buckets[i].array[j];
        }
        free(buckets[i].array);
    }

    end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC * 1000;

    printf("Sorted array: \n");
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\nTime taken using %d pthreads: %.5f milliseconds\n", NUM_THREADS, cpu_time_used);

    return 0;
}

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUCKET_SIZE 10
#define ARRAY_SIZE 20
#define INTERVAL 0.1
#define NUM_THREADS 4

struct Node {
    float data;
    struct Node* next;
};

struct Bucket {
    struct Node* head;
    pthread_mutex_t lock;
} buckets[BUCKET_SIZE];

struct ThreadData {
    float* arr;
    int start;
    int end;
};

void initializeBuckets();
void* fillBuckets(void* arg);
void* sortBucket(void* arg);
struct Node* insertionSort(struct Node* list);
void printArray(float arr[], int size);
void addNode(struct Bucket* bucket, float value);
void mergeBuckets(float* arr);

int main() {
    float arr[ARRAY_SIZE] = {
        0.42, 0.32, 0.33, 0.52, 0.37, 0.47, 0.51,
        0.19, 0.91, 0.82, 0.73, 0.12, 0.23, 0.01,
        0.68, 0.97, 0.22, 0.45, 0.55, 0.38
    };

    printf("Original array:\n");
    printArray(arr, ARRAY_SIZE);

    initializeBuckets();

    pthread_t threads[NUM_THREADS];
    struct ThreadData threadData[NUM_THREADS];

    // Split array into parts and create threads for filling buckets
    int length = ARRAY_SIZE / NUM_THREADS;
    for (int i = 0; i < NUM_THREADS; i++) {
        threadData[i].arr = arr;
        threadData[i].start = i * length;
        threadData[i].end = (i + 1) * length;
        if (i == NUM_THREADS - 1) threadData[i].end = ARRAY_SIZE; // Last thread may have more to process

        pthread_create(&threads[i], NULL, fillBuckets, (void*)&threadData[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Sort each bucket using separate threads
    for (int i = 0; i < BUCKET_SIZE; i++) {
        pthread_create(&threads[i % NUM_THREADS], NULL, sortBucket, (void*)&buckets[i]);
        if ((i + 1) % NUM_THREADS == 0 || i == BUCKET_SIZE - 1) {
            // Join threads every NUM_THREADS or on last bucket
            int joinUntil = (i + 1) % NUM_THREADS == 0 ? NUM_THREADS : (i + 1) % NUM_THREADS;
            for (int j = 0; j < joinUntil; j++) {
                pthread_join(threads[j], NULL);
            }
        }
    }

    mergeBuckets(arr);

    printf("Sorted array:\n");
    printArray(arr, ARRAY_SIZE);

    return 0;
}

void initializeBuckets() {
    for (int i = 0; i < BUCKET_SIZE; i++) {
        buckets[i].head = NULL;
        pthread_mutex_init(&buckets[i].lock, NULL);
    }
}

void* fillBuckets(void* arg) {
    struct ThreadData* data = (struct ThreadData*)arg;
    for (int i = data->start; i < data->end; i++) {
        float value = data->arr[i];
        int index = value / INTERVAL;
        addNode(&buckets[index], value);
    }
    pthread_exit(NULL);
}

void* sortBucket(void* arg) {
    struct Bucket* bucket = (struct Bucket*)arg;
    pthread_mutex_lock(&bucket->lock);
    bucket->head = insertionSort(bucket->head);
    pthread_mutex_unlock(&bucket->lock);
    pthread_exit(NULL);
}

struct Node* insertionSort(struct Node* list) {
    struct Node *sorted = NULL, *current = list;
    while (current != NULL) {
        struct Node* next = current->next;
        if (sorted == NULL || sorted->data >= current->data) {
            current->next = sorted;
            sorted = current;
        } else {
            struct Node* temp = sorted;
            while (temp->next != NULL && temp->next->data < current->data) {
                temp = temp->next;
            }
            current->next = temp->next;
            temp->next = current;
        }
        current = next;
    }
    return sorted;
}

void printArray(float arr[], int size) {
    for (int i = 0; i < size; i++) {
        printf("%.2f ", arr[i]);
    }
    printf("\n");
}

void addNode(struct Bucket* bucket, float value) {
    pthread_mutex_lock(&bucket->lock);
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->data = value;
    newNode->next = bucket->head;
    bucket->head = newNode;
    pthread_mutex_unlock(&bucket->lock);
}

void mergeBuckets(float* arr) {
    int idx = 0;
    for (int i = 0; i < BUCKET_SIZE; i++) {
        struct Node* node = buckets[i].head;
        while (node) {
            arr[idx++] = node->data;
            struct Node* temp = node;
            node = node->next;
            free(temp);
        }
    }
}

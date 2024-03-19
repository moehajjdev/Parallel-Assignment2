#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define BUCKET_SIZE 10
#define ARRAY_SIZE 20
#define INTERVAL 0.1

struct Node {
    float data;
    struct Node* next;
};

void bucketSort(float arr[], int n);
struct Node* insertionSort(struct Node* list);
void printArray(float arr[], int n);
void addNode(struct Node** list, float value);

int main() {
    float arr[ARRAY_SIZE] = {
        0.42, 0.32, 0.33, 0.52, 0.37, 0.47, 0.51,
        0.19, 0.91, 0.82, 0.73, 0.12, 0.23, 0.01,
        0.68, 0.97, 0.22, 0.45, 0.55, 0.38
    };

    printf("Original array: \n");
    printArray(arr, ARRAY_SIZE);

    bucketSort(arr, ARRAY_SIZE);

    printf("Sorted array: \n");
    printArray(arr, ARRAY_SIZE);

    return 0;
}

void bucketSort(float arr[], int n) {
    struct Node* buckets[BUCKET_SIZE] = {NULL};

    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        int index = arr[i] / INTERVAL;

        // Critical section to prevent concurrent write access
        #pragma omp critical
        addNode(&buckets[index], arr[i]);
    }

    // Note: Sorting individual buckets is done sequentially here
    for (int i = 0; i < BUCKET_SIZE; i++) {
        buckets[i] = insertionSort(buckets[i]);
    }

    int idx = 0;
    for (int i = 0; i < BUCKET_SIZE; i++) {
        struct Node* node = buckets[i];
        while (node) {
            arr[idx++] = node->data;
            node = node->next;
        }
    }
}

struct Node* insertionSort(struct Node* list) {
    struct Node *k, *nodeList;
    if (list == NULL || list->next == NULL) {
        return list;
    }

    nodeList = list;
    k = list->next;
    nodeList->next = NULL;

    while (k != NULL) {
        struct Node *ptr;
        if (nodeList->data > k->data) {
            struct Node* temp = k;
            k = k->next;
            temp->next = nodeList;
            nodeList = temp;
            continue;
        }

        for (ptr = nodeList; ptr->next != NULL; ptr = ptr->next) {
            if (ptr->next->data > k->data) break;
        }

        struct Node* temp = k;
        k = k->next;
        temp->next = ptr->next;
        ptr->next = temp;
    }

    return nodeList;
}

void printArray(float arr[], int n) {
    for (int i = 0; i < n; i++) {
        printf("%.2f ", arr[i]);
    }
    printf("\n");
}

void addNode(struct Node** list, float value) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->data = value;
    newNode->next = *list;
    *list = newNode;
}

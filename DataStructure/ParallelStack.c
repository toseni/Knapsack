//
// Created by tomas on 18.11.14.
//

#include <stdlib.h>
#include <omp.h>
#include "DataStructure.h"

static stack_node *head;

static omp_lock_t lock;
static int numberOfThreads;

static volatile int *workFlags;

static int _isWorking();

void initStackParallel() {
    head = NULL;

    numberOfThreads = omp_get_num_threads();

    workFlags = (int *) malloc(sizeof(int) * numberOfThreads);
    for (int i = 0; i < numberOfThreads; ++i) {
        workFlags[i] = 0;
    }

    omp_init_lock(&lock);
}

void pushParallel(stack_data data) {
    omp_set_lock(&lock);

    stack_node *tmp = (stack_node *) malloc(sizeof(stack_node));
    if (tmp == NULL) {
        exit(111);
    }

    tmp->data = data;
    tmp->next = head;
    head = tmp;

    omp_unset_lock(&lock);
}

int popParallel(stack_data *element) {
    omp_set_lock(&lock);

    if (head == NULL) {
        workFlags[omp_get_thread_num()] = 0;

        omp_unset_lock(&lock);
        return 0;
    }

    stack_node *tmp = head;
    *element = head->data;
    head = head->next;

    free(tmp);

    omp_unset_lock(&lock);

    return 1;
}

int isEmptyParallel() {
    omp_set_lock(&lock);

    int isEmpty = head == NULL ? 1 : 0;
    if (isEmpty) {
        omp_unset_lock(&lock);
        return _isWorking();
    }

    workFlags[omp_get_thread_num()] = 1;
    omp_unset_lock(&lock);
    return 0;
}

void destroyStack() {
    omp_destroy_lock(&lock);
    free((void*)workFlags);
}

static int _isWorking() {
    for (int i = 0; i < numberOfThreads; ++i) {
        if (workFlags[i]) {
            return 0;
        }
    }

    return 1;
}
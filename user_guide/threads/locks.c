#include <stdio.h>

//
#include <uv.h>

uv_barrier_t blocker;
uv_rwlock_t numlock;
int sharednum;

void reader(void* arg);
void writer(void* arg);

int main(int argc, char* argv[]) {
    uv_barrier_init(&blocker, 4);

    sharednum = 0;
    uv_rwlock_init(&numlock);

    uv_thread_t threads[4];

    int thread_nums[] = {1, 2, 1, 2};
    uv_thread_create(&threads[0], reader, &thread_nums[0]);
    uv_thread_create(&threads[1], reader, &thread_nums[1]);

    uv_thread_create(&threads[2], writer, &thread_nums[2]);
    // uv_thread_create(&threads[3], writer, &thread_nums[3]);

    printf("-------\n");
    uv_thread_join(&threads[0]);
    uv_thread_join(&threads[1]);
    uv_thread_join(&threads[2]);
    // uv_thread_join(&threads[3]);

    // uv_barrier_wait(&blocker);
    // printf("0000000\n");
    // uv_barrier_destroy(&blocker);

    uv_rwlock_destroy(&numlock);
    return 0;
}

void reader(void* arg) {
    int num = *(int*)arg;
    int i;
    for (i = 0; i < 10; i++) {
        uv_rwlock_rdlock(&numlock);
        printf("Reader %d: aquired lock\n", num);
        printf("Reader %d: shared num = %d\n", num, sharednum);
        uv_rwlock_rdunlock(&numlock);
        printf("Reader %d: release lock\n", num);
    }
    printf("reader %d\n", num);
    // uv_barrier_wait(&blocker);
}

void writer(void* arg) {
    int num = *(int*)arg;
    int i;
    for (i = 0; i < 10; i++) {
        uv_rwlock_wrlock(&numlock);
        printf("Writer %d: aquired lock\n", num);
        sharednum += 1;
        printf("Writer %d: incremented share num = %d\n", num, sharednum);
        uv_rwlock_wrunlock(&numlock);
        printf("Writer %d: release lock\n", num);
    }
    printf("writer %d\n", num);
    // uv_barrier_wait(&blocker);
}

/// uv_barrier_wait bug?
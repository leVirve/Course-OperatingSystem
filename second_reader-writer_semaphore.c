/* compiler arguments -pthread -lrt -std=gnu99 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

#define init(x)     sem_init(x, 0, 1)
#define wait        sem_wait
#define signal      sem_post
#define semaphore   sem_t
#define MAX_THREADS 15
#define MICRO_SEC   1000000
#define LONG_WORK   MICRO_SEC * 3.3
#define SHORT_WORK  MICRO_SEC * 1.3

int readercount, writercount;
semaphore rdcnt, wrcnt, mutex;
semaphore r, w;

struct reader_writer {
    long tid;
    double sleep_time;
};

struct timeval start;

void *writer(void *arg)
{
    struct reader_writer *rw_data = (struct reader_writer*) arg;
    wait(&wrcnt);
    writercount++;
    if (writercount == 1)
        wait(&r);
    signal(&wrcnt);

    wait(&w);
    usleep(rw_data->sleep_time);
    signal(&w);

    wait(&wrcnt);
    writercount--;
    if (writercount == 0)
        signal(&r);
    signal(&wrcnt);

    struct timeval timestamp;
    gettimeofday(&timestamp, NULL);
    double diff = timestamp.tv_sec - start.tv_sec + (double)(timestamp.tv_usec - start.tv_usec) / MICRO_SEC;
    printf("Writer thread#%ld @(time: %.1f)\n", rw_data->tid, diff);
    fflush(stdout);
    pthread_exit(NULL);
}

void *reader(void *arg)
{
    struct reader_writer *rw_data = (struct reader_writer*) arg;
    wait(&mutex);
        wait(&r);
            wait(&rdcnt);
            readercount++;
            if (readercount == 1)
                wait(&w);
            signal(&rdcnt);
        signal(&r);
    signal(&mutex);

    usleep(rw_data->sleep_time);

    wait(&rdcnt);
    readercount--;
    if (readercount == 0)
        signal(&w);
    signal(&rdcnt);

    struct timeval timestamp;
    gettimeofday(&timestamp, NULL);
    double diff = timestamp.tv_sec - start.tv_sec + (double)(timestamp.tv_usec - start.tv_usec) / MICRO_SEC;
    printf("Reader thread#%ld @(time: %.1f)\n", rw_data->tid, diff);
    fflush(stdout);
    pthread_exit(NULL);
}

void initialize()
{
    readercount = writercount = 0;
    init(&rdcnt);
    init(&wrcnt);
    init(&mutex);
    init(&r);
    init(&w);
    gettimeofday(&start, NULL);
}

int main (int argc, char *argv[])
{
    pthread_t threads[MAX_THREADS];
    struct reader_writer readers_writers[MAX_THREADS];

    initialize();

    for(long t = 1; t < argc; t++) {
        char rw_type = argv[t][0];
        switch(rw_type) {
        case 'w': case 'W':
            readers_writers[t] = (struct reader_writer) { .tid = t, .sleep_time = isupper(rw_type) ? LONG_WORK : SHORT_WORK };
            pthread_create(&threads[t], NULL, writer, (void *) &readers_writers[t]);
            break;
        case 'r': case 'R':
            readers_writers[t] = (struct reader_writer) { .tid = t, .sleep_time = isupper(rw_type) ? LONG_WORK : SHORT_WORK };
            pthread_create(&threads[t], NULL, reader, (void *) &readers_writers[t]);
            break;
        }
        usleep(MICRO_SEC);
    }
    pthread_exit(NULL);
}

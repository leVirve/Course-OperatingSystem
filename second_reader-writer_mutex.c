/* compiler arguments -pthread -lrt -std=gnu99 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/time.h>

#define init(x)     pthread_mutex_init(x, NULL)
#define wait        pthread_mutex_lock
#define signal      pthread_mutex_unlock
#define semaphore   pthread_mutex_t
#define MAX_THREADS 15
#define MICRO_SEC   1000000
#define LONG_WORK   MICRO_SEC * 3.3
#define SHORT_WORK  MICRO_SEC * 1.3

#define time_diff(x) x.tv_sec - start.tv_sec + (double)(x.tv_usec - start.tv_usec) / MICRO_SEC
#define dump_status(tid)  struct timeval timestamp; \
                    gettimeofday(&timestamp, NULL); \
                    printf("Writer thread#%ld @(time: %.1f)\n", tid, time_diff(timestamp)); \
                    fflush(stdout);

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

    dump_status(rw_data->tid);
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

    dump_status(rw_data->tid);
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
        readers_writers[t] = (struct reader_writer) { .tid = t, .sleep_time = isupper(rw_type) ? LONG_WORK : SHORT_WORK };
        switch(rw_type) {
        case 'w': case 'W':
            pthread_create(&threads[t], NULL, writer, (void *) &readers_writers[t]);
            break;
        case 'r': case 'R':
            pthread_create(&threads[t], NULL, reader, (void *) &readers_writers[t]);
            break;
        }
        usleep(MICRO_SEC);
    }
    pthread_exit(NULL);
}

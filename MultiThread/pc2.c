/**
 * Author: TripleZ
 * Date: 6/27/2018
 * 
 * Use semaphore to solve producer, computer and consumer problem.
 * 
 **/
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 4
#define ITEM_COUNT 8

char buffer1[BUFFER_SIZE] = {0};
char buffer2[BUFFER_SIZE] = {0};

int bf1_in, bf1_out;
int bf2_in, bf2_out;

typedef struct {
    int value;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} sema_t;

sema_t buffer1_mutex, buffer2_mutex;
sema_t bf1_full_cond, bf1_empty_cond;
sema_t bf2_full_cond, bf2_empty_cond;


void sema_init(sema_t *sema, int value) {
    sema->value = value;
    pthread_mutex_init(&sema->mutex, NULL);
    pthread_cond_init(&sema->cond, NULL);
}

void sema_wait(sema_t *sema) {
    pthread_mutex_lock(&sema->mutex);
    while (sema->value <= 0) 
        pthread_cond_wait(&sema->cond, &sema->mutex);
    sema->value--;
    pthread_mutex_unlock(&sema->mutex);
}

void sema_signal(sema_t *sema) {
    pthread_mutex_lock(&sema->mutex);
    ++sema->value;
    pthread_cond_signal(&sema->cond);
    pthread_mutex_unlock(&sema->mutex);
}

int buffer1_is_empty() {
    return bf1_in == bf1_out;
}

int buffer1_is_full() {
    return (bf1_in + 1) % BUFFER_SIZE == bf1_out;
}

int buffer2_is_empty() {
    return bf2_in == bf2_out;
}

int buffer2_is_full() {
    return (bf2_in + 1) % BUFFER_SIZE == bf2_out;
}

char get_item_from_buf1() {
    char item;

    item = buffer1[bf1_out];
    bf1_out = (bf1_out + 1) % BUFFER_SIZE;
    
    return item;
}

void put_item_to_buf1(char item) {
    buffer1[bf1_in] = item;
    bf1_in = (bf1_in + 1) % BUFFER_SIZE;
}

char get_item_from_buf2() {
    char item;

    item = buffer2[bf2_out];
    bf2_out = (bf2_out + 1) % BUFFER_SIZE;
    
    return item;
}

void put_item_to_buf2(char item) {
    buffer2[bf2_in] = item;
    bf2_in = (bf2_in + 1) % BUFFER_SIZE;
}

void *producer (void *args) {
    int i;
    char item;

    for (i = 0; i < ITEM_COUNT; i++) {
        sema_wait(&bf1_empty_cond);
        sema_wait(&buffer1_mutex);

        item = 'a' + i;
        put_item_to_buf1(item);
        printf("\033[0;32mProduce item: %c\033[0m\n", item);

        sema_signal(&buffer1_mutex);
        sema_signal(&bf1_full_cond);
    }
    return NULL;
}

void *computer (void *args) {
    int i;
    char item;
    for (i = 0; i < ITEM_COUNT; i++) {
        sema_wait(&bf1_full_cond);
        sema_wait(&buffer1_mutex);

        item = get_item_from_buf1();
        printf("\033[0;33mComputer get item: %c\033[0m\n", item);
        sema_signal(&buffer1_mutex);
        sema_signal(&bf1_empty_cond);

        // Compute item
        sema_wait(&bf2_empty_cond);
        sema_wait(&buffer2_mutex);
        
        item = item - 32;
        put_item_to_buf2(item);
        printf("\033[0;34mComputer put item: %c\033[0m\n", item);
        sema_signal(&buffer2_mutex);
        sema_signal(&bf2_full_cond);
    }
    return NULL;
    
}

void *consumer (void *args) {
    int i;
    char item;
    for (i = 0; i < ITEM_COUNT; i++) {
        sema_wait(&bf2_full_cond);
        sema_wait(&buffer2_mutex);
        
        item = get_item_from_buf2();
        printf("\033[0;35mConsume item: %c\033[0m\n", item);
        sema_signal(&buffer2_mutex);
        sema_signal(&bf2_empty_cond);
    }
    return NULL;
}

int main (int argc, char *argv[]) {
    pthread_t producer_pid, consumer_pid, computer_pid;

    sema_init(&buffer1_mutex, 1);
    sema_init(&buffer2_mutex, 1);
    sema_init(&bf1_full_cond, 0);
    sema_init(&bf1_empty_cond, BUFFER_SIZE - 1);
    sema_init(&bf2_full_cond, 0);
    sema_init(&bf2_empty_cond, BUFFER_SIZE - 1);

    pthread_create(&producer_pid, NULL, producer, NULL);
    pthread_create(&computer_pid, NULL, computer, NULL);
    pthread_create(&consumer_pid, NULL, consumer, NULL);

    pthread_join(consumer_pid, NULL);

    return 0;
}


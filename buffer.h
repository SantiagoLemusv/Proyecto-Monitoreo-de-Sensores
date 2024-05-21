#ifndef BUFFER_H
#define BUFFER_H

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

// Definición de la estructura del Buffer
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t condProducer;
    pthread_cond_t condConsumer;
    char** dataQueue;
    int bufferSize;
    int front;
    int rear;
    int itemCount;
} Buffer;

// Prototipos de funciones
Buffer* createBuffer(int size);
void destroyBuffer(Buffer* buffer);
void add(Buffer* buffer, char* data);
char* removeItem(Buffer* buffer);

#endif


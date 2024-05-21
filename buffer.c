/**************
// Pontificia Universidad Javeriana
// Autor: Santiago Lemus/Paula Malagon
// Fecha: 21/05/2024
// Materia: Sistemas Operativos
// Tema: Proyecto Sensores
// Fichero: buffer.c 
// Objetivo: Capacidad de utilizar herramientas
para la comunicación y sincronización de procesos e hilo
// Documentación general:
// Este programa implementa la función de monitor de sensores
// Presenta diversas etapas detalladas en los comentarios.
*********************************************/

#include <stdlib.h>  // Librería estándar de C para funciones de memoria y conversión
#include <pthread.h> // Librería para utilizar hilos y sincronización
#include <string.h>  // Librería para funciones de manipulación de cadenas

// Definición de la estructura del buffer
typedef struct {
    char** dataQueue;               // Cola de datos para almacenar punteros a cadenas
    pthread_mutex_t mutex;          // Mutex para sincronizar el acceso al buffer
    pthread_cond_t condProducer;    // Condición para controlar la producción
    pthread_cond_t condConsumer;    // Condición para controlar el consumo
    int bufferSize;                 // Tamaño del buffer
    int front;                      // Índice del frente del buffer
    int rear;                       // Índice de la parte trasera del buffer
    int itemCount;                  // Contador de elementos en el buffer
} Buffer;

// Prototipos de funciones
Buffer* createBuffer(int size);     // Función para crear un buffer
void destroyBuffer(Buffer* buffer); // Función para destruir un buffer
void add(Buffer* buffer, char* data); // Función para agregar un elemento al buffer
char* removeItem(Buffer* buffer);   // Función para eliminar un elemento del buffer

// Constructor del Buffer
Buffer* createBuffer(int size) {
    Buffer* buffer = (Buffer*)malloc(sizeof(Buffer)); // Asignar memoria para un nuevo buffer
    if (buffer == NULL) { // Verificar si la asignación fue exitosa
        return NULL; // Si no, retornar NULL
    }

    buffer->dataQueue = (char**)malloc(size * sizeof(char*)); // Asignar memoria para la cola de datos
    if (buffer->dataQueue == NULL) { // Verificar si la asignación fue exitosa
        free(buffer); // Liberar memoria del buffer
        return NULL; // Retornar NULL si falla la asignación de la cola de datos
    }

    pthread_mutex_init(&buffer->mutex, NULL); // Inicializar el mutex
    pthread_cond_init(&buffer->condProducer, NULL); // Inicializar la condición del productor
    pthread_cond_init(&buffer->condConsumer, NULL); // Inicializar la condición del consumidor
    buffer->bufferSize = size; // Establecer el tamaño del buffer
    buffer->front = 0; // Inicializar el índice del frente
    buffer->rear = -1; // Inicializar el índice trasero
    buffer->itemCount = 0; // Inicializar el contador de elementos

    return buffer; // Retornar el puntero al buffer creado
}

// Destructor del Buffer 
void destroyBuffer(Buffer* buffer) {
    if (buffer != NULL) { // Verificar si el buffer no es NULL
        free(buffer->dataQueue); // Liberar la memoria de la cola de datos
        pthread_mutex_destroy(&buffer->mutex); // Destruir el mutex
        pthread_cond_destroy(&buffer->condProducer); // Destruir la condición del productor
        pthread_cond_destroy(&buffer->condConsumer); // Destruir la condición del consumidor
        free(buffer); // Liberar la memoria del buffer
    }
}

// Agregar un elemento al Buffer
void add(Buffer* buffer, char* data) {
    pthread_mutex_lock(&buffer->mutex); // Bloquear el mutex antes de modificar el buffer
    while (buffer->itemCount >= buffer->bufferSize) { // Esperar si el buffer está lleno
        pthread_cond_wait(&buffer->condProducer, &buffer->mutex); // Esperar hasta que haya espacio
    }
    buffer->rear = (buffer->rear + 1) % buffer->bufferSize; // Calcular el nuevo índice trasero
    buffer->dataQueue[buffer->rear] = strdup(data); // Agregar el dato al buffer (copiar la cadena)
    buffer->itemCount++; // Incrementar el contador de elementos
    pthread_cond_signal(&buffer->condConsumer); // Señalar al consumidor que hay un nuevo elemento
    pthread_mutex_unlock(&buffer->mutex); // Desbloquear el mutex
}

// Eliminar un elemento del Buffer
char* removeItem(Buffer* buffer) {
    pthread_mutex_lock(&buffer->mutex); // Bloquear el mutex antes de modificar el buffer
    while (buffer->itemCount <= 0) { // Esperar si el buffer está vacío
        pthread_cond_wait(&buffer->condConsumer, &buffer->mutex); // Esperar hasta que haya elementos
    }
    char* data = buffer->dataQueue[buffer->front]; // Obtener el dato del frente del buffer
    buffer->front = (buffer->front + 1) % buffer->bufferSize; // Calcular el nuevo índice del frente
    buffer->itemCount--; // Decrementar el contador de elementos
    pthread_cond_signal(&buffer->condProducer); // Señalar al productor que hay espacio disponible
    pthread_mutex_unlock(&buffer->mutex); // Desbloquear el mutex
    return data; // Retornar el dato removido del buffer
}

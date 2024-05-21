/**************
// Pontificia Universidad Javeriana
// Autor: Santiago Lemus/Paula Malagon
// Fecha: 21/05/2024
// Materia: Sistemas Operativos
// Tema: Proyecto Sensores
// Fichero: monitor.c 
// Objetivo: Capacidad de utilizar herramientas
para la comunicación y sincronización de procesos e hilo
// Documentación general:
// Este programa implementa la función de monitor de sensores
// Presenta diversas etapas detalladas en los comentarios.
*********************************************/

#include <stdio.h>      // Librería estándar de entrada/salida
#include <stdlib.h>     // Librería estándar de C para funciones de memoria y conversión
#include <pthread.h>    // Librería para utilizar hilos y sincronización
#include <semaphore.h>  // Librería para utilizar semáforos
#include <unistd.h>     // Librería para utilizar funciones POSIX como sleep y close
#include <fcntl.h>      // Librería para controlar operaciones de archivos
#include <string.h>     // Librería para funciones de manipulación de cadenas

#define BUFFER_SIZE 256 // Definir tamaño del buffer

// Estructura para pasar argumentos a los hilos
typedef struct {
    char* pipeName; // Nombre del pipe
    sem_t* sem;     // Puntero al semáforo
    FILE* fileTemp; // Archivo para datos de temperatura
    FILE* filePh;   // Archivo para datos de pH
} ThreadArgs;

// Función que recopila datos del sensor
void* sensor_data_collector(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg; // Convertir el argumento a la estructura ThreadArgs
    char buffer[BUFFER_SIZE]; // Buffer para almacenar datos leídos
    int pipeFd = open(args->pipeName, O_RDONLY); // Abrir el pipe en modo lectura

    if (pipeFd < 0) { // Verificar si la apertura del pipe falló
        sem_post(args->sem); // Liberar el semáforo
        perror("Error al abrir Pipe"); // Imprimir mensaje de error
        return NULL; // Terminar el hilo
    }

    while (1) { // Bucle infinito para leer datos del pipe
        int bytesRead = read(pipeFd, buffer, BUFFER_SIZE - 1); // Leer datos del pipe
        if (bytesRead <= 0) { // Verificar si la lectura falló o el pipe está vacío
            sleep(10); // Dormir el hilo por 10 segundos
            fprintf(stderr, "Desconexion de Sensor en sistema, reevaluando conexion..\n"); // Imprimir mensaje de desconexión
            sem_post(args->sem); // Liberar el semáforo
            break; // Salir del bucle
        }
        buffer[bytesRead] = '\0'; // Añadir terminador nulo al final de la cadena leída
        fputs(buffer, stdout); // Imprimir los datos leídos al estándar de salida

        // Simulando la división de datos para fines de demostración
        fprintf(args->fileTemp, "Temp:%s\n", buffer); // Escribir datos en el archivo de temperatura
        fflush(args->fileTemp); // Asegurar que los datos se escriben en el archivo de temperatura
        fprintf(args->filePh, "pH:%s\n", buffer); // Escribir datos en el archivo de pH
        fflush(args->filePh); // Asegurar que los datos se escriben en el archivo de pH
    }

    close(pipeFd); // Cerrar el pipe
    return NULL; // Terminar el hilo
}

// Función principal
int main(int argc, char* argv[]) {
    // Verificar si el número de argumentos es incorrecto
    if (argc != 9) {
        fprintf(stderr, "Usage: %s -b bufferSize -t fileTemp -h filePh -p pipeName\n", argv[0]);
        return 1; // Terminar el programa con error
    }

    char* pipeName = NULL; // Inicializar nombre del pipe
    int bufferSize = 0; // Inicializar tamaño del buffer
    char* fileTempName = NULL; // Inicializar nombre del archivo de temperatura
    char* filePhName = NULL; // Inicializar nombre del archivo de pH

    // Procesar argumentos de la línea de comandos
    for (int i = 1; i < argc; i += 2) {
        printf("Processing argument: %s %s\n", argv[i], argv[i + 1]); // Imprimir el argumento actual
        if (strcmp(argv[i], "-p") == 0) {
            pipeName = argv[i + 1]; // Asignar el nombre del pipe
        } else if (strcmp(argv[i], "-b") == 0) {
            bufferSize = atoi(argv[i + 1]); // Asignar el tamaño del buffer
        } else if (strcmp(argv[i], "-t") == 0) {
            fileTempName = argv[i + 1]; // Asignar el nombre del archivo de temperatura
        } else if (strcmp(argv[i], "-h") == 0) {
            filePhName = argv[i + 1]; // Asignar el nombre del archivo de pH
        } else {
            fprintf(stderr, "Argumento invalido: %s\n", argv[i]); // Imprimir mensaje de error
            return 1; // Terminar el programa con error
        }
    }

    printf("Debug: pipeName = %s, bufferSize = %d, fileTempName = %s, filePhName = %s\n",
           pipeName, bufferSize, fileTempName, filePhName); // Imprimir valores de depuración

    // Verificar si algún argumento es NULL o inválido
    if (pipeName == NULL || bufferSize == 0 || fileTempName == NULL || filePhName == NULL) {
        fprintf(stderr, "Usage: %s -b bufferSize -t fileTemp -h filePh -p pipeName\n", argv[0]);
        return 1; // Terminar el programa con error
    }

    sem_t sem; // Declarar semáforo
    sem_init(&sem, 0, 0); // Inicializar el semáforo

    FILE* fileTemp = fopen(fileTempName, "w"); // Abrir archivo de temperatura en modo escritura
    if (!fileTemp) { // Verificar si la apertura del archivo falló
        perror("Error al abrir el archivo de temperatura"); // Imprimir mensaje de error
        sem_destroy(&sem); // Destruir el semáforo
        return 1; // Terminar el programa con error
    }

    FILE* filePh = fopen(filePhName, "w"); // Abrir archivo de pH en modo escritura
    if (!filePh) { // Verificar si la apertura del archivo falló
        perror("Error al abrir el archivo de pH"); // Imprimir mensaje de error
        fclose(fileTemp); // Cerrar archivo de temperatura
        sem_destroy(&sem); // Destruir el semáforo
        return 1; // Terminar el programa con error
    }

    // Inicializar estructura de argumentos
    ThreadArgs args = {pipeName, &sem, fileTemp, filePh};

    pthread_t sensorThread; // Declarar hilo
    pthread_create(&sensorThread, NULL, sensor_data_collector, &args); // Crear hilo para recolector de datos de sensor

    pthread_join(sensorThread, NULL); // Esperar a que el hilo de sensor termine

    sem_destroy(&sem); // Destruir el semáforo

    return 0; // Terminar el programa exitosamente
}

/**************
// Pontificia Universidad Javeriana
// Autor: Santiago Lemus/Paula Malagon
// Fecha: 21/05/2024
// Materia: Sistemas Operativos
// Tema: Proyecto Sensores
// Fichero: sensor.c 
// Objetivo: Capacidad de utilizar herramientas
para la comunicación y sincronización de procesos e hilo
// Documentación general:
// Este programa implementa la función de monitor de sensores
// Presenta diversas etapas detalladas en los comentarios.
*********************************************/

#include <stdio.h>     // Librería estándar de entrada/salida
#include <stdlib.h>    // Librería estándar de C para funciones de memoria y conversión
#include <unistd.h>    // Librería para funciones POSIX como getopt
#include <fcntl.h>     // Librería para operaciones de control de archivos
#include <string.h>    // Librería para funciones de manipulación de cadenas

// Prototipos de funciones
void buffer_init(void** buffer, size_t size);
void buffer_put(void* buffer, const void* data);
void buffer_destroy(void* buffer);

int main(int argc, char* argv[]) {
    FILE* filePh;            // Puntero de archivo para pH
    FILE* fileTemp;          // Puntero de archivo para temperatura
    char* sensorType = NULL; // Tipo de sensor 
    char* timeInterval = NULL; // Intervalo de tiempo 
    char* fileName = NULL;   // Nombre del archivo de datos
    char* pipeName = NULL;   // Nombre del pipe

    // Parsear los argumentos de la línea de comandos
    int opt;
    while ((opt = getopt(argc, argv, "s:t:f:p:")) != -1) {
        switch (opt) {
            case 's':
                sensorType = optarg; // Asignar el tipo de sensor
                break;
            case 't':
                timeInterval = optarg; // Asignar el intervalo de tiempo
                break;
            case 'f':
                fileName = optarg; // Asignar el nombre del archivo de datos
                break;
            case 'p':
                pipeName = optarg; // Asignar el nombre del pipe
                break;
            default:
                // Imprimir mensaje de uso y salir si hay un argumento inválido
                fprintf(stderr, "Uso: %s -s sensorType -t timeInterval -f fileName -p pipeName\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Verificar si el nombre del archivo no ha sido especificado
    if (fileName == NULL) {
        fprintf(stderr, "Error: No se ha especificado un archivo de datos.\n");
        exit(EXIT_FAILURE);
    }

    // Abrir el archivo de datos
    FILE* dataFile = fopen(fileName, "r");
    if (dataFile == NULL) {
        perror("Error al abrir archivo de medidas"); // Imprimir error si la apertura falla
        return 1;
    }

    // Inicializar el buffer
    void* buffer;
    size_t BUFFER_SIZE = 1024; // Tamaño del buffer
    buffer_init(&buffer, BUFFER_SIZE);

    // Abrir el pipe
    int pipeFd = open(pipeName, O_WRONLY);
    if (pipeFd == -1) {
        perror("Error al abrir el pipe"); // Imprimir error si la apertura falla
        fclose(dataFile); // Cerrar el archivo de datos
        buffer_destroy(buffer); // Destruir el buffer
        return 1;
    }

    // Leer líneas del archivo y escribirlas en el pipe
    char line[256];
    while (fgets(line, sizeof(line), dataFile) != NULL) {
        buffer_put(buffer, line); // Añadir la línea al buffer
        ssize_t bytes_written = write(pipeFd, line, strlen(line) + 1); // Escribir la línea en el pipe
        if (bytes_written == -1) {
            perror("Error al escribir en el pipe"); // Imprimir error si la escritura falla
            close(pipeFd); // Cerrar el pipe
            fclose(dataFile); // Cerrar el archivo de datos
            buffer_destroy(buffer); // Destruir el buffer
            return 1;
        }
        fprintf(stderr, "Dato a revisar: %s", line); // Imprimir la línea para depuración
    }

    // Limpiar
    close(pipeFd); // Cerrar el pipe
    fclose(dataFile); // Cerrar el archivo de datos
    buffer_destroy(buffer); // Destruir el buffer

    return 0; // Terminar el programa exitosamente
}

// Función para inicializar el buffer
void buffer_init(void** buffer, size_t size) {
    *buffer = malloc(size); // Asignar memoria para el buffer
    if (*buffer == NULL) {
        perror("Error al inicializar el buffer"); // Imprimir error si la asignación falla
        exit(EXIT_FAILURE); // Terminar el programa con error
    }
}

// Función para añadir datos al buffer
void buffer_put(void* buffer, const void* data) {
    strncat((char*)buffer, (char*)data, sizeof(buffer) - strlen((char*)buffer) - 1); // Añadir los datos al buffer
}

// Función para destruir el buffer
void buffer_destroy(void* buffer) {
    free(buffer); // Liberar la memoria del buffer
}


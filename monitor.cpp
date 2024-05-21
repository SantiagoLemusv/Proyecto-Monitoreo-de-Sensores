/**************
// Pontificia Universidad Javeriana
// Autor: Santiago Lemus/Paula Malagon
// Fecha: 21/05/2024
// Materia: Sistemas Operativos
// Tema: Proyecto Sensores
// Fichero: monitor.cpp
// Objetivo: Monitorización de los sensores y registro de los datos en los archivos correspondientes.
*********************************************/
#include <ctime>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string> 
#include <pthread.h>
#include <semaphore.h>
#include "buffer.h"

//Prototipos de funciones
bool is_float(const std::string& str);
bool is_integer(const std::string& str);
std::string getCurrentTime();
void *h_recolector(void *arg);
void *h_ph(void *arg);
void *h_temperatura(void *arg);

struct ThreadArgs {
    char *filePh;          // Nombre del archivo donde se escribirán los datos pH
    char *fileTemp;        // Nombre del archivo donde se escribirán los datos temperatura
    char *pipeName;        // PipeName para la conexión con el sensor
    Buffer *bufferPh;      // Dato Apuntador al búfer para los datos de pH
    Buffer *bufferTemp;    // Dato Apuntador al búfer para los datos de temperatura
    sem_t sem;             // Semaforo para la sincronización de hilos
};


bool is_float(const std::string& str) {
  try {
    // Intenta convertir la cadena a un float usando std::stof
    std::stof(str);
    return true;
  } catch (const std::invalid_argument& e) {
    // La cadena contiene caracteres no numéricos
    return false;
  } catch (...) {
    // Se captura cualquier otra excepción inesperada dentro del try-catch
    return false;
  }
}

bool is_integer(const std::string& str) {
  try {
    // Intenta convertir la cadena a un int usando std::stoi
    std::stoi(str);
    return true;
  } catch (const std::invalid_argument& e) {
    // La cadena contiene caracteres no numéricos
    return false;
  } catch (...) {
    // Se captura cualquier otra excepción inesperada dentro del try-catch
    return false;
  }
}

std::string getCurrentTime() {
  std::time_t currentTime = std::time(nullptr);
  std::tm* localTime = std::localtime(&currentTime);
  char timeString[20]; 

  // Verifica el resultado de strftime para asegurar la conversion exitosa
  if (std::strftime(timeString, sizeof(timeString), "%H:%M:%S", localTime) > 0) {
    return std::string(timeString);
  } else {
    // En caso de error en la conversion, retorna una cadena vacia
    return "";
  }
}


void *h_recolector(void *arg) {

    ThreadArgs *args = (ThreadArgs *)arg;
    Buffer *bufferPh = args->bufferPh;
    Buffer *bufferTemp = args->bufferTemp;
    char *pipeName = args->pipeName;

    // Abrir el Pipe
    int pipeFd = open(pipeName, O_RDONLY);
    if (pipeFd < 0) {
        sem_post(&args->sem);
        std::cerr << "Error al abrir el Pipe: " << pipeName << std::endl;
        return NULL;
    }
    // Leer datos del pipe
    std::string line;
    while (true) {
        int attempts = 0;
        int bytesRead = 0;
        do {
            char buffer[256];
            bytesRead = read(pipeFd, buffer, sizeof(buffer) - 1);
            if (bytesRead > 0) {
                // Procesar la linea actual y agregar a los buffers
                buffer[bytesRead] = '\0';
                line = buffer;
                // Revisando si la linea es un tipo de valor int
                if (is_integer(line)) {
                    int value = std::stoi(line);
                    if (value >= 0) {
                        bufferTemp->add(line);
                    } else {
                        std::cerr << "Se ha recibido un valor negativo desde el sensor"
                                  << std::endl;
                    }
                }
                    // Revisando si la linea es un  tipo de valor float
                else if (is_float(line)) {
                    float value = std::stof(line);
                    if (value >= 0.0) {
                        bufferPh->add(line);
                    } else {
                        std::cerr << "Se ha recibido un valor negativo desde el sensor"
                                  << std::endl;
                    }
                } else {
                    std::cerr << "Se ha recibido un valor invalido desde el sensor" << std::endl;
                }
                attempts = 0; // Reiniciar el contador de intentos de conexion al pipe

            } else {
               //El sensor se ha desconectado, espera 3 segundos y vuelve a intentar conectarse
                sleep(3);
                attempts++;
                std::cout << "Esperando sensor: " << attempts << " sg"
                          << std::endl;
            }

        } while (bytesRead <= 0 && attempts < 3);

        if (attempts == 3) {
            // El sensor no se conectó después de 3 intentos, terminar el process.
            // Enviando mensajes a los otros hilos para que terminen
            bufferPh->add("-1");
            bufferTemp->add("-1");
            // Borrando pipe y terminando el proceso
            unlink(args->pipeName);
            std::cout << "Finished processing measurements" << std::endl;
            break;
        }
    }
    // Cerrando el pipe
    close(pipeFd);

    return NULL;
}

//escritura Ph
void *h_ph(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    Buffer *bufferPh = args->bufferPh;
    char *fileName = args->filePh;

    int sem_val;
    sem_getvalue(&args->sem, &sem_val);
    // Abrir el archivo de escritura 
    std::ofstream filePh(fileName);
    if (!filePh.is_open()) {
        std::cerr << "Error al abrir el archivo: file-ph.txt" << std::endl;
        return NULL;
    }

    if (sem_val > 0) {
        filePh.close();
        bufferPh->~Buffer();
        return NULL;
    }

    // Escritura de datos del buffer al archivo de lecturas Ph
    std::string data;
    while ((data = bufferPh->remove()) != "-1") {
        float value = std::stof(data);
        if (value >= 8.0 || value <= 6.0) {
            std::cout << "¡Alarma!: el valor de lectura Ph es: " << value << std::endl;
        }
        filePh << value << " " << getCurrentTime() << std::endl;
    }

    // Cerrando archivo de escritura
    filePh.close();
    bufferPh->~Buffer();

    return NULL;
}

//Escritura Temp
void *h_temperatura(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    Buffer *bufferTemp = args->bufferTemp;
    char *fileName = args->fileTemp;

    int sem_val;
    sem_getvalue(&args->sem, &sem_val);
    // Abrir el archivo de escritura
    std::ofstream fileTemp(fileName);
    if (!fileTemp.is_open()) {
        std::cerr << "Error al abrir el archivo: file-temp.txt" << std::endl;
        return NULL;
    }
    if (sem_val > 0) {
        fileTemp.close();
        bufferTemp->~Buffer();
        return NULL;
    }
    // Escribir datos del buffer al archivo
    std::string data;
    while ((data = bufferTemp->remove()) != "-1") {
        int value = std::stoi(data);
        if (value >= 31.6 || value <= 20) {
            std::cout << "¡Alarma!: el valor de lectura temperatura es: " << value << std::endl;
        }
        fileTemp << value << " " << getCurrentTime() << std::endl;
    }

    // Cerrando archivo de escritura
    fileTemp.close();
    bufferTemp->~Buffer();
    return NULL;
}


int main(int argc, char *argv[]) {

    int opt;
    int bufferSize = 0;
    char *fileTemp = nullptr;
    char *filePh = nullptr;
    char *pipeName = nullptr;
    // Argumentos en orden alternable 
    while ((opt = getopt(argc, argv, "b:t:h:p:")) != -1) {
        switch (opt) {
            case 'b':
                bufferSize = atoi(optarg);
                break;
            case 't':
                fileTemp = optarg;
                break;
            case 'h':
                filePh = optarg;
                break;
            case 'p':
                pipeName = optarg;
                break;
            default:
                std::cerr << "Uso: " << argv[0]
                          << " -b bufferSize -t fileTemp -h filePh -p pipeName"
                          << std::endl;
                return 1;
        }
    }

    // Creando el Pipe
    if (mkfifo(pipeName, 0666) < 0) {
        std::cerr << "Error al crear Pipe: " << pipeName << std::endl;
        return 1;
    }

    // Abriendo el pipe
    int pipeFd = open(pipeName, O_RDONLY);
    if (pipeFd < 0) {
        std::cerr << "Error al abrir Pipe: " << pipeName << std::endl;
        return 1;
    }

    // Creando los buffers 
    Buffer bufferPh(bufferSize);
    Buffer bufferTemp(bufferSize);

    ThreadArgs args;
    args.bufferPh = &bufferPh;
    args.bufferTemp = &bufferTemp;
    args.pipeName = pipeName;
    args.fileTemp = fileTemp;
    args.filePh = filePh;
    sem_init(&args.sem, 0, 0);
    // Creando los Hilos
    pthread_t threadRecolector, threadPh, threadTemp;
    pthread_create(&threadRecolector, NULL, h_recolector, &args);
    pthread_create(&threadPh, NULL, h_ph, &args);
    pthread_create(&threadTemp, NULL, h_temperatura, &args);
    // Atando Hilos
    pthread_join(threadRecolector, NULL);
    pthread_join(threadPh, NULL);
    pthread_join(threadTemp, NULL);

    // Cerrando pipe y destruyendo semaphore
    close(pipeFd);
    sem_destroy(&args.sem);
    return 0;
}

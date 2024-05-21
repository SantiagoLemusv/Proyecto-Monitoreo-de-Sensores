/**************
// Pontificia Universidad Javeriana
// Autor: Santiago Lemus/Paula Malagon
// Fecha: 21/05/2024
// Materia: Sistemas Operativos
// Tema: Proyecto Sensores
// Fichero: sensor.cpp 
// Objetivo: Capacidad de utilizar herramientas
para la comunicación y sincronización de procesos e hilo
// Documentación general:
// Este programa implementa la función de monitor de sensores
// Presenta diversas etapas detalladas en los comentarios.
*********************************************/

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>

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
int main(int argc, char *argv[]) {
    int opt;
    int sensorType = 0;
    int timeInterval = 0;
    char* fileName = nullptr;
    char* pipeName = nullptr;
    //Argumentos alternables
    while ((opt = getopt(argc, argv, "s:t:f:p:")) != -1) {
        switch (opt) {
            case 's':
                sensorType = atoi(optarg);
                break;
            case 't':
                timeInterval = atoi(optarg);
                break;
            case 'f':
                fileName = optarg;
                break;
            case 'p':
                pipeName = optarg;
                break;
            default:
                std::cerr << "Uso " << argv[0] << " -s sensorType -t timeInterval -f fileName -p pipeName" << std::endl;
                return 1;
        }
    }

    // Abriendo archivo de lectura
    std::ifstream dataFile(fileName);
    if (!dataFile.is_open()) {
        std::cerr << "Error al abrir el archivo de lectura: " << fileName << std::endl;
        return 1;
    }

    // Abriendo pipe
    int pipeFd;
    do {
        pipeFd = open(pipeName, O_WRONLY | O_NONBLOCK);
        if (pipeFd < 0) {
            std::cerr << "Error al abrir pipe: " << pipeName << ", porfavor espere..." << std::endl;
            sleep(1);
        }
    } while (pipeFd < 0);

    // Leyendo archivo y escribiendo en el pipe
    std::string line;
    while (std::getline(dataFile, line)) {
        // Verificar el tipo de sensor
        if (sensorType == 1) { // Temperatura
            if (is_integer(line)) {
                write(pipeFd, line.c_str(), line.size() + 1);
                std::cerr << line << std::endl;
                sleep(timeInterval);

            }
        } else if (sensorType == 2) { // PH
            if (!is_integer(line)) {
                if (is_float(line)){
                    write(pipeFd, line.c_str(), line.size() + 1);
                    std::cerr << line << std::endl;
                    sleep(timeInterval);
                }

            }
        } else {
            std::cerr << "Tipo de sensor invalido: 1. Temp 2. Ph: " << sensorType << std::endl;
        }
    }

    // Cerrando archivo de lectura y pipe
    dataFile.close();
    close(pipeFd);

    return 0;
}
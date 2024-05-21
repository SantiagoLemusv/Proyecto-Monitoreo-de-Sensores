/*****************************
// Fecha: 21/05/2024
// Materia: Sistemas Operativos
// Tema: Proyecto Sensores
// Fichero: buffer.h
// Objetivo: Capacidad de utilizar herramientas
// para la comunicación y sincronización de procesos e hilo
// Documentación general:
//Archivo de cabezera para el buffer, incluyendo sus datos minimos y metodos.
// Presenta diversas etapas detalladas en los comentarios.
*********************************************/

#ifndef BUFFER_H
#define BUFFER_H

#include <pthread.h>
#include <queue>
#include <stdlib.h>
#include <string>
// Definición de la entity del Buffer
class Buffer {

public:
  ~Buffer();
  Buffer(int size);
  void add(std::string data);
  std::string remove();

private:
  std::queue<std::string> dataQueue;
  pthread_mutex_t mutex;
  pthread_cond_t condConsumer;
  pthread_cond_t condProducer;
  int size;
};
#endif // BUFFER_H

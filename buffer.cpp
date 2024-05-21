/**************
// Pontificia Universidad Javeriana
// Autor: Santiago Lemus/Paula Malagon
// Fecha: 21/05/2024
// Materia: Sistemas Operativos
// Tema: Proyecto Sensores
// Fichero: buffer.cpp 
// Objetivo: Capacidad de utilizar herramientas
para la comunicación y sincronización de procesos e hilo
// Documentación general:
// Este programa implementa la función de los bufferes dentro del programa de Sensores.
// Presenta diversas etapas detalladas en los comentarios.
*********************************************/

#include "buffer.h" 
//Constructor Buffer
Buffer::Buffer(int size) : size(size){
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&condProducer, NULL);
  pthread_cond_init(&condConsumer, NULL);
}
//Destructor Buffer
Buffer::~Buffer() {
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&condProducer);
  pthread_cond_destroy(&condConsumer);
}
//Eliminar Datos del buffer
std::string Buffer::remove() {
  pthread_mutex_lock(&mutex);
  while (dataQueue.empty()) {
      pthread_cond_wait(&condConsumer, &mutex);
  }
  std::string data = dataQueue.front();
  dataQueue.pop();
  pthread_cond_signal(&condProducer);
  pthread_mutex_unlock(&mutex);
  return data;
}
//Añadir datos al Buffer
void Buffer::add(std::string data) {
  pthread_mutex_lock(&mutex);
  while (dataQueue.size() >= size) {
      pthread_cond_wait(&condProducer, &mutex);
  }
  dataQueue.push(data);
  pthread_cond_signal(&condConsumer);
  pthread_mutex_unlock(&mutex);
}
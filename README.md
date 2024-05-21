# Proyecto-Monitoreo-de-Sensores
Simula un sistema de monitoreo de calidad de agua mediante la medición de pH y temperatura.
# Monitoreo de Sensores

Este proyecto es parte del curso de Sistemas Operativos (SIU 4085) en el Departamento de Ingeniería de Sistemas de la Pontificia Universidad Javeriana, realizado durante el período de enero a mayo de 2024.

## Objetivo del Proyecto

El objetivo de este proyecto es capacitar a los estudiantes en el uso de herramientas para la comunicación y sincronización de procesos e hilos. Se emplearán pipes nominales para la comunicación entre procesos y semáforos para sincronizar los hilos. Además, se utilizarán llamadas al sistema para la manipulación de archivos de texto.

## Contexto

El agua es un recurso vital en nuestro planeta. Con una población en aumento, es importante monitorear su calidad para detectar cambios y tomar las acciones requeridas. Este proyecto se centra en la medición de dos parámetros de una reserva de agua: PH y temperatura.

## Descripción General del Sistema a Desarrollar

En este proyecto, se simula la medición de dos parámetros de una reserva de agua (PH y temperatura) a través de sensores. Estos parámetros se envían a un proceso monitor, que los almacena y alerta al usuario si se generan alguna alerta con los indicadores medidos.

### Sensores

Los sensores que miden la temperatura y el PH serán simulados por procesos. Los procesos sensores se invocarán desde el shell de la siguiente forma:

$ ./sensor –s tipo-sensor –t tiempo –f archivo –p pipe nominal

Donde:
- `tipo_sensor`: especifica si el proceso reportará PH (2) o temperatura (1).
- `tiempo`: indica cada cuanto tiempo se enviará la medición del proceso sensor al proceso monitor.
- `archivo`: archivo desde el cual se leerán los valores de las variables.
- `pipe_nominal`: pipe nominal para la comunicación entre el proceso sensor y el proceso monitor.

### Proceso Monitor

El proceso monitor recibe las mediciones de los sensores a través de pipes nominales, las almacena y notifica al usuario si se generan alertas con los indicadores medidos.

$ ./monitor –t tiempo –f archivo –p pipe_nominal

Donde:
- `tiempo`: indica cada cuanto tiempo se realizará la verificación de los valores almacenados.
- `archivo`: archivo en el que se almacenarán las mediciones de los sensores.
- `pipe_nominal`: pipe nominal para la comunicación entre el proceso monitor y los procesos sensores.


### Mecanismos de Comunicación

Se utilizarán pipes nominales para la comunicación entre los procesos sensores y el proceso monitor. Además, se emplearán semáforos para sincronizar los hilos y garantizar la integridad de los datos.

---

## Instrucciones de Uso

1. Clona este repositorio en tu máquina local.
2. Compila los archivos fuente utilizando el compilador de C.
3. Ejecuta el proceso monitor y los procesos sensores desde el terminal, siguiendo las instrucciones proporcionadas en la descripción de cada componente del sistema.
4. Observa la salida del proceso monitor para recibir notificaciones sobre la calidad del agua monitoreada.

---

## Contribuyentes

- [Santiago Lemus](https://github.com/SantiagoLemusv): Desarrollador principal.
- [Paula Malagon](paula2704): Contribuyente.

---


## Licencia

Este proyecto está bajo la Licencia [GNU General Public License (GPL)](https://www.gnu.org/licenses/gpl-3.0.html) de Código Abierto (FOSS).



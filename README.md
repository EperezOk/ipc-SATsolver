# TP 1 SO - README

## Introduction

The aim of this project is learning how to use the different IPC mechanisms present in a POSIX system.
We implemented a system that will distribute SAT solving tasks among several processes.

## Autores

- Baliarda Gonzalo 61490
- Pérez Ezequiel 61475
- Ye Li Valentín 61011

## Aplicación
La aplicación tiene como objetivo resolver problemas de tipo **SAT** y consiste en los siguientes módulos:

**master**: crea los **slaves** y les envía los archivos a resolver. Los datos procesados luego se los comparte a **view** y los escribe en un archivo `results.txt`.
**slave**: realiza el procesamiento de los archivos .CNF mediante **minisat**.
**view**: imprime los resultados por salida estándar.

<center><img src="https://imgur.com/vOO6Squ.png" alt="text"></center>

## Requerimientos
Tener instalado los siguientes programas:
+ **gcc**: para instalarlo, `sudo apt install build-essential`.
+ **minisat**: para instalarlo, `sudo apt install minisat`.

## Compilación
Ejecutar `make` o `make all` en la carpeta base para compilar los archivos. 
> Ver **Requerimientos**

Luego de compilar se generan los siguientes ejecutables:
+ **SATSolver.out**
+ **slave.out**
+ **view.out**

Para remover los archivos creados, ejecutar `make clean` desde el mismo directorio donde se realizo `make all`.

## Ejecución

Para correr el programa ejecutar desde la carpeta base alguna de las siguientes formas:
+ `./SATSolver.out <CNF files> | ./view.out`
+ `./SATSolver.out <CNF files>` y durante la ejecución del programa, `./view.out <ID>`, donde \<ID> es el número que imprime **SATSolver** por salida estándar.

## Resultados
Los resultados se pueden leer de dos maneras:
+ Desde el archivo creado por **master**, `results.txt`.
+ Por salida estándar mientras **master** se esta ejecutando a través de **view** .



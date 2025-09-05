#!/bin/bash

echo "Inicio la creación de archivos intermedios de utils"
cd utils
make clean
make
cd ..
echo "Finalizo la creación de archivos intermedios de utils"

echo "Inicio la creación del ejecutable master"
cd master
make clean
make
cd ..
echo "Finalizo la creación del ejecutable master"

echo "Inicio la creación del ejecutable storage"
cd storage
make clean
make
cd ..
echo "Finalizo la creación del ejecutable storage"

echo "Inicio la creación del ejecutable worker"
cd worker
make clean
make
cd ..
echo "Finalizo la creación del ejecutable worker"

echo "Inicio la creación del ejecutable query_control"
cd query_control
make clean
make
cd ..
echo "Finalizo la creación del ejecutable query_control"

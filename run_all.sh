#!/bin/bash

echo "Inicio la apertura y ejecución de storage"
cd storage
xfce4-terminal --working-directory="$PWD" -e "bash -c './bin/storage ./storage.config; exec bash'"
cd ..
echo "Finalizo la apertura de storage"


echo "Inicio la apertura y ejecución de master"
cd master
xfce4-terminal --working-directory="$PWD" -e "bash -c './bin/master ./master.config; exec bash'"
cd ..
echo "Finalizo la apertura de master"


echo "Inicio la apertura y ejecución de worker"
cd worker
xfce4-terminal --working-directory="$PWD" -e "bash -c './bin/worker ./worker.config intel; exec bash'"
cd ..
echo "Finalizo la apertura de worker"


echo "Inicio la apertura y ejecución de query"
cd query_control
xfce4-terminal --working-directory="$PWD" -e "bash -c './bin/query ./query.config query1.asm 23; exec bash'"
cd ..
echo "Finalizo la apertura de query"

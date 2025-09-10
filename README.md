# master_of_files
Dedicado para el tp de SO de la utn frba


🔍 Para detectar fugas de memoria con Valgrind:
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./tu_programa [args]


--leak-check=full → análisis completo de memoria.

--show-leak-kinds=all → muestra tipos de fuga (definitivas, indirectas, posibles, alcanzables).

--track-origins=yes → indica el origen de errores de memoria (muy útil).

./tu_programa [args] → reemplazá con tu ejecutable y argumentos.

👉 Ejemplo:

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./main config.txt
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/master
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/query_control query_control.config ruta_archivo_query 15
⚡ Para detectar condiciones de carrera con Helgrind:
valgrind --tool=helgrind ./tu_programa [args]


👉 Ejemplo:

valgrind --tool=helgrind ./servidor

# Libraries
LIBS=utils commons pthread readline m crypto

# Custom libraries' paths
SHARED_LIBPATHS=
STATIC_LIBPATHS=../utils

# Compiler flags
CDEBUG=-g -Wall -DDEBUG -fdiagnostics-color=always
CRELEASE=-O3 -Wall -DNDEBUG -g --log-file=valgrind-out.txt

# Source files (*.c) to be excluded from tests compilation
TEST_EXCLUDE=src/main.c

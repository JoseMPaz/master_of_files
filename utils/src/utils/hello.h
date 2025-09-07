#ifndef UTILS_HELLO_H_
#define UTILS_HELLO_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <commons/collections/list.h>
#include <pthread.h>

typedef enum
{
	CLIENTE = 0,
	SERVIDOR = 1
}t_conexion;

typedef enum
{
	DESCONEXION = 0,
	MENSAJE = 1,
	PAQUETE = 2,
	NEW_QUERY = 3,
	END_QUERY = 4
}t_operacion;


typedef struct
{
	int longitud;
	char * flujo;
}t_carga_util;

typedef struct
{
	t_operacion operacion;
	t_carga_util * carga_util;
}t_paquete;

/**
* @brief Imprime un saludo por consola
* @param quien Módulo desde donde se llama a la función
* @return No devuelve nada
*/
void saludar(char* quien);

/**
* @brief Crea un socket
* @param tipo_conexion: CLIENTO|SERVIDOR, ip: IP del servidor, puerto:Puerto del servidor
* @return No devuelve nada
*/
int crear_socket (t_conexion tipo_conexion, const char * ip, const char * puerto);
void * atender_cliente (void * argumento);
t_operacion recibir_operacion(int socket);
void solicitar_atencion (int socket_cliente, char * ip_servidor, char * puerto_servidor);
t_paquete * crear_paquete (t_operacion operacion);
void destruir_paquete (t_paquete * paquete);
void agregar_a_paquete (t_paquete * paquete, void * cadena, int longitud);
void * serializar_paquete (t_paquete * paquete, int bytes_a_enviar);
void enviar_paquete (t_paquete * paquete, int socket);
t_list * recibir_carga_util (int socket);

#endif

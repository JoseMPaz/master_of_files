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
#include <commons/string.h>
#include <signal.h>
#include <semaphore.h>


typedef enum
{
	CLIENTE = 0,
	SERVIDOR = 1
}t_conexion;

typedef enum
{
	DESCONEXION 	= 0,
	
	NEW_QUERY 		= 3,
	READ_QUERY 		= 4,
	NEW_READ 		= 5,
	END_QUERY 		= 6,
	
	NEW_WORKER 		= 7,	
	EXEC_QUERY = 8,
	SIZE_BLOCK
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
* @brief Crea un socket cliente o socket escucha del servidor
* @param tipo_conexion: CLIENTO|SERVIDOR, ip: IP del servidor, puerto:Puerto del servidor
* @return Devuelve el socket que debera cerrarse
*/
int crear_socket (t_conexion tipo_conexion, const char * ip, const char * puerto);

/**
* @brief Con un socket cliente abierto, recibe de el su operacion y carga util, lo procesa
* @param argumento: socket_cliente
* @return Por ahora no devuelve nada, pero en caso sea necesario lo hara
*/


/**
* @brief recibe la operacion del socket
* @param argumento: socket
* @return Devuelve la operacion
*/
t_operacion recibir_operacion(int socket);

/**
* @brief Sirve para conectarse a un servidor por medio del socket_cliente
* @param socket_cliente, ip_servidor y puerto_servidor
* @return No devuelve nada
*/
void solicitar_atencion (int socket_cliente, char * ip_servidor, char * puerto_servidor);

/**
* @brief Crea espacio para el paquete y la carga util y agrega la operacion
* @param operacion
* @return el paquete generado
*/
t_paquete * crear_paquete (t_operacion operacion);

/**
* @brief Elimina espacio del paquete, la carga util y el flujo
* @param operacion
* @return el paquete generado
*/
void destruir_paquete (t_paquete * paquete);

/**
* @brief agrega una cadena y su longitud al paquete, agranda el flujo
* @param paquete, cadena y longitud
* @return No devuelve nada
*/
void agregar_a_paquete (t_paquete * paquete, void * cadena, int longitud);

/**
* @brief Pone en serie la operacion, longitud del flujo y el flujo para ser enviado
* @param paquete y bytes_a_enviar
* @return el paquete serializado
*/
void * serializar_paquete (t_paquete * paquete, int bytes_a_enviar);

/**
* @brief Envia el paquete, serializandolo a traves del socket
* @param paquete y socket
* @return No devuelve nada
*/
void enviar_paquete (t_paquete * paquete, int socket);

/**
* @brief Recibe las cadena y longitudes desde el socket 
* @param paquete y socket
* @return Lista de cadenas
*/
t_list * recibir_carga_util (int socket);




#endif

#ifndef _GESTION_H_
#define _GESTION_H_

#include <utils/hello.h>

#define POSICION_ID 0
#define NO_HAY_WORKER_CONECTADOS "No se pudo atender debido a que no hay workers conectados"

extern t_list * workers; 
extern int socket_escucha;
extern t_config * configuracion;
extern t_log * bitacora_del_sistema;
extern pthread_mutex_t mutex_workers;

typedef struct
{
	char * id;
	int socket;
	bool esta_libre;
}t_worker;

void * gestionar_query_worker (void * socket_de_atencion);
void cerrar_servidor(int signum);
 
#endif

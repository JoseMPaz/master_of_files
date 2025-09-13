#ifndef _GESTION_H_
#define _GESTION_H_

#include <utils/hello.h>

extern t_list* workers; 
extern int socket_escucha;
extern t_config * configuracion;
extern t_log * bitacora_del_sistema;


void * gestionar_query_worker (void * argumento);
void cerrar_servidor(int signum);
 
#endif

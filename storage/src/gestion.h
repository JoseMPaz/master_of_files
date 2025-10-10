#ifndef _GESTION_H_
#define _GESTION_H_

#include <utils/hello.h>

#define TAMANO_DE_BLOQUE "450"


extern t_list* workers; 
extern int socket_escucha;
extern t_config * configuracion;
extern t_log * bitacora_del_sistema;



void * gestionar_worker (void * argumento);

 
#endif

#include "gestion.h"



void * gestionar_worker (void * argumento)
{
	int socket = *(int*)argumento;
	
	free (argumento);
	   
	int operacion = recibir_operacion (socket);
	
	switch (operacion) 
	{
		case SIZE_BLOCK:
			t_paquete * paquete = crear_paquete (SIZE_BLOCK);
			agregar_a_paquete (paquete,  (void *) TAMANO_DE_BLOQUE, strlen (TAMANO_DE_BLOQUE) + 1/*por el '\0'*/);
			enviar_paquete (paquete, socket);
			destruir_paquete (paquete);
			break;
			
		case DESCONEXION:
				//log_error(logger, "el cliente se desconecto. Terminando servidor");
				//free(cliente);
				//free(parametros);
			return NULL;
		default:
				//log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
	}
	
	return NULL;
}	



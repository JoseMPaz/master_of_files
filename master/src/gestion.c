#include "gestion.h"



void * gestionar_query_worker (void * argumento)
{
	int socket = *(int*)argumento;
	free (argumento);
	   
	int operacion = recibir_operacion (socket);
		
	switch (operacion) 
	{
		case NEW_QUERY:
			t_list * lista = recibir_carga_util (socket);//Recibe los argumentos que envio el query control: path y prioridad
			if (list_is_empty(workers))//No hay workers conectados
			{
				/*Responde al query que no puede realizar su consulta ya que no dispone de ningun worker*/
				t_paquete * paquete = crear_paquete (END_QUERY);
				agregar_a_paquete (paquete,  (void *) NO_HAY_WORKER_CONECTADOS, strlen (NO_HAY_WORKER_CONECTADOS) + 1/*por el '\0'*/);
				enviar_paquete (paquete, socket);
				destruir_paquete (paquete);
				/*Cierra el socket hacia la query*/
				close(socket);
				return NULL;
			}
			list_destroy_and_destroy_elements (lista, free);
			/*t_list * lista = recibir_carga_util (socket);
			
			
			printf ("El primer argumento que llego es: %s\n", (char *) list_get(lista, 0));
			printf ("El segundo argumento que llego es: %s\n", (char *) list_get(lista, 1));
			
			list_destroy_and_destroy_elements (lista, free);
			
			t_paquete * paquete = crear_paquete (END_QUERY);
			
			enviar_paquete (paquete, socket);
			
			destruir_paquete (paquete);
			close(socket);

			//transmitir list_get(lista, 0) al worker*/

			break;
		case NEW_MASTER:
			//cuando se conecte un cliente worker, su socket e id agregarlos a una lista
			
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

void cerrar_servidor(int signum) 
{
    if (bitacora_del_sistema) 
    {
        log_destroy(bitacora_del_sistema);
    }
    if (configuracion) 
    {
        config_destroy(configuracion);
    }
    if (socket_escucha >= 0) 
    {
        close(socket_escucha);
    }
    exit(0);
}

#include "gestion.h"



void * gestionar_query_worker (void * argumento)
{
	int socket = *(int*)argumento;
	
	free (argumento);
	   
	int operacion = recibir_operacion (socket);
		
	switch (operacion) 
	{
		case NEW_QUERY:
			t_list * ruta_prioridad = recibir_carga_util (socket);//Recibe los argumentos que envio el query control: path y prioridad 
			/*No hay workers conectados*/
			if (list_is_empty(workers))
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
			/*Si hay workers conectados*/
			t_worker * worker = (t_worker *) list_get(workers, 0);
			//printf ("Datos de worker conectado: id: %s ")
			
			t_paquete * paquete = crear_paquete (RECIBIR_QUERY);
			agregar_a_paquete (paquete,  (void *) list_get(ruta_prioridad, 0), strlen (list_get(ruta_prioridad, 0)) + 1/*por el '\0'*/);
			enviar_paquete (paquete, worker->socket);
			
			
			
			list_destroy_and_destroy_elements (ruta_prioridad, free);
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
		case NEW_WORKER:
			/*Cuando se conecta un worker, se lee su identificador, y se lo agrega junto a su socket y su estado a la lista de workers*/
			t_list * lista = recibir_carga_util (socket);//Recibe el argumento que envio el worker: id
			
			t_worker * new_worker = (t_worker *) malloc (sizeof(t_worker));
			strcpy (new_worker->id, list_get(lista, POSICION_ID));
			new_worker->socket = socket;
			new_worker->esta_libre = true;
			
			list_add (workers, (void *) new_worker);
			
			list_destroy_and_destroy_elements (lista, free);
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
	if (list_is_empty(workers))
		free (workers);
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

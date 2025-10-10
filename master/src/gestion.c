#include "gestion.h"

void * gestionar_query_worker (void * socket_de_atencion)
{
	int socket = *(int*)socket_de_atencion; 
	t_list * lista = NULL;
	
	free (socket_de_atencion);//Se libera ya que su valor fue copiado en socket
	   
	int operacion = recibir_operacion (socket);
	
	/* Operaciones que atiende Master
	* NEW_QUERY 
	* NEW_WORKER
	*/
	switch (operacion) 
	{
		case NEW_WORKER:
			/*Cuando se conecta un worker, se lee su identificador, y se lo agrega junto a su socket y su estado a la lista de workers*/
			lista = recibir_carga_util (socket); //Recibe el argumento que envio el worker: id
			
			t_worker * new_worker = (t_worker *) malloc (sizeof(t_worker));
			new_worker->id = strdup ( list_get(lista ,0) );
			new_worker->socket = socket;
			new_worker->esta_libre = true;
			
			pthread_mutex_lock(&mutex_workers);
			list_add (workers, (void *) new_worker);
			pthread_mutex_unlock(&mutex_workers);
						
			list_destroy_and_destroy_elements (lista, free);
			lista = NULL;
			break;
		case NEW_QUERY:
			lista = recibir_carga_util (socket); //Recibe la carga util enviado por el query: path_query y prioridad 
			
			pthread_mutex_lock(&mutex_workers);
			int cant_workers = list_size(workers);
			pthread_mutex_unlock(&mutex_workers);
			
			printf ("NEW_QUERY -> Cantidad de worker: %d\n", list_size(workers));
			
			if ( cant_workers == 0)/*No hay workers conectados*/
			{
				t_paquete * paquete = crear_paquete (END_QUERY);//Finaliza la consulta y le indica que no dispone de workers
				
				agregar_a_paquete (paquete,  (void *) NO_HAY_WORKER_CONECTADOS, strlen (NO_HAY_WORKER_CONECTADOS) + 1/*por el '\0'*/);
				
				enviar_paquete (paquete, socket);
				
				destruir_paquete (paquete);
							
				/*Cierra el socket hacia la query*/				
				close(socket);
			}
			else
			{
				t_paquete * paquete = crear_paquete (READ_QUERY);//Finaliza la consulta y le indica que no dispone de workers
				agregar_a_paquete (paquete,  (void *) "Lectura 1", strlen ("Lectura 1") + 1/*por el '\0'*/); 
				enviar_paquete (paquete, socket);
				destruir_paquete (paquete);
				paquete = crear_paquete (READ_QUERY);
				agregar_a_paquete (paquete,  (void *) "Lectura 2", strlen ("Lectura 1") + 1/*por el '\0'*/); 
				enviar_paquete (paquete, socket);
				destruir_paquete (paquete);
				paquete = crear_paquete (END_QUERY);
				agregar_a_paquete (paquete,  (void *) "Fin de Consulta", strlen ("Fin de Consulta") + 1/*por el '\0'*/);
				enviar_paquete (paquete, socket);
				destruir_paquete (paquete);
				close(socket);
			}
			list_destroy_and_destroy_elements (lista, free);
			lista = NULL;
			
				break;
		
		case DESCONEXION:
				//log_error(logger, "el cliente se desconecto. Terminando servidor");
				//free(cliente);
				//free(parametros);
			break;
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

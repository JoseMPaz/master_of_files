#include "gestion.h"

void * admitir_queries_workers (void * socket_de_atencion)
{
    int socket = *(int*)socket_de_atencion; //socker puede ser se query como de worker
    t_list * parametros_new_query = NULL;
    t_list * parametros_new_worker = NULL;
    int grado_de_multiprocesamiento;//Cantidad de Workes conectados a Master
    
    free (socket_de_atencion);//Se libera ya que su valor fue copiado en socket
       
    int operacion = recibir_operacion (socket);
    
    switch (operacion) 
    {
    	case NEW_QUERY: //Se encolar la query en queries_ready
    	
            parametros_new_query = recibir_carga_util (socket); //Recibe path_query y prioridad 
            			
            t_qcb * new_qcb = (t_qcb *) malloc (sizeof(t_qcb));// Se crea un nuevo query control block
                
            /*Se inicializa el nuevo qcb*/
            new_qcb->id = generar_id ();   
            new_qcb->socket = socket;              
            new_qcb->program_counter = 0;
            new_qcb->archivo = strdup ( (char *) list_get (parametros_new_query, ARCHIVO_DE_CONSULTAS));
            new_qcb->prioridad = strdup ( (char *) list_get (parametros_new_query, PRIORIDAD_DE_CONSULTA));
            new_qcb->estado = READY;
            new_qcb->worker_asignado = NULL;
            
            /*Se agrega a la cola de queries_ready*/     
            pthread_mutex_lock(&mutex_queries);
            	list_add ( queries_ready , (void *) new_qcb );
            pthread_mutex_unlock(&mutex_queries);
                 
            /*Avisa al scheduler que hay un nuevo query listo*/
            sem_post(&sem_queries_ready); 

			/*Se obtiene el grado de multiprocesamiento del sistema distribuido*/
            pthread_mutex_lock(&mutex_workers);
            	grado_de_multiprocesamiento = list_size (workers);
			pthread_mutex_unlock(&mutex_workers);
				
            /* Log mínimo y obligatorio 1: Conexión de Query Control */
             log_trace ( bitacora_del_sistema, "## Se conecta un Query Control para ejecutar la Query %s con prioridad %s multiprocesamiento %d",
            			 new_qcb->archivo , new_qcb->prioridad , grado_de_multiprocesamiento );

			/*Se destruye la lista junto a sus nodos*/
			list_destroy_and_destroy_elements (parametros_new_query, free);
            parametros_new_query = NULL;	
            
            /* Manejo de desconexiones de query*/            
			while (true) 
			{
    			int op = recibir_operacion(socket);
    			if (op == -1) 
    			{
        			manejar_desconexion_query(socket);
        			pthread_exit(NULL);
    			}

    			switch (op) 
    			{
        			case NEW_READ:
            			// El query puede mandar algo
            			break;
        			case DESCONEXION:
            			manejar_desconexion_query(socket);
            			pthread_exit(NULL);
            			break;
        			default:
            			// ignorar
            			;
    			}
			}
            
            break;
    	
        case NEW_WORKER: //Se encolar el worker en workers
        
            parametros_new_worker = recibir_carga_util (socket); //Recibe el identificador del worker
            
            t_worker * new_worker = malloc(sizeof(t_worker)); // Se crea un nuevo worker
            
            /*Se inicializa el nuevo worker*/
            new_worker->id = strdup ( (char *) list_get( parametros_new_worker , ID_WORKER ) );
            new_worker->socket = socket;
            new_worker->esta_libre = true;
            
            /*Se agrega a la cola de workers*/ 
            pthread_mutex_lock(&mutex_workers);
            	list_add(workers, new_worker);
            pthread_mutex_unlock(&mutex_workers);
            
            pthread_mutex_lock(&mutex_workers);
            	grado_de_multiprocesamiento = list_size (workers);
            pthread_mutex_unlock(&mutex_workers);
            
            /*Avisa al scheduler que hay un nuevo worker*/
            sem_post (&sem_workers_libres);//Avisa que hay un nuevo worker libre
            
            /* Log mínimo y obligatorio 2: Conexión de Worker */
    		log_trace (	bitacora_del_sistema, "## Se conecta el Worker %s - Cantidad total de Workers: %d", 
    					new_worker->id , grado_de_multiprocesamiento );
    		
            /*Se destruye la lista junto a sus nodos*/         
            list_destroy_and_destroy_elements(parametros_new_worker, free);
            parametros_new_worker = NULL;
            break;        

        case DESCONEXION:
            puts ("Se desconecto algo\n");
            break;

        default:
            // operación desconocida
            break;
    }
    
    return NULL;
}

void cerrar_servidor(int signum) 
{
    log_info(bitacora_del_sistema, "Recibida señal %d. Cerrando servidor...", signum);

    // Evitar que nuevos hilos manipulen las listas: opcional, depende de diseño.
    // Cerrar socket de escucha para que accept deje de bloquear
    if (socket_escucha >= 0) 
    {
        close(socket_escucha);
        socket_escucha = -1;
    }

    // Destruir elementos de workers (liberar id y struct)
    if (workers) 
    {
        pthread_mutex_lock(&mutex_workers);
        list_destroy_and_destroy_elements(workers, (void(*)(void*)) free_worker); 
        // Implementá free_worker que libere correctamente el t_worker y sus campos.
        workers = NULL;
        pthread_mutex_unlock(&mutex_workers);
    }

    // Si queries_ready tiene elementos, liberarlos también
    if (queries_ready) 
    {
        pthread_mutex_lock(&mutex_queries);
        // suponiendo que contains pointers a t_qcb
        list_destroy_and_destroy_elements(queries_ready, free);
        queries_ready = NULL;
        pthread_mutex_unlock(&mutex_queries);
    }

    if (bitacora_del_sistema) 
    {
        log_destroy(bitacora_del_sistema);
        bitacora_del_sistema = NULL;
    }
    if (configuracion) 
    {
        config_destroy(configuracion);
        configuracion = NULL;
    }

    exit(EXIT_SUCCESS);
}

int reservar_worker_libre_y_marcar(void)
{
    pthread_mutex_lock(&mutex_workers);
    int index = -1;
    for (int i = 0; i < list_size(workers); i++)
    {
        t_worker *w = list_get(workers, i);
        if (w->esta_libre)
        {
            w->esta_libre = false; // lo reservamos inmediatamente
            index = i;
            break;
        }
    }
    pthread_mutex_unlock(&mutex_workers);
    return index;
}


char *generar_id(void) 
{
    pthread_mutex_lock(&mutex_id_global);
    	int id_actual = id_global++;
    pthread_mutex_unlock(&mutex_id_global);

    return string_itoa (id_actual); // si estás usando la commons
}

void free_worker(void *elem)
{
    if (!elem) return;
    t_worker *w = (t_worker*) elem;
    if (w->id) free(w->id);
    // cerrar socket del worker si corresponde: close(w->socket);
    free(w);
}

void manejar_desconexion_query (int socket_query)
{
    pthread_mutex_lock(&mutex_queries);
    int grado_de_multiprocesamiento;//Cantidad de Workes conectados a Master

    t_qcb * q = NULL;

    // Buscar en queries_ready
    for (int i = 0; i < list_size(queries_ready); i++) 
    {
        t_qcb * tmp = list_get (queries_ready, i);
        
        if (tmp->socket == socket_query) 
        {
            q = list_remove (queries_ready, i);
            break;
        }
    }

    // Si no está en ready, buscar en execution
    if (q == NULL) 
    {
        for (int i = 0; i < list_size(queries_execution); i++) 
        {
            t_qcb *tmp = list_get(queries_execution, i);
            
            if (tmp->socket == socket_query) 
            {
                q = list_remove(queries_execution, i);
                break;
            }
        }
    }

    pthread_mutex_unlock(&mutex_queries);

    if (q) 
    {
        if (q->worker_asignado) 
        {
            pthread_mutex_lock(&mutex_workers);
            	q->worker_asignado->esta_libre = true;
            pthread_mutex_unlock(&mutex_workers);
            sem_post(&sem_workers_libres);
        }

		pthread_mutex_lock(&mutex_workers);
            grado_de_multiprocesamiento = list_size (workers);
		pthread_mutex_unlock(&mutex_workers);
		
		/* Log mínimo y obligatorio 3: Conexión de Query Control */
		log_trace ( bitacora_del_sistema, "## Se desconecta un Query Control. Se finaliza la Query %s con prioridad %s. Nivel multiprocesamiento %d", 
					q->id, q->prioridad, grado_de_multiprocesamiento);
		
        free(q->id);
        free(q->archivo);
        free(q->prioridad);
        free(q);
    }

    close(socket_query);
}


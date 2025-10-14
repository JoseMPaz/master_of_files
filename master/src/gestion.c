#include "gestion.h"

void * gestionar_query_worker (void * socket_de_atencion)
{
    int socket = *(int*)socket_de_atencion; 
    t_list * lista = NULL;
    int cant_workers;
    
    free (socket_de_atencion);//Se libera ya que su valor fue copiado en socket
       
    int operacion = recibir_operacion (socket);
    
    switch (operacion) 
    {
    	case NEW_QUERY:
            lista = recibir_carga_util (socket); //Recibe path_query y prioridad 
            
            if (!lista || list_is_empty(lista)) 
            {
                // manejar error
                if (lista) 
                	list_destroy_and_destroy_elements(lista, free);
                close(socket);
                return NULL;
            }

            pthread_mutex_lock(&mutex_workers);
            	cant_workers = list_size(workers);
            pthread_mutex_unlock(&mutex_workers);
            
            if (cant_workers == 0) 
            {
                t_paquete * paquete = crear_paquete (END_QUERY);
                agregar_a_paquete(paquete, (void *) NO_HAY_WORKER_CONECTADOS, strlen(NO_HAY_WORKER_CONECTADOS) + 1);
                enviar_paquete(paquete, socket);
                destruir_paquete(paquete);
                close(socket);
            }
            else
            {              
                // Encolar la query en queries_ready
                t_qcb * new_qcb = (t_qcb *) malloc (sizeof(t_qcb));
                
                new_qcb->id = generar_id (); 
                
                new_qcb->program_counter = 0;
                new_qcb->socket = socket;
                new_qcb->prioridad = strdup (list_get(lista, 0));
                new_qcb->estado = READY;
                new_qcb->worker_asignado = NULL;
                 
                pthread_mutex_lock(&mutex_queries);
                	list_add (queries_ready, new_qcb);
                pthread_mutex_unlock(&mutex_queries);
                 
                sem_post(&sem_queries_ready);

                pthread_mutex_lock(&mutex_workers);
            		cant_workers = list_size(workers);
				pthread_mutex_unlock(&mutex_workers);
				
                /* Log mínimo y obligatorio 1 */
                log_trace (bitacora_del_sistema, "## Se conecta un Query Control para ejecutar la Query %s con prioridad %s multiprocesamiento %d",
                (char *) list_get (lista, 0) , (char *) list_get (lista, 1) , cant_workers);
                 
			}
			
            list_destroy_and_destroy_elements (lista, free);
            lista = NULL;
            break;
    	
        case NEW_WORKER:
            lista = recibir_carga_util (socket); //Recibe el argumento que envio el worker: id
            
            if (!lista || list_is_empty(lista)) 
            {
                // manejar error
                if (lista) 
                	list_destroy_and_destroy_elements(lista, free);
                close(socket);
                return NULL;
            }

            t_worker * new_worker = malloc(sizeof(t_worker));
            
            new_worker->id = strdup(list_get(lista, 0));
            new_worker->socket = socket;
            new_worker->esta_libre = true;
            
            pthread_mutex_lock(&mutex_workers);
            	list_add(workers, new_worker);
            pthread_mutex_unlock(&mutex_workers);
            
            sem_post(&sem_workers_libres);
            
            pthread_mutex_lock(&mutex_workers);
            	cant_workers = list_size(workers);
            pthread_mutex_unlock(&mutex_workers);
            sem_post (&sem_workers_libres);
            /* Log mínimo y obligatorio 2 */
    		log_trace (bitacora_del_sistema, "## Se conecta el Worker %s - Cantidad total de Workers: %d", (char *) list_get(lista, 0), cant_workers);
    		
                        
            list_destroy_and_destroy_elements(lista, free);
            lista = NULL;
            break;

        

        case DESCONEXION:
            // manejar desconexión
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


char * generar_id (void)
{
    pthread_mutex_lock(&mutex_id_global);
    int id_actual = id_global++;
    pthread_mutex_unlock(&mutex_id_global);

    // Convertir el número a string dinámico
    // snprintf(NULL, 0, ...) devuelve el tamaño necesario
    int longitud = snprintf(NULL, 0, "%d", id_actual);
    char * id_str = (char *) malloc  ( sizeof(char *) * (longitud + 1)); // +1 para '\0'

    if (id_str == NULL) {
        perror("Error en malloc");
        return NULL;
    }

    snprintf(id_str, longitud + 1, "%d", id_actual);
    return id_str;
}

void free_worker(void *elem)
{
    if (!elem) return;
    t_worker *w = (t_worker*) elem;
    if (w->id) free(w->id);
    // cerrar socket del worker si corresponde: close(w->socket);
    free(w);
}



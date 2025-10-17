#include <utils/hello.h>
#include "gestion.h"

/*Variables Externas*/
t_config * configuracion = NULL;
t_log * bitacora_del_sistema = NULL;
t_list * workers = NULL;   
t_list * queries_ready = NULL;
t_list * queries_execution = NULL;
pthread_mutex_t mutex_workers = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_id_global = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_queries = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_listas = PTHREAD_MUTEX_INITIALIZER;
sem_t sem_queries_ready;
sem_t sem_queries_execution;
sem_t sem_workers_libres;
int socket_escucha;//Por medio de este socket se escuchan peticiones de todo aquel que sepa el ip y puerto del mater, en este caso las query_control y los worker
int id_global = 0;

/*Funciones para planificar las queries*/
void * fifo (void * arg);
void * prioridades (void * arg);
void enviar_query_al_worker(int socket_worker, t_qcb * query);
t_qcb* buscar_qcb_por_id(char *id);

/*Puntero a funcion*/
void * (*scheduler) (void * arg);

void * ejecutar_queries (void *arg);

int main(int argc, char* argv[]) 
{
	/*socket_temporal se usa porque ante un control+c se evita pedir memoria que no se puede liberar porque el programa murio*/
    int socket_temporal;
    pthread_t hilo_planificador, hilo_ejecutor;
    
    saludar("master");
        
    if (argc != 2)
	{
		fprintf (stderr, "Debe ingresar ejecutable y ruta_archivo_configuracion");
		return EXIT_FAILURE;
	}
    
    workers = list_create();
    queries_ready = list_create ();
    queries_execution = list_create ();
    signal (SIGINT, cerrar_servidor);
    
    sem_init(&sem_queries_ready, 0, 0);
    sem_init(&sem_queries_execution, 0, 0);
    sem_init(&sem_workers_libres, 0, 0);
    
    /*Abre el archivo de configuracion que esta en la misma carpeta que el archivo makefile*/
    configuracion = config_create ("master.config");
    
    /*Abre el archivo que contiene el registro de eventos con el nivel que indica el archivo de configuracion*/
    bitacora_del_sistema = log_create ("master.log", "MASTER", true, (t_log_level) config_get_int_value (configuracion, "LOG_LEVEL"));
    
    /*Genero el socket que escucha peticiones tanto de queries para solicitar atencion como de workers para ejecutar*/
    socket_escucha = crear_socket (SERVIDOR, NULL, config_get_string_value (configuracion, "PUERTO_ESCUCHA"));
       
      
	/*Indico que algoritmo de planficacion de va a usar*/
	if (strcmp (config_get_string_value (configuracion, "ALGORITMO_PLANIFICACION"), "FIFO") == 0)
		scheduler = fifo;
	if (strcmp (config_get_string_value (configuracion, "ALGORITMO_PLANIFICACION"), "PRIORIDADES") == 0)
		scheduler = prioridades;
	
	/*Delego la responsabilidad al hilo de planficacion*/
	pthread_create (&hilo_planificador, NULL, scheduler, NULL);
	pthread_detach (hilo_planificador);
	
	
	pthread_create (&hilo_ejecutor, NULL, ejecutar_queries, NULL);
	pthread_detach (hilo_ejecutor);
	
    while(true)
    {
    	pthread_t hilo_de_atencion; //Por cada solicitud de atencion genera un hilo nuevo  	
    	
    	socket_temporal = accept (socket_escucha, NULL, NULL);//Se queda aca esperando peticiones -> accept es bloqueante
    	
    	if (socket_temporal >= 0)
    	{
    		int * socket_de_atencion = (int *) malloc (sizeof(int)); //La funcion dentro del hilo debe liberar esta peticion de memoria
    		
    		*socket_de_atencion = socket_temporal;
    		
    		pthread_create (&hilo_de_atencion, NULL, admitir_queries_workers, (void *) socket_de_atencion);
    		pthread_detach(hilo_de_atencion);
    	}   	
    }
    pthread_exit (NULL);//Para esperar que terminen de procesar los hilos
    
    /*Liberacion de recursos*/
    if (socket_escucha > 0)
    	close (socket_escucha);
    config_destroy (configuracion);
    log_destroy (bitacora_del_sistema);	
    return 0;
}

void * fifo(void *arg)
{
    while (true) 
    {
        sem_wait(&sem_queries_ready);
        sem_wait(&sem_workers_libres);

        pthread_mutex_lock(&mutex_listas);

        if (list_is_empty(queries_ready)) 
        {
            pthread_mutex_unlock(&mutex_listas);
            continue;
        }
		
        t_qcb * query = NULL;
        query = list_remove(queries_ready, 0);

        t_worker * worker = NULL;
        for (int i = 0; i < list_size(workers); i++) 
        {
            t_worker *w = (t_worker *) list_get(workers, i);
            
            if (w->esta_libre) 
            {
                worker = w;
                w->esta_libre = false;
                break;
            }
        }

        if (worker !=NULL && query != NULL) 
        {
            query->worker_asignado = worker;
            query->estado = EXECUTING;
            list_add(queries_execution, query);

            sem_post(&sem_queries_execution);  // 🔸 avisa al hilo ejecutor
        }

        pthread_mutex_unlock(&mutex_listas);
    }
    return NULL;
}

void * prioridades(void *arg) 
{
    while (true) 
    {
        // Esperar que haya queries en READY y al menos un worker libre
        sem_wait(&sem_queries_ready);
        sem_wait(&sem_workers_libres);

        pthread_mutex_lock(&mutex_listas);

        if (list_is_empty(queries_ready)) 
        {
            pthread_mutex_unlock(&mutex_listas);
            continue;
        }

        // Buscar la query con MENOR valor numérico de prioridad
        int idx = 0;
        int prioridad_min = INT_MAX;

        for (int i = 0; i < list_size(queries_ready); i++) 
        {
            t_qcb *q = list_get(queries_ready, i);
            int prio = atoi(q->prioridad);
            if (prio < prioridad_min) 
            {
                prioridad_min = prio;
                idx = i;
            }
        }

        // Remover la query seleccionada
        t_qcb *query = list_remove(queries_ready, idx);

        // Buscar un worker libre
        t_worker *worker = NULL;
        for (int i = 0; i < list_size(workers); i++) 
        {
            t_worker *w = (t_worker *)list_get(workers, i);
            if (w->esta_libre) 
            {
                worker = w;
                w->esta_libre = false;
                break;
            }
        }

        // Asignar y mover a ejecución
        if (worker && query) 
        {
            query->worker_asignado = worker;
            query->estado = EXECUTING;
            list_add(queries_execution, query);

            // Despertar al hilo ejecutor
            sem_post(&sem_queries_execution);
        }

        pthread_mutex_unlock(&mutex_listas);
    }

    return NULL;
}

void enviar_query_al_worker(int socket_worker, t_qcb * query) 
{
    t_paquete *paquete = crear_paquete(EXEC_QUERY);
    agregar_a_paquete(paquete, query->archivo, strlen(query->archivo) + 1);
    agregar_a_paquete(paquete, query->id, strlen(query->id) + 1);

    enviar_paquete(paquete, socket_worker);
    destruir_paquete(paquete);

	/* Log mínimo y obligatorio 5: Envío de Query a Worker */
    log_trace(bitacora_del_sistema,
              "## Se envía la Query %s (%s) al Worker %s",
              query->id, query->prioridad, query->worker_asignado->id);
}

t_qcb* buscar_qcb_por_id(char *id) {
    for (int i = 0; i < list_size(queries_execution); i++) {
        t_qcb *qcb = list_get(queries_execution, i);
        if (strcmp(qcb->id, id) == 0) {
            log_trace(bitacora_del_sistema, "buscar_qcb_por_id: encontrado Query %s", id);
            return qcb;
        }
    }
    log_warning(bitacora_del_sistema, "buscar_qcb_por_id: NO encontrado Query %s", id);
    return NULL;
}

void * ejecutar_queries (void *arg)
{
    while (true)
    {
        sem_wait(&sem_queries_execution); // Bloqueado hasta que haya query en ejecución

        pthread_mutex_lock(&mutex_listas);

        if (list_is_empty(queries_execution))
        {
            pthread_mutex_unlock(&mutex_listas);
            continue;
        }

        t_qcb * query = list_remove(queries_execution, 0);
        t_worker * worker = query->worker_asignado;

        pthread_mutex_unlock(&mutex_listas);

        if (worker != NULL && query != NULL)
        {
            puts("Se envia query a worker\n");
            enviar_query_al_worker(worker->socket, query);
            puts("Ya se envio\n");

            int operacion;
            t_list * lista = NULL;

            do 
            {
                operacion = recibir_operacion(worker->socket);

                switch (operacion)
                {
                    case NEW_READ:
                    {
                        lista = recibir_carga_util(worker->socket);
                        char *instruccion = list_get(lista, 0);
                        printf("Worker %d envió: %s\n", worker->socket, instruccion);

                        // 🔸 Reenviar al Query Control como READ_QUERY
                        t_paquete *paq = crear_paquete(READ_QUERY);
                        agregar_a_paquete(paq, instruccion, strlen(instruccion) + 1);
                        enviar_paquete(paq, query->socket);
                        destruir_paquete(paq);

                        list_destroy_and_destroy_elements(lista, free);
                        break;
                    }

                    case END_QUERY:
                    {
                        printf("Fin de query %s\n", query->id);

                        // 🔸 Notificar al Query Control fin de query
                        char mensaje[128];
                        snprintf(mensaje, sizeof(mensaje),
                                 "Finalización de query atendido por worker %s", worker->id);

                        t_paquete *paq = crear_paquete(END_QUERY);
                        agregar_a_paquete(paq, mensaje, strlen(mensaje) + 1);
                        enviar_paquete(paq, query->socket);
                        destruir_paquete(paq);

                        pthread_mutex_lock(&mutex_listas);
                        worker->esta_libre = true;
                        pthread_mutex_unlock(&mutex_listas);
                        sem_post(&sem_workers_libres);
                        break;
                    }

                    case DESCONEXION:
                        printf("Worker desconectado.\n");
                        break;

                    default:
                        log_warning(bitacora_del_sistema,
                                    "Operación desconocida o nula del Worker %s: %d",
                                    worker->id, operacion);
                        break;
                }

            } while (operacion != END_QUERY);
        }
    }
    return NULL;
}


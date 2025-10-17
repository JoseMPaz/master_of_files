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
sem_t sem_workers_libres;
int socket_escucha;//Por medio de este socket se escuchan peticiones de todo aquel que sepa el ip y puerto del mater, en este caso las query_control y los worker
int id_global = 0;

/*Funciones para planificar las queries*/
void * fifo (void * arg);
void * prioridades (void * arg);
/*Puntero a funcion*/
void * (*scheduler) (void * arg);

int main(int argc, char* argv[]) 
{
	/*socket_temporal se usa porque ante un control+c se evita pedir memoria que no se puede liberar porque el programa murio*/
    int socket_temporal;
    pthread_t hilo_planificador;
    
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

void * fifo (void * arg)
{
	while (true) 
	{
        sem_wait (&sem_queries_ready); // Esperar que haya al menos un query en ready 
        sem_wait (&sem_workers_libres); // Esperar que haya al menos un worker libre

		pthread_mutex_lock(&mutex_listas);

		/*Por seguridad ante condiciones de carrera, se vuelve a verificar que realmente haya un query_ready antes de intentar removerlo.*/
        if (list_is_empty(queries_ready)) 
        {
            pthread_mutex_unlock(&mutex_listas);
            continue;
        }
		
        // Tomar el primer query READY por ser FIFO
        t_qcb * query = list_remove(queries_ready, 0);

        // Buscar worker libre
        t_worker * worker = NULL;
        
        for (int i = 0; i < list_size (workers); i++) 
        {
            t_worker * w = (t_worker *) list_get (workers, i);
            if (w->esta_libre) 
            {
                worker = w;
                w->esta_libre = false;//Lo marco como ocupado
                break;//Dejo de buscar
            }
        }

        if (worker && query) 
        {
            query->worker_asignado = worker;
            query->estado = EXECUTING;
            list_add (queries_execution, query);
            //printf("[PLANIFICADOR] Asignado query %s → worker %s\n", query->id, worker->id);
        }

        pthread_mutex_unlock(&mutex_listas);

        if (worker && query) 
        {
        	
            //enviar_query_al_worker(worker->socket, query);
            
            
            
        }

        //usleep(50000); // 50 ms para evitar loop muy agresivo
    }
    return NULL;
}

void * prioridades (void * arg)
{

	return NULL;
}


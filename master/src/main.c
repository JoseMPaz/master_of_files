#include <utils/hello.h>
#include "gestion.h"

/*Variables Externas*/
t_config * configuracion = NULL;
t_log * bitacora_del_sistema = NULL;
t_list * workers = NULL;   // definición real
pthread_mutex_t mutex_workers = PTHREAD_MUTEX_INITIALIZER;
int socket_escucha;//Por medio de este socket se escuchan peticiones de todo aquel que sepa el ip y puerto del mater, en este caso las query_control y los worker

int main(int argc, char* argv[]) 
{
	/*socket_temporal se usa porque ante un control+c se evita pedir memoria que no se puede liberar porque el programa murio*/
    int socket_temporal;
    
    saludar("master");
        
    if (argc != 2)
	{
		fprintf (stderr, "Debe ingresar ejecutable y ruta_archivo_configuracion");
		return EXIT_FAILURE;
	}
    
    workers = list_create();
    
    signal (SIGINT, cerrar_servidor);
    
    /*Abre el archivo de configuracion que esta en la misma carpeta que el archivo makefile*/
    configuracion = config_create ("master.config");
    
    /*Abre el archivo que contiene el registro de eventos con el nivel que indica el archivo de configuracion*/
    bitacora_del_sistema = log_create ("master.log", "MASTER", false, (t_log_level) config_get_int_value (configuracion, "LOG_LEVEL"));
    
    /*Genero el socket que escucha peticiones tanto de queries para solicitar atencion como de workers para ejecutar*/
    socket_escucha = crear_socket (SERVIDOR, NULL, config_get_string_value (configuracion, "PUERTO_ESCUCHA"));
       
    while(true)
    {
    	pthread_t hilo_de_atencion; //Por cada solicitud de atencion genera un hilo nuevo  	
    	
    	socket_temporal = accept (socket_escucha, NULL, NULL);//Se queda aca esperando peticiones -> accept es bloqueante
    	
    	if (socket_temporal >= 0)
    	{
    		int * socket_de_atencion = (int *) malloc (sizeof(int)); //La funcion dentro del hilo debe liberar esta peticion de memoria
    		
    		*socket_de_atencion = socket_temporal;
    		
    		pthread_create (&hilo_de_atencion, NULL, gestionar_query_worker, (void *) socket_de_atencion);
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




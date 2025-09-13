#include <utils/hello.h>

/*Variables Externas*/
t_config * configuracion = NULL;
t_log * bitacora_del_sistema = NULL;
int socket_escucha;//Por medio de este socket se escuchan peticiones de todo aquel que sepa el ip y puerto de storage, en este caso los worker

void cerrar_servidor(int signum);

int main(int argc, char* argv[]) 
{
    saludar("storage");
    signal(SIGINT, cerrar_servidor);
    int socket_temporal;
    /*Abre el archivo de configuracion que esta en la misma carpeta que el archivo makefile*/
    configuracion = config_create ("storage.config");
     /*Abre el archivo que contiene el registro de eventos con el nivel que indica el archivo de configuracion*/
	bitacora_del_sistema = log_create ("registro_de_eventos.log", "STORAGE", true, (t_log_level) config_get_int_value (configuracion, "LOG_LEVEL"));
	
	socket_escucha = crear_socket (SERVIDOR, NULL, config_get_string_value (configuracion, "PUERTO_ESCUCHA"));
	printf ("socket_escucha: %d\n", socket_escucha);
	
	while(true)
    {
    	pthread_t hilo_de_atencion;
    	/*socket_temporal se usa porue ante un control+c se evita pedir memoria que no se puede liberar porque el programa murio*/
    	socket_temporal = accept (socket_escucha, NULL, NULL);//Se queda aca esperando peticiones porque accept es bloqueante
    	if (socket_temporal >= 0)
    	{
    		int * socket_de_atencion = (int *) malloc (sizeof(int));//La funcion dentro del hilo debe liberar esta peticion de memoria
    		*socket_de_atencion = socket_temporal;
    		printf ("Nuevo cliente conectado\n");
    		pthread_create (&hilo_de_atencion, NULL, atender_cliente, (void *) socket_de_atencion);
    		pthread_detach(hilo_de_atencion);
    	} 
    
    }
	
	
		
	if (socket_escucha > 0)
    	close (socket_escucha);
	config_destroy (configuracion);
    log_destroy (bitacora_del_sistema);	
    return 0;
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

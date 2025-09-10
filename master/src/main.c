#include <utils/hello.h>

/*Variables Externas*/
t_config * configuracion = NULL;
t_log * bitacora_del_sistema = NULL;
t_list * workers = NULL;
int socket_escucha;//Por medio de este socket se escuche peticiones de todo aquel que sepa el ip y puerto, en este caso las query_control y los worker

void cerrar_servidor(int signum);



int main(int argc, char* argv[]) 
{
    saludar("master");//Sera removido
    
    signal(SIGINT, cerrar_servidor);
    /*Abre el archivo de configuracion que esta en la misma carpeta que el archivo makefile*/
    configuracion = config_create ("master.config");
    /*Abre el archivo que contiene el registro de eventos con el nivel que indica el archivo de configuracion*/
    bitacora_del_sistema = log_create ("master.log", "MASTER", false, (t_log_level) config_get_int_value (configuracion, "LOG_LEVEL"));
    /*Master no requiere que se ingresen argumentos por el CLA*/
    socket_escucha = crear_socket (SERVIDOR, NULL, config_get_string_value (configuracion, "PUERTO_ESCUCHA"));
    printf ("socket_escucha: %d\n", socket_escucha);
    
    while(true)
    {
    	pthread_t hilo_de_atencion;
    	int * socket_de_atencion = (int *) malloc (sizeof(int));//La funcion dentro del hilo debe liberar esta peticion de memoria
    	*socket_de_atencion = accept (socket_escucha, NULL, NULL);//Se queda aca esperando peticiones porque accept es bloqueante
    	//Registra en el archivo de eventos que hubo una nueva conexion
    	printf ("Nuevo cliente conectado\n");
    	pthread_create (&hilo_de_atencion, NULL, atender_cliente, (void *) socket_de_atencion);
    	pthread_detach(hilo_de_atencion);
    }
    pthread_exit (NULL);//Para esperar que terminen de procesar los hilos
    
    /*Liberacion de recursos*/
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

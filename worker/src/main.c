#include <utils/hello.h>

#define ARCHIVO_DE_CONFIGURACION 1
#define ID_WORKER 2
#define CANTIDAD_ARGUMENTOS 2

/*Variables Externas*/
t_config * configuracion = NULL;
t_log * bitacora_del_sistema = NULL;

int main(int argc, char* argv[]) 
{
	int socket_worker_a_storage;
	int socket_worker_a_master;
	t_paquete * paquete;
	int operacion;
	
    saludar("worker");
    	
	if (argc != 1 + CANTIDAD_ARGUMENTOS)//Valida que se ingrese el ejecutable + 3 argumentos por el CLA
	{
		fprintf (stderr, "Error: El ejecutable requiere 2 argumentos por linea de comandos\n");
		return EXIT_FAILURE;
	}
	
	/*Abre el archivo de configuracion pasado por el CLA*/
    if ((configuracion = config_create(argv[ARCHIVO_DE_CONFIGURACION])) == NULL)
    {
    	fprintf(stderr, "%s\n", "Error: No se pudo abrir el archivo de configuración");
    	return EXIT_FAILURE;
    }
	/*Abre el archivo que contiene el registro de eventos con el nivel que indica el archivo de configuracion*/
    bitacora_del_sistema = log_create ("query_control.log", "WORKER", false, (t_log_level) config_get_int_value (configuracion, "LOG_LEVEL"));
    /*Crea un socker para conectarse al storage*/
	socket_worker_a_storage = crear_socket (CLIENTE	, config_get_string_value(configuracion, "IP_STORAGE")
													, config_get_string_value(configuracion, "PUERTO_STORAGE"));
	/*Crea un socker para conectarse al master*/
	socket_worker_a_master = crear_socket (CLIENTE	, config_get_string_value (configuracion, "IP_MASTER")
													, config_get_string_value (configuracion, "PUERTO_MASTER"));
	/*Se establece la conexion con storage*/
	solicitar_atencion (socket_worker_a_storage, config_get_string_value (configuracion, "IP_STORAGE"), config_get_string_value (configuracion, "PUERTO_STORAGE"));
	/*Se establece la conexion con master*/
	solicitar_atencion (socket_worker_a_master, config_get_string_value (configuracion, "IP_MASTER"), config_get_string_value (configuracion, "PUERTO_MASTER"));
	
	paquete = crear_paquete (NEW_WORKER);
	agregar_a_paquete (paquete,  (void *) argv[ID_WORKER], strlen (argv[ID_WORKER]) + 1/*por el '\0'*/);
	
	/*Envia el saludo al master para que lo agrege a una lista mediante su ID_WORKER y cuando requiera master le haga consultas a worker*/
	enviar_paquete (paquete, socket_worker_a_master);
	
	while (true)
	{
		operacion = recibir_operacion(socket_worker_a_master); 
		switch (operacion) 
		{
			case RECIBIR_QUERY:
			/*Aca hay que leer argumento de la operacion*/
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
	
	
	}
	
	
	
	
	
	
	
	destruir_paquete (paquete);
	if (socket_worker_a_storage >= 0)
		close (socket_worker_a_storage);
		
	if (socket_worker_a_master >= 0)
		close (socket_worker_a_master);
	
	config_destroy (configuracion);
    log_destroy (bitacora_del_sistema);
    return 0;
}

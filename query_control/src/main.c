#include <utils/hello.h>

#define ARCHIVO_DE_CONFIGURACION 1
#define ARCHIVO_DE_CONSULTAS 2
#define PRIORIDAD_DE_CONSULTA 3
#define CANTIDAD_ARGUMENTOS 3

/*Variables Externas*/
t_config * configuracion = NULL;
t_log * bitacora_del_sistema = NULL;

/*CLA: Argumentos por Linea de Comandos*/

int main(int argc, char* argv[]) 
{
	int socket_query_control;//Por medio de este socket se establece conexion con el master
	t_paquete * paquete;
	int operacion;

	if (argc != 1 + CANTIDAD_ARGUMENTOS)//Valida que se ingrese el ejecutable + 3 argumentos por el CLA
	{
		fprintf (stderr, "Error: El ejecutable requiere 3 argumentos por linea de comandos");
		return EXIT_FAILURE;
	}
	/*Abre el archivo de configuracion pasado por el CLA*/
    configuracion = config_create (argv[ARCHIVO_DE_CONFIGURACION]);
    /*Abre el archivo que contiene el registro de eventos con el nivel que indica el archivo de configuracion*/
    bitacora_del_sistema = log_create ("registro_de_eventos.log", "QUERY_CONTROL", false, (t_log_level) config_get_int_value (configuracion, "LOG_LEVEL"));
    /*Crea el socket como cliente hacia el servidor master*/
    socket_query_control = crear_socket (CLIENTE, config_get_string_value (configuracion, "IP_MASTER"), config_get_string_value (configuracion, "PUERTO_MASTER"));
    solicitar_atencion (socket_query_control, config_get_string_value (configuracion, "IP_MASTER"), config_get_string_value (configuracion, "PUERTO_MASTER"));
    log_trace ( bitacora_del_sistema, 
    			"## Conexión al Master exitosa. IP: %s, Puerto: %s", 
    			config_get_string_value (configuracion, "IP_MASTER"), config_get_string_value (configuracion, "PUERTO_MASTER"));
    printf ("Se establecio la conexion al servidor a traves del socket_master: %d\n", socket_query_control);
    paquete = crear_paquete (NEW_QUERY);
    agregar_a_paquete (paquete,  (void *) argv[ARCHIVO_DE_CONSULTAS], strlen (argv[ARCHIVO_DE_CONSULTAS]) + 1/*por el '\0'*/);
    agregar_a_paquete (paquete,  (void *) argv[PRIORIDAD_DE_CONSULTA], strlen (argv[PRIORIDAD_DE_CONSULTA]) + 1/*por el '\0'*/);
    
    enviar_paquete (paquete, socket_query_control);
    do
    {
    	operacion = recibir_operacion(socket_query_control); 
    	switch (operacion) 
		{
			case END_QUERY:
				printf ("Llego end query\n");
				
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
    }while (operacion != END_QUERY);
    
    
    
    
    destruir_paquete (paquete);
    config_destroy (configuracion);
    log_destroy (bitacora_del_sistema);

    return EXIT_SUCCESS;
}

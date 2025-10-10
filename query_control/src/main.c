#include <utils/hello.h>

#define ARCHIVO_DE_CONFIGURACION 1
#define ARCHIVO_DE_CONSULTAS 2
#define PRIORIDAD_DE_CONSULTA 3
#define CANTIDAD_ARGUMENTOS 3


/*Variables Externas*/
t_config * configuracion = NULL;
t_log * bitacora_del_sistema = NULL;

/*CLA: Argumentos por Linea de Comandos*/
//./bin/query_control configuracion archivo_query 15

int main(int argc, char* argv[]) 
{
	int socket_query_control;//Por medio de este socket se establece conexion con el master
	t_paquete * paquete;
	int operacion;
	t_list * lista = NULL;
	
	saludar("query");

	if (argc != 1 + CANTIDAD_ARGUMENTOS)//Valida que se ingrese el ejecutable + 3 argumentos por el CLA
	{
		fprintf (stderr, "Error: El ejecutable requiere 3 argumentos por linea de comandos\n");
		return EXIT_FAILURE;
	}
	
	/*Abre el archivo de configuracion pasado por el CLA*/
    if ((configuracion = config_create (argv[ARCHIVO_DE_CONFIGURACION])) == NULL)
    {
    	fprintf (stderr, "%s\n", "Error: No se pudo abrir el archivo de configuración");
    	return EXIT_FAILURE;
    }
    
    /*Abre el archivo que contiene el registro de eventos con el nivel que indica el archivo de configuracion*/
    bitacora_del_sistema = log_create ("query_control.log", "QUERY_CONTROL", true, (t_log_level) config_get_int_value (configuracion, "LOG_LEVEL"));
   
    /*Crea el socket como cliente hacia el servidor master*/
    socket_query_control = crear_socket (CLIENTE, config_get_string_value (configuracion, "IP_MASTER"), config_get_string_value (configuracion, "PUERTO_MASTER"));
    /*Se conecta a master*/
    solicitar_atencion (socket_query_control, config_get_string_value (configuracion, "IP_MASTER"), config_get_string_value (configuracion, "PUERTO_MASTER"));
    
    /* Log mínimo y obligatorio 1 */
    log_trace ( bitacora_del_sistema, "## Conexión al Master exitosa. IP: %s, Puerto: %s", config_get_string_value (configuracion, "IP_MASTER"),
    			config_get_string_value (configuracion, "PUERTO_MASTER") );
        
    // Protocolo: operacion | longitud_carga_util | longitud_1 | cadena_1 | longitud_2 | cadena_2 | ... | longitud_N | cadena_N
	
    // Se agrega la operacion para que master gestiene la consulta
    paquete = crear_paquete (NEW_QUERY); 
    // Se agrega el nombre de archivo de queries
    agregar_a_paquete (paquete,  (void *) argv[ARCHIVO_DE_CONSULTAS], strlen (argv[ARCHIVO_DE_CONSULTAS]) + 1/*por el '\0'*/); 
     // Se agrega la prioridad de la query
    agregar_a_paquete (paquete,  (void *) argv[PRIORIDAD_DE_CONSULTA], strlen (argv[PRIORIDAD_DE_CONSULTA]) + 1/*por el '\0'*/);
    
    /* Envia la ruta del archivo_query y la prioridad que estan empaquetados y se serializan al enviar a master*/
    enviar_paquete (paquete, socket_query_control);
    /* Log mínimo y obligatorio 2 */
    log_trace (bitacora_del_sistema, "## Solicitud de ejecución de Query: %s, prioridad:%s", argv[ARCHIVO_DE_CONSULTAS], argv[PRIORIDAD_DE_CONSULTA]);
 
    do
    {
    	/* Se queda esperando recibir alguna operacion - Bloqueante */
    	operacion = recibir_operacion(socket_query_control); 
    	switch (operacion) 
		{
			case END_QUERY:
				lista = recibir_carga_util (socket_query_control); //Recibe el motivo de fin de consulta
				/* Log mínimo y obligatorio 4 */
				log_trace (bitacora_del_sistema, "## Query Finalizada - %s", (char *) list_get (lista, 0));
				list_destroy_and_destroy_elements (lista, free);
				close(socket_query_control);				
				break;
			
			case READ_QUERY:
				lista = recibir_carga_util (socket_query_control); 
				log_trace (bitacora_del_sistema, "## Lectura realizada: File <File:Tag>, contenido: %s", (char *) list_get (lista, 0));
				list_destroy_and_destroy_elements (lista, free);
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

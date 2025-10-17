#include <utils/hello.h>

#define ARCHIVO_DE_CONFIGURACION 1
#define ARCHIVO_DE_CONSULTAS 2
#define PRIORIDAD_DE_CONSULTA 3
#define CANTIDAD_ARGUMENTOS 3
#define MOTIVO_FIN_DE_QUERY 0
#define LECTURA_DE_QUERY 0

/*Variables Externas*/
t_config * configuracion = NULL;
t_log * bitacora_del_sistema = NULL;
int socket_master;
char archivo_query[100];

// Protocolo de mensajes: operacion | longitud_carga_util | longitud_1 | cadena_1 | longitud_2 | cadena_2 | ... | longitud_N | cadena_N

void manejar_sigint(int signum);
void manejar_sigstp(int signum);

/*Descripcion:
Se conecta a Master y le solicita un NEW_QUERY con los parametros nombre_archivo_query y prioridad
Se queda en un bucle esperando recibir de Master una READ_QUERY o bien un END_QUERY, en ambos casos de loguea y si es un END_QUERY finaliza el bucle
*/

int main(int argc, char* argv[]) 
{
	int socket;//Por medio de este socket se establece conexion con el master
	t_paquete * paquete;
	int operacion;
	t_list * parametros_end_query = NULL;
	t_list * parametros_read_query = NULL;
	
	strcpy (archivo_query, argv[ARCHIVO_DE_CONSULTAS]);
	
	signal(SIGINT, manejar_sigint);   // atrapa Ctrl+C
	signal(SIGTSTP, manejar_sigstp);  // atrapa Ctrl+Z
	
	saludar("query");

	if (argc != 1 + CANTIDAD_ARGUMENTOS)//Valida que se ingrese el ejecutable + 3 argumentos por el CLA
	{
		fprintf (stderr, "Error: Debe ingresar ./bin/query <ruta_archivo_cofiguracio> <nombre_archivo_con_instrucciones> <prioridad>\n");
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
    socket = crear_socket (CLIENTE, config_get_string_value (configuracion, "IP_MASTER"), config_get_string_value (configuracion, "PUERTO_MASTER"));
    socket_master = socket;
    /*Se conecta a master*/
    solicitar_atencion (socket, config_get_string_value (configuracion, "IP_MASTER"), config_get_string_value (configuracion, "PUERTO_MASTER"));
    
    /* Log mínimo y obligatorio 1: Conexión al master*/
    log_trace ( bitacora_del_sistema, "## Conexión al Master exitosa. IP: %s, Puerto: %s", 
    			config_get_string_value (configuracion, "IP_MASTER"), config_get_string_value (configuracion, "PUERTO_MASTER") );
        
    /* Se agrega un paquete con la operacion NEW_QUERY para que el Master lo encole */
    paquete = crear_paquete (NEW_QUERY); 
    
    /* Se agrega el paquete con el nombre de archivo de queries y  su prioridad */
    agregar_a_paquete (paquete,  (void *) argv[ARCHIVO_DE_CONSULTAS], strlen (argv[ARCHIVO_DE_CONSULTAS]) + 1/*por el '\0'*/); 
    agregar_a_paquete (paquete,  (void *) argv[PRIORIDAD_DE_CONSULTA], strlen (argv[PRIORIDAD_DE_CONSULTA]) + 1/*por el '\0'*/);
    
    /* Envia envia el paquete serializado a Master*/
    enviar_paquete (paquete, socket);
    destruir_paquete (paquete);
    
    /* Log mínimo y obligatorio 2: Envío de Query */
    log_trace (	bitacora_del_sistema, "## Solicitud de ejecución de Query: %s, prioridad:%s", 
    			argv[ARCHIVO_DE_CONSULTAS], argv[PRIORIDAD_DE_CONSULTA]);
 
 	/*	Aca se espera que en master el socker se encuentre en una lista query_ready y cuando hay un worker disponible 
 		Master le pide a Worker que lea el archivo_de_query retransmitido, worker busca en sus archivos y empieza a retransmitir 
 		de a una instruccion a Master y Master retransmite a query con READ_QUERY*/
    do
    {
    	/* Se queda esperando recibir una operacion por parte de Master: END_QUERY, READ_QUERY */
    	operacion = recibir_operacion (socket); /* Bloqueante */
    	
    	switch (operacion) 
		{
			case READ_QUERY:
				parametros_read_query = recibir_carga_util (socket); 
				
				/* Log mínimo y obligatorio 3: Lectura de File */
				log_trace (	bitacora_del_sistema, "## Lectura realizada: File <File:Tag>, contenido: %s", 
							(char *) list_get (parametros_read_query, LECTURA_DE_QUERY));
				
				list_destroy_and_destroy_elements (parametros_read_query, free);
				parametros_read_query = NULL;
				
				
				break;
				
			case END_QUERY:
				parametros_end_query = recibir_carga_util (socket); // Recibe el motivo de fin de consulta
				
				/* Log mínimo y obligatorio 4: Finalización de la Query */
				log_trace (	bitacora_del_sistema, "## Query Finalizada - %s", 
							(char *) list_get (parametros_end_query, MOTIVO_FIN_DE_QUERY));
				
				list_destroy_and_destroy_elements (parametros_end_query, free);
				parametros_end_query = NULL;
				close (socket);				
				break;			
			
			case DESCONEXION:
				break;
			default:
				//log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
    }while (operacion != END_QUERY);   
    
    
    config_destroy (configuracion);
    log_destroy (bitacora_del_sistema);

    return EXIT_SUCCESS;
}

void manejar_sigstp(int signum) 
{
    // Evita que el proceso se suspenda
    signal(SIGTSTP, SIG_IGN); 

    log_trace (bitacora_del_sistema, 
        "## Query %s Finalizada - Interrupción del usuario (Ctrl+Z)", archivo_query);

    close(socket_master);
    config_destroy(configuracion);
    log_destroy(bitacora_del_sistema);
    exit(0);
}

void manejar_sigint(int signum)
{
    log_trace (bitacora_del_sistema, "## Query %s Finalizada - Interrupción del usuario (Ctrl+C)", archivo_query);
    close(socket_master);
    config_destroy(configuracion);
    log_destroy(bitacora_del_sistema);
    exit(0);
}

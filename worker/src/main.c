#include <utils/hello.h>

#define ARCHIVO_DE_CONFIGURACION 1
#define ID_WORKER 2
#define CANTIDAD_ARGUMENTOS 2 //Archivo de configuracion e Identificador del worker

/*Variables Externas*/
t_config * configuracion = NULL;
t_log * bitacora_del_sistema = NULL;

int main(int argc, char* argv[]) 
{
	//int socket_storage;
	int socket_master;
	t_paquete * paquete;
	int operacion;
	
    saludar("worker");
    
    //Valida que se ingrese el ejecutable + 2 argumentos por el CLA
	if (argc != 1 + CANTIDAD_ARGUMENTOS)
	{
		fprintf (stderr, "Error: El ejecutable requiere 2 argumentos por linea de comandos\n");
		return EXIT_FAILURE;
	}
	
	/*Abre el archivo de configuracion pasado por el CLA*/
    if ( (configuracion = config_create (argv[ARCHIVO_DE_CONFIGURACION]) ) == NULL)
    {
    	fprintf (stderr, "%s\n", "Error: No se pudo abrir el archivo de configuración");
    	return EXIT_FAILURE;
    }
    
	/*Abre el archivo que contiene el registro de eventos con el nivel que indica el archivo de configuracion*/
    bitacora_del_sistema = log_create ("query_control.log", "WORKER", true, (t_log_level) config_get_int_value (configuracion, "LOG_LEVEL"));
    
    /*Crea un socker para conectarse al storage*/
	//socket_worker_a_storage = crear_socket (CLIENTE	, config_get_string_value(configuracion, "IP_STORAGE")
	//												, config_get_string_value(configuracion, "PUERTO_STORAGE"));
													
	/*Crea un socker para conectarse al master*/
	socket_master = crear_socket ( CLIENTE	, config_get_string_value (configuracion, "IP_MASTER")
											, config_get_string_value (configuracion, "PUERTO_MASTER"));
													
	/*Se establece la conexion con storage*/
	//solicitar_atencion (socket_worker_a_storage, config_get_string_value (configuracion, "IP_STORAGE"), config_get_string_value (configuracion, "PUERTO_STORAGE"));
	/*Se establece la conexion con master*/
	solicitar_atencion ( socket_master, config_get_string_value (configuracion, "IP_MASTER"), config_get_string_value (configuracion, "PUERTO_MASTER"));
	
	/*Envio al master sus datos para que lo agende si necesita que realicen trabajo*/
	paquete = crear_paquete (NEW_WORKER);
	agregar_a_paquete (paquete,  (void *) argv[ID_WORKER], strlen (argv[ID_WORKER]) + 1/*por el '\0'*/);
	
	/*Envia el saludo al master para que lo agrege a una lista mediante su ID_WORKER y cuando requiera master le haga consultas a worker*/
	enviar_paquete (paquete, socket_master);
	
	while (true)
	{
		operacion = recibir_operacion(socket_master); //Bloqueante hasta que master le envie algo
		
		switch (operacion) 
		{
			case EXEC_QUERY:
				 t_list *lista = recibir_carga_util(socket_master);
    if (!lista || list_is_empty(lista) || list_size(lista) < 2) {
        log_error(bitacora_del_sistema, "Datos de query incompletos recibidos del Master");
        if (lista) list_destroy_and_destroy_elements(lista, free);
        break;
    }

    char * archivo_query = strdup((char *)list_get(lista, 0));
    char * id_query      = strdup((char *)list_get(lista, 1));
    list_destroy_and_destroy_elements(lista, free);

    // Construir ruta completa al archivo
    char ruta_completa[512];
    snprintf(ruta_completa, sizeof(ruta_completa), "%s%s", config_get_string_value (configuracion, "PATH_QUERIES") , archivo_query);

    FILE * f = fopen(ruta_completa, "r");
    if (!f) {
        log_error(bitacora_del_sistema, "No se pudo abrir el archivo de query: %s", ruta_completa);
        free(archivo_query);
        free(id_query);
        break;
    }

    char linea[512];
    while ( fgets(linea, sizeof(linea), f)) 
    {
        linea[strcspn(linea, "\r\n")] = 0; // quitar salto de línea

        // Enviar instrucción al Master
        t_paquete *paq = crear_paquete(NEW_READ);
        agregar_a_paquete(paq, linea, strlen(linea) + 1);
        enviar_paquete(paq, socket_master);
        destruir_paquete(paq);

        log_trace(bitacora_del_sistema, "Worker envía instrucción de Query %s al Master: %s", id_query, linea);

        if (strcmp(linea, "END") == 0) break;
		
    }

    fclose(f);
    free(archivo_query);
    

    // Avisar al Master que la query terminó
    t_paquete * paq_fin = crear_paquete(END_QUERY);
    enviar_paquete(paq_fin, socket_master);
    destruir_paquete(paq_fin);

    log_trace(bitacora_del_sistema, "Worker finalizó ejecución de la query %s", id_query);
    free(id_query);

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
	
	/*if (socket_worker_a_storage >= 0)
		close (socket_worker_a_storage);*/
		
	if (socket_master >= 0)
		close (socket_master);
	
	config_destroy (configuracion);
    log_destroy (bitacora_del_sistema);
    
    return 0;
}

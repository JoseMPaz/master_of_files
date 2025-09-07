#include <utils/hello.h>

void saludar(char* quien) 
{
    printf("Hola desde %s!!\n", quien);
}

int crear_socket (t_conexion tipo_conexion, const char * ip, const char * puerto)
{
	int conexion;
	int err;

	struct addrinfo hints;
	struct addrinfo * server_info;
	
	memset ( &hints , 0 , sizeof(hints) );
	hints.ai_family 	= AF_INET;
	hints.ai_socktype 	= SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
		
	if (tipo_conexion == CLIENTE)
	{
		err = getaddrinfo ( ip , puerto , &hints , &server_info );
		conexion = socket ( server_info->ai_family, server_info->ai_socktype , server_info->ai_protocol);
	}
	if (tipo_conexion == SERVIDOR)
	{	
		err = getaddrinfo ( NULL , puerto , &hints , &server_info );
		conexion = socket ( server_info->ai_family, server_info->ai_socktype , server_info->ai_protocol);
		
		err = setsockopt ( conexion , SOL_SOCKET , SO_REUSEPORT , &(int){1} , sizeof(int) );
		err = bind ( conexion , server_info->ai_addr , server_info->ai_addrlen );
		err = listen ( conexion , SOMAXCONN );
		if (err == -1) 
		{
    		puts("No se pudo hacer listen");
    		abort();
		}
	}

	freeaddrinfo(server_info);
		
	return conexion;
}

void * atender_cliente (void * argumento)
{
	int * socket = (int *) argumento;
	   
	int operacion = recibir_operacion (*socket);
		
	switch (operacion) 
	{
		case MENSAJE:
				//recibir_mensaje(*cliente, logger);
			break;
		case PAQUETE:
				//lista = recibir_paquete(cliente_fd);
				//log_info(logger, "Me llegaron los siguientes valores:\n");
				//list_iterate(lista, (void*) iterator);
			break;
		case NEW_QUERY:
			printf ("LLego un new query\n");
			t_list * lista = recibir_carga_util (*socket);
			
			printf ("El primer argumento que llego es: %s\n", (char *) list_get(lista, 0));
			printf ("El segundo argumento que llego es: %s\n", (char *) list_get(lista, 1));
			
			list_destroy_and_destroy_elements (lista, free);
			
			t_paquete * paquete = crear_paquete (END_QUERY);
			
			enviar_paquete (paquete, *socket);
			
			destruir_paquete (paquete);

			break;
			
		case DESCONEXION:
				//log_error(logger, "el cliente se desconecto. Terminando servidor");
				//free(cliente);
				//free(parametros);
			return NULL;
		default:
				//log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
	}
	close(*socket);
	free (socket);
	return NULL;
}	
	
t_operacion recibir_operacion(int socket) 
{
	t_operacion operacion;
	// Recibo el codigo de operacion
    if(recv(socket, &operacion, sizeof(t_operacion), 0) > 0)
    {
		return operacion;
	} 
	else 
	{
		close(socket);
		return DESCONEXION;
	}
}

void solicitar_atencion (int socket_cliente, char * ip_servidor, char * puerto_servidor)
{	
	struct addrinfo hints;
	struct addrinfo * server_info;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(ip_servidor, puerto_servidor, &hints, &server_info);//llamada al sistema
	
	if ( connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
	{
		fprintf (stderr, "%s", "No se logro establecer la conexion hacia el servidor ");
		exit (EXIT_FAILURE);
	}

	return;
}

t_paquete * crear_paquete (t_operacion operacion)
{
	t_paquete * paquete = NULL; 
	
	paquete = (t_paquete *) malloc (sizeof( t_paquete ));
		
	paquete->operacion = operacion;
	
	paquete->carga_util = NULL;
	paquete->carga_util = (t_carga_util *) malloc (sizeof (t_carga_util));
	paquete->carga_util->longitud = 0;
	paquete->carga_util->flujo= NULL;

	return paquete;
}

void destruir_paquete (t_paquete * paquete)
{
	if (paquete->carga_util->flujo != NULL)
		free(paquete->carga_util->flujo);
	if (paquete->carga_util !=NULL)
		free(paquete->carga_util);
	if (paquete != NULL)
		free(paquete);
	return;
}

void agregar_a_paquete (t_paquete * paquete, void * cadena, int longitud)
{
	/* realloc: redimensiona un bloque de memoria previamente asignado ptr*/
	// void * realloc(void *ptr, size_t new_size);
	paquete->carga_util->flujo = realloc (	paquete->carga_util->flujo /*origen - Si es NULL realloc actua como un malloc*/, 
											paquete->carga_util->longitud + /*longitud actual*/ +
											sizeof (longitud) /*incremento para el tamano de cadena*/ +
											longitud /*incremento para la cadena*/);
											
	/* memcpy: copia un bloque de memoria(size) de origen a destino */
	//void *memcpy(void *destino, const void * origen, size_t size);
	memcpy (paquete->carga_util->flujo /*<Aca inicia*/ + paquete->carga_util->longitud/*se desplazado hasta el final*/,  
			&longitud/*lo que copia*/, sizeof (longitud)/*el tamaño de lo ue copia*/);
	memcpy (paquete->carga_util->flujo /*<Aca inicia*/ + paquete->carga_util->longitud/*se desplazado hasta el final*/ + 
			sizeof(longitud)/*se desplaza para no pisar la longitud*/, cadena/*lo que copia*/ , longitud/*el tamaño de lo ue copia*/);

	paquete->carga_util->longitud += sizeof(longitud) + longitud;
	
	return;
}

void * serializar_paquete (t_paquete * paquete, int bytes_a_enviar)
{
	void * mensaje_serializado = NULL;
	int desplazamiento = 0;
	
	mensaje_serializado = malloc (bytes_a_enviar);
	/* memcpy: copia un bloque de memoria(size) de origen a destino */
	//void *memcpy(void *destino, const void * origen, size_t size);
	memcpy (mensaje_serializado + desplazamiento, &(paquete->operacion), sizeof (paquete->operacion) );
	desplazamiento += sizeof (paquete->operacion);
	if (paquete->operacion != END_QUERY)
	{
		memcpy ( mensaje_serializado + desplazamiento, &(paquete->carga_util->longitud), sizeof (paquete->carga_util->longitud) );
		desplazamiento += sizeof (paquete->carga_util->longitud);
	
		memcpy ( mensaje_serializado + desplazamiento, paquete->carga_util->flujo, paquete->carga_util->longitud );
	}

	//desplazamiento += paquete->carga_util->longitud; //No se usa

	return mensaje_serializado;
}

void enviar_paquete (t_paquete * paquete, int socket)
{
	int bytes_a_enviar;
	void * mensaje_serializado = NULL;
	if (paquete->operacion != END_QUERY)
		bytes_a_enviar = sizeof (paquete->operacion) + sizeof (paquete->carga_util->longitud) +  paquete->carga_util->longitud;
	else
		bytes_a_enviar = sizeof (paquete->operacion);
	mensaje_serializado = serializar_paquete (paquete, bytes_a_enviar);
	/*ssize_t send(int sock, const void * mensaje_serializado, size_t len, int flags);*/
	send (socket, mensaje_serializado, bytes_a_enviar, 0);
	free (mensaje_serializado);
	
	return;
}

t_list * recibir_carga_util (int socket)
{
	int longitud_flujo;
	int desplazamiento = 0;
	void * carga_util;
	t_list * lista = list_create();
	int longitud_cadena;

	recv (socket, &longitud_flujo, sizeof(longitud_flujo), MSG_WAITALL);
	carga_util = malloc(longitud_flujo);
	recv (socket, carga_util, longitud_flujo, MSG_WAITALL);

	while(desplazamiento < longitud_flujo)
	{
		memcpy ( &longitud_cadena, carga_util + desplazamiento, sizeof (longitud_cadena) );
		desplazamiento += sizeof (longitud_cadena);
		
		char * cadena = (char *) malloc (longitud_cadena);
		
		memcpy (cadena, carga_util + desplazamiento, longitud_cadena);
		desplazamiento += longitud_cadena;
		list_add (lista, cadena);
	}
	free (carga_util);
	
	return lista;
}





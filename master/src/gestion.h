#ifndef _GESTION_H_
#define _GESTION_H_

#include <utils/hello.h>

#define POSICION_ID 0
#define NO_HAY_WORKER_CONECTADOS "No se pudo atender debido a que no hay workers conectados"

extern t_list * workers; 
extern t_list * queries_ready;
extern t_list * queries_execution;
extern int socket_escucha;
extern t_config * configuracion;
extern t_log * bitacora_del_sistema;
extern pthread_mutex_t mutex_workers;
extern int id_global;

/* nuevos */
extern pthread_mutex_t mutex_id_global;
extern pthread_mutex_t mutex_queries; // si usarás una lista queries_ready

typedef struct
{
    char * id;
    int socket;
    bool esta_libre;
} t_worker;

typedef enum 
{
    READY = 0,
    EXECUTING = 1,
    FINISHED = 2
} t_estado_query;

typedef struct {
    char * id;
    int program_counter;
    int socket;
    char * prioridad;
    //time_t tiempo_espera;
    t_estado_query estado;
    t_worker * worker_asignado;   
} t_qcb;


void * gestionar_query_worker (void * socket_de_atencion);
void cerrar_servidor(int signum);
int obtener_indice_worker_libre (void);
int reservar_worker_libre_y_marcar(void); /* busca y marca como ocupado de forma atómica */
char * generar_id (void);
void free_worker(void *elem);

#endif


#ifndef _GESTION_H_
#define _GESTION_H_

#include <utils/hello.h>
#include <limits.h>

#define POSICION_ID 0
#define NO_HAY_WORKER_CONECTADOS "No se pudo atender debido a que no hay workers conectados"
#define ARCHIVO_DE_CONSULTAS 0
#define PRIORIDAD_DE_CONSULTA 1
#define ID_WORKER 0

extern t_list * workers; 
extern t_list * queries_ready;
extern t_list * queries_execution;
extern int socket_escucha;
extern t_config * configuracion;
extern t_log * bitacora_del_sistema;
extern pthread_mutex_t mutex_workers;
extern int id_global;
extern sem_t sem_queries_ready;
extern sem_t sem_workers_libres;

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
    char * archivo;
    char * prioridad;
    t_estado_query estado;
    t_worker * worker_asignado;   
} t_qcb;


void * admitir_queries_workers (void * socket_de_atencion);
void cerrar_servidor(int signum);
int obtener_indice_worker_libre (void);
int reservar_worker_libre_y_marcar(void); /* busca y marca como ocupado de forma atómica */
char * generar_id (void);
void free_worker(void *elem);
void manejar_desconexion_query(int socket_query);

#endif


#include <utils/hello.h>

#define CANTIDAD_ARGUMENTOS 2

/*Variables Externas*/
t_config * configuracion = NULL;
t_log * bitacora_del_sistema = NULL;

int main(int argc, char* argv[]) 
{
    saludar("worker");
    configuracion = config_create ("configuracion");
	bitacora_del_sistema = log_create ("registro_de_eventos.log", "WORKER", true, (t_log_level) config_get_int_value (configuracion, "LOG_LEVEL"));
	
	if (argc != 1 + CANTIDAD_ARGUMENTOS)//Valida que se ingrese el ejecutable + 3 argumentos por el CLA
	{
		fprintf (stderr, "Error: El ejecutable requiere 2 argumentos por linea de comandos\n");
		return EXIT_FAILURE;
	}
	
	
	config_destroy (configuracion);
    log_destroy (bitacora_del_sistema);
    return 0;
}

#include <utils/hello.h>

/*Variables Externas*/
t_config * configuracion = NULL;
t_log * bitacora_del_sistema = NULL;

int main(int argc, char* argv[]) 
{
    saludar("storage");
    configuracion = config_create ("configuracion");
	bitacora_del_sistema = log_create ("registro_de_eventos.log", "STORAGE", true, (t_log_level) config_get_int_value (configuracion, "LOG_LEVEL"));
	
	printf("El punto de montaje es %s\n", config_get_string_value (configuracion, "PUNTO_MONTAJE"));
	
	config_destroy (configuracion);
    log_destroy (bitacora_del_sistema);	
    return 0;
}

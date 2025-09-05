#include <utils/hello.h>

/*Variables Externas*/
t_config * configuracion = NULL;
t_log * bitacora_del_sistema = NULL;

int main(int argc, char* argv[]) 
{
	saludar("query_control");
    configuracion = config_create ("configuracion");
    bitacora_del_sistema = log_create ("registro_de_eventos.log", "QUERY_CONTROL", true, (t_log_level) config_get_int_value (configuracion, "LOG_LEVEL"));
    
    printf("El puerto master es %d\n", config_get_int_value (configuracion, "PUERTO_MASTER"));
    
    config_destroy (configuracion);
    log_destroy (bitacora_del_sistema);

    return 0;
}

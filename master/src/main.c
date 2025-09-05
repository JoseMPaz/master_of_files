#include <utils/hello.h>

/*Variables Externas*/
t_config * configuracion = NULL;
t_log * bitacora_del_sistema = NULL;

int main(int argc, char* argv[]) 
{
    saludar("master");
    configuracion = config_create ("configuracion");
    bitacora_del_sistema = log_create ("registro_de_eventos.log", "MASTER", true, (t_log_level) config_get_int_value (configuracion, "LOG_LEVEL"));
    
    printf("El tiempo de envejecimiento es %s\n", config_get_string_value (configuracion, "TIEMPO_AGING"));
    
    
    config_destroy (configuracion);
    log_destroy (bitacora_del_sistema);	
    return 0;
}

#include "contiki.h"
#include <string.h>
#include "rest-engine.h"
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
static void res_post_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
// Ressource pour contrôler la climatisation
RESOURCE(res_climatisation,
         "title=\"Climatisation: POST/PUT temperature={value}\";rt=\"Control\"",
         NULL,
         res_post_put_handler,
         res_post_put_handler,
         NULL);
static void
res_post_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  size_t len = 0;
  const char *temperature_str = NULL;
  int temperature = 0;
  int success = 1;
  // Récupération de la valeur de température depuis la requête
  if ((len = REST.get_post_variable(request, "temperature", &temperature_str))) {
    PRINTF("Temperature %.*s\n", len, temperature_str);
    temperature = atoi(temperature_str); // Convertir la température en entier
   // Vérification si la température est dans l'intervalle souhaité (par exemple, entre 20°C et 25°C)
    if (temperature < 20 || temperature > 25) {
        // Code pour éteindre la climatisation
      PRINTF("Temperature out of range. Turning off the climatisation.\n");
    } else {
        // Code pour allumer la climatisation
      PRINTF("Temperature in range. Turning on the climatisation.\n");   
    }
  } else {
    success = 0;
  }
  if (!success) {
    REST.set_response_status(response, REST.status.BAD_REQUEST);
  }
}


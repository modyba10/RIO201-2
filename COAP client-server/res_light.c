#include "contiki.h"

#if PLATFORM_HAS_LIGHT

#include <string.h>
#include "rest-engine.h"
#include "dev/light-sensor.h"

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* A simple getter example. Returns 1 or 0 based on the light sensor reading (to control light) */
RESOURCE(res_light,
         "title=\"Ambient light (supports JSON)\";rt=\"LightSensor\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  uint16_t light = light_sensor.value(0) / LIGHT_SENSOR_VALUE_SCALE;
  printf("La lumière è la valeur suivante :");
  printf (light);

  // Change the response based on light intensity
  uint8_t light_value = (light < 100) ? 1 : 0;
  printf ("Le led va donc s'éteindre" );

  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%u", light_value);

  REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
}
#endif /* PLATFORM_HAS_LIGHT */

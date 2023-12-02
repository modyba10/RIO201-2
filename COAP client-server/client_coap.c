client CoAP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "er-coap-engine.h"
#include "dev/button-sensor.h"

#include "dev/serial-line.h"
#include "dev/leds.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

/* FIXME: This server address is hard-coded for Cooja and link-local for unconnected border router. */
#define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0x2001, 0x660, 0x330f, 0x3100, 0, 0, 0, 0xa576)      /* cooja2 */
/* #define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0x1) */

#define LOCAL_PORT      UIP_HTONS(COAP_DEFAULT_PORT + 1)
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)

#define TOGGLE_INTERVAL 2
int toggle_interval = 2;

extern resource_t
  res_hello,
  res_mirror,
  res_chunks,
  res_separate,
  res_push,
  res_event,
  res_sub,
  res_b1_sep_b2,
  res_pressure,
  res_gyros,
  res_accel,

  res_magne;
  

#if PLATFORM_HAS_LEDS
extern resource_t res_leds, res_toggle;
#endif
#if PLATFORM_HAS_LIGHT
#include "dev/light-sensor.h"
extern resource_t res_light;
#endif
#if PLATFORM_HAS_BATTERY
#include "dev/battery-sensor.h"
extern resource_t res_battery;
#endif
#if PLATFORM_HAS_TEMPERATURE
#include "dev/temperature-sensor.h"
extern resource_t res_temperature;
#endif
/*
extern resource_t res_battery;
#endif
#if PLATFORM_HAS_RADIO
#include "dev/radio-sensor.h"
extern resource_t res_radio;
#endif
#if PLATFORM_HAS_SHT11
#include "dev/sht11/sht11-sensor.h"
extern resource_t res_sht11;
#endif
*/
#if PLATFORM_HAS_PRESSURE
#include "dev/pressure-sensor.h"
extern resource_t res_pressure;
#endif
#if PLATFORM_HAS_GYROSCOPE
#include "dev/gyr-sensor.h"
extern resource_t res_gyros;
#endif
#if PLATFORM_HAS_ACCELEROMETER
#include "dev/acc-mag-sensor.h"
extern resource_t res_accel;
#endif
#if PLATFORM_HAS_MAGNETOMETER
#include "dev/acc-mag-sensor.h"
extern resource_t res_magne;
#endif

extern char* res_serial_data;


PROCESS(er_example_client, "Erbium Example Client");
PROCESS(er_example_server, "Erbium Example Server");
AUTOSTART_PROCESSES(&er_example_server, &er_example_client);

uip_ipaddr_t server_ipaddr;
static struct etimer et;

/* Example URIs that can be queried. */
#define NUMBER_OF_URLS 4
/* leading and ending slashes only for demo purposes, get cropped automatically when setting the Uri-Path */
char *service_urls[NUMBER_OF_URLS] =
{ ".well-known/core", "/actuators/toggle", "/sensors/temperature", "/sensors/light" };
#if PLATFORM_HAS_BUTTON
static int uri_switch = 0;
#endif

/* This function is will be passed to COAP_BLOCKING_REQUEST() to handle responses. */
// Assurez-vous que ce fichier d'en-tête est inclus pour utiliser les LEDs

 
void client_chunk_handler(void *response) {


    const uint8_t *chunk;
    int len = coap_get_payload(response, &chunk);

    printf("|%.*s", len, (char *)chunk);

    if(len == 1) { // Vérifie la longueur de la réponse
        if(*chunk == '1') {
            leds_on(LEDS_RED); // Allume la LED rouge si la réponse est '1'
            printf ("LED rouge allumée car la lumière est en dessous de la lumière requise pour bien vivre \n");

        } else if (*chunk == '0') {
            leds_off(LEDS_RED); // Éteint la LED rouge si la réponse est '0'

            printf("LED rouge éteinte car on a assez de lumière dans l'environnement pour bien vivre\n");
        } 
        
        else {
            printf("Réponse inattendue\n");
        }
    } else {
        printf("Réponse invalide\n");
    }

}

PROCESS_THREAD(er_example_client, ev, data)
{
  PROCESS_BEGIN();

  static coap_packet_t request[1];      /* This way the packet can be treated as pointer as usual. */

  SERVER_NODE(&server_ipaddr);

  /* receives all CoAP messages */
  coap_init_engine();

  etimer_set(&et, TOGGLE_INTERVAL * CLOCK_SECOND);

#if PLATFORM_HAS_BUTTON
  SENSORS_ACTIVATE(button_sensor);
  printf("Press a button to request %s\n", service_urls[uri_switch]);
#endif

while (1) {
    PROCESS_YIELD();

    if (etimer_expired(&et)) {
        printf("--Toggle timer--\n");

        /* Requête GET pour la température */
        //coap_init_message(request, COAP_TYPE_CON, COAP_PUT, 0);
        //coap_set_header_uri_path(request, "sensors/temperature");

        PRINT6ADDR(&server_ipaddr);
        PRINTF(" : %u\n", UIP_HTONS(REMOTE_PORT));

        COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request, client_chunk_handler);

        printf("\n--Done temperature--\n");

        /* Requête pour la lumière */
        coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
        coap_set_header_uri_path(request, "sensors/light");

        PRINT6ADDR(&server_ipaddr);
        PRINTF(" : %u\n", UIP_HTONS(REMOTE_PORT));

        COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request, client_chunk_handler_light);

        printf("\n--Done light--\n");

        etimer_reset(&et);
    }
}

  }

  PROCESS_END();
}

PROCESS_THREAD(er_example_server, ev, data)
{
  PROCESS_BEGIN();

  PROCESS_PAUSE();

  PRINTF("Starting Erbium Example Server\n");

#ifdef RF_CHANNEL
  PRINTF("RF channel: %u\n", RF_CHANNEL);
#endif
#ifdef IEEE802154_PANID
  PRINTF("PAN ID: 0x%04X\n", IEEE802154_PANID);
#endif

  PRINTF("uIP buffer: %u\n", UIP_BUFSIZE);
  PRINTF("LL header: %u\n", UIP_LLH_LEN);
  PRINTF("IP+UDP header: %u\n", UIP_IPUDPH_LEN);
  PRINTF("REST max chunk: %u\n", REST_MAX_CHUNK_SIZE);

  /* Initialize the REST engine. */
  rest_init_engine();




#if PLATFORM_HAS_LEDS
/*  rest_activate_resource(&res_leds, "actuators/leds"); */
  rest_activate_resource(&res_toggle, "actuators/toggle");
#endif
#if PLATFORM_HAS_LIGHT
  rest_activate_resource(&res_light, "sensors/light"); 
  SENSORS_ACTIVATE(light_sensor);  
#endif
#if PLATFORM_HAS_BATTERY
  rest_activate_resource(&res_battery, "sensors/battery");  
  SENSORS_ACTIVATE(battery_sensor);  
#endif
#if PLATFORM_HAS_TEMPERATURE
  rest_activate_resource(&res_temperature, "sensors/temperature");  
  SENSORS_ACTIVATE(temperature_sensor);  
#endif
/*
#if PLATFORM_HAS_RADIO
  rest_activate_resource(&res_radio, "sensors/radio");  
  SENSORS_ACTIVATE(radio_sensor);  
#endif
#if PLATFORM_HAS_SHT11
  rest_activate_resource(&res_sht11, "sensors/sht11");  
  SENSORS_ACTIVATE(sht11_sensor);  
#endif
*/
#if PLATFORM_HAS_PRESSURE
  rest_activate_resource(&res_pressure, "sensors/pressure");
  SENSORS_ACTIVATE(pressure_sensor);
#endif
#if PLATFORM_HAS_GYROSCOPE
  rest_activate_resource(&res_gyros, "sensors/gyros");
  SENSORS_ACTIVATE(gyr_sensor);
#endif
#if PLATFORM_HAS_ACCELEROMETER
  rest_activate_resource(&res_accel, "sensors/accel");
  SENSORS_ACTIVATE(acc_sensor);
#endif
#if PLATFORM_HAS_MAGNETOMETER
  rest_activate_resource(&res_magne, "sensors/magne");
  SENSORS_ACTIVATE(mag_sensor);
#endif

  /* Define application-specific events here. */
  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == serial_line_event_message) {
      res_serial_data = (char*)data;

      printf("recv\n");

      /* Call the event_handler for this application-specific event. */
      res_event.trigger();

      /* Also call the separate response example handler. */
      // res_separate.resume();
    }
  }                             /* while (1) */

  PROCESS_END();
}
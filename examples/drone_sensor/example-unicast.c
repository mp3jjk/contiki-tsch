/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Best-effort single-hop unicast example
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime/rime.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>
#include <stdlib.h>
//#include "platform-conf.h"

#if DUAL_RADIO
#ifdef ZOLERTIA_Z1
#include	"../platform/z1/dual_radio.h"
#elif COOJA /* ZOLERTIA_Z1 */
#include	"../platform/cooja/dual_conf.h"
#else /* ZOLERTIA_Z1 */
#include "../platform/zoul/dual_radio.h"
#endif /* ZOLERTIA_Z1 */
#endif /* DUAL_RADIO */
static int led_count;


/*---------------------------------------------------------------------------*/
PROCESS(example_unicast_process, "Example unicast");
AUTOSTART_PROCESSES(&example_unicast_process);
/*---------------------------------------------------------------------------*/
static void
recv_uc(struct broadcast_conn *c, const linkaddr_t *from)
{

  printf("From %d.%d: ",
	 from->u8[0], from->u8[1]);

  printf("%s\n",
         (char *)packetbuf_dataptr());

	led_count ++;
	switch (led_count%3) {
		case 0: 
			leds_off(LEDS_BLUE);
			leds_on(LEDS_RED);
			break;
		case 1: 
			leds_off(LEDS_RED);
			leds_on(LEDS_GREEN);
			break;
		case 2: 
			leds_off(LEDS_GREEN);
			leds_on(LEDS_BLUE);
			break;
	}
}
static const struct broadcast_callbacks broadcast_callbacks = {recv_uc};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_unicast_process, ev, data)
{
  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
    
  PROCESS_BEGIN();
	SENSORS_ACTIVATE(button_sensor);
	
  static int count=0;
	led_count=0;
	broadcast_open(&broadcast, 129, &broadcast_callbacks);
	leds_on(LEDS_RED);
	leds_on(LEDS_BLUE);
	leds_on(LEDS_GREEN);

	PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event &&
                          data == &button_sensor);
	leds_off(LEDS_RED);
	leds_off(LEDS_BLUE);
	leds_off(LEDS_GREEN);

  while(1) {
		count ++;
    static struct etimer et;
		static char * buf;
    linkaddr_t addr;
  	/*
 		if(count%2 ==0){
 			dual_radio_switch(SHORT_RADIO);			
 		} else {
			dual_radio_switch(LONG_RADIO);
 		}*/

    etimer_set(&et, CLOCK_SECOND/10);
    
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

		buf = (char *) malloc(20);
		sprintf(buf,"Hello %d\0",count);
    packetbuf_copyfrom(buf, 20);
    addr.u8[0] = 0;
    addr.u8[1] = 1;
		// printf("ADDRESS: %d %d\n", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
    if(!linkaddr_cmp(&addr, &linkaddr_node_addr)) {
      broadcast_send(&broadcast);
			printf("COUNT = %d\n",count);
    }
		free (buf);
		if (count == 10000){
			break;
		}
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

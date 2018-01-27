/*
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

#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"
#include "sys/ctimer.h"
#include "net/nbr-table.h"
#include "rpl/rpl-private.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"

#ifdef WITH_COMPOWER
#include "powertrace.h"
#endif
#include <stdio.h>
#include <string.h>

/* Only for TMOTE Sky? */
#include "dev/serial-line.h"
#include "dev/uart1.h"
#include "net/ipv6/uip-ds6-route.h"

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define UDP_EXAMPLE_ID  190

#define DEBUG DEBUG_FULL
//#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

//#ifndef PERIOD
//#define PERIOD 0	// defined in lanada/param.h
//#endif

#ifdef PERIOD
#define PS (600 / PERIOD)
#endif

#ifdef ARRIVAL_RATE
#define PS (600 / ARRIVAL_RATE)
#endif

#include "param.h"

#define START_INTERVAL		(15 * CLOCK_SECOND)
#if TRAFFIC_MODEL == 0
#define SEND_INTERVAL		(PERIOD * CLOCK_SECOND)
#define SEND_TIME		(random_rand() % (SEND_INTERVAL))
#endif
#define MAX_PAYLOAD_LEN		50


#if TRAFFIC_MODEL == 1
#include "lib/random.h"
#include <math.h>
#endif

#include "core/sys/residual.h"
#include "core/sys/log_message.h"

#if PS_COUNT
static int lifetime = 1;
#endif

/* Remaining energy init JJH*/
//int id_array[MAX_NUM_NODE]={0,};
//uint8_t id_count[BUF_SIZE]={0,};
//extern uint8_t id_count[BUF_SIZE]={0,};

//struct uip_udp_conn *conn_backup;
struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;
//uip_ipaddr_t* before_addr;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static int seq_id;
static int reply;
static uint8_t myaddr;

static void
tcpip_handler(void)
{
  char *str;

  if(uip_newdata()) {
    str = uip_appdata;
    str[uip_datalen()] = '\0';
    reply++;
    printf("DATA recv '%s' (s:%d, r:%d)\n", str, seq_id, reply);
  }
}
/*---------------------------------------------------------------------------*/
static void
send_packet(void *ptr)
{
//	before_addr=&client_conn->ripaddr;
  char buf[MAX_PAYLOAD_LEN];
	static char radio_temp = 'M';
	static int parent_temp = 0;

/*	printf("client_conn %p conn_backup %p\n",client_conn,conn_backup);
	if(client_conn != conn_backup)
	{
		client_conn = conn_backup;
	}
	printf("client_conn restore %p\n",client_conn);*/
#ifdef SERVER_REPLY
  uint8_t num_used = 0;
  uip_ds6_nbr_t *nbr;
	
  nbr = nbr_table_head(ds6_neighbors);
  while(nbr != NULL) {
    nbr = nbr_table_next(ds6_neighbors, nbr);
    num_used++;
  }

  if(seq_id > 0) {
    ANNOTATE("#A r=%d/%d,color=%s,n=%d %d\n", reply, seq_id,
             reply == seq_id ? "GREEN" : "RED", uip_ds6_route_num_routes(), num_used);
  }
#endif /* SERVER_REPLY */


#if RPL_ICMP_ENERGY_LOG
	LOG_MESSAGE("DATA_PACKET, Energy: %d, Number: %d\n",(int) get_residual_energy(), seq_id); 
#endif
	seq_id++;

#if PS_COUNT
  data_message_count = seq_id-1;
	int total_count1, total_count2;
	total_count1 = dio_count + dao_count + dis_count + dio_ack_count;
	total_count2 = dao_ack_count + dao_fwd_count + dao_ack_fwd_count + LSA_count;
	if (data_message_count%PS == 0) {
		LOG_MESSAGE("[PS] Periodic status review:\n");
		LOG_MESSAGE("[PS] Control: %d, Data: %d, Data fwd: %d\n", 
				tcp_output_count-data_message_count-data_fwd_count, data_message_count, data_fwd_count);
		LOG_MESSAGE("[PS] ICMP: %d, TCP_OUTPUT: %d\n",
				icmp_count, tcp_output_count);
		LOG_MESSAGE("[PS] DIO:%d, DAO: %d, DIS: %d, DIO_ACK: %d, Total: %d\n", 
				dio_count, dao_count, dis_count, dio_ack_count, total_count1);
		LOG_MESSAGE("[PS] DAO_ACK:%d, DAO_FWD: %d, DAO_ACK_FWD: %d, LSA: %d, Total: %d\n",
				dao_ack_count, dao_fwd_count,dao_ack_fwd_count, LSA_count, total_count2 );
		LOG_MESSAGE("[PS] CSMA_Transmission: %d, CXMAC_Transmission: %d, CXMAC_Collision: %d\n", 
				csma_transmission_count, rdc_transmission_count, rdc_collision_count);
		LOG_MESSAGE("[PS] CSMA_Drop: %d, CXMAC_Retransmission: %d\n",
				csma_drop_count, rdc_retransmission_count - csma_drop_count);
		LOG_MESSAGE("[PS] Remaining energy: %d\n", (int) get_residual_energy());
		rpl_parent_t *p = nbr_table_head(rpl_parents);
		if (p != NULL) {
			rpl_parent_t *preferred_parent = p->dag->preferred_parent;
			if (preferred_parent != NULL) {
				uip_ds6_nbr_t *nbr = rpl_get_nbr(preferred_parent);
				LOG_MESSAGE("[PS] My parent is : %c %d\n", nbr->ipaddr.u8[8]>128 ? 'L':'S', nbr->ipaddr.u8[15]) ;
			}
		}
	}

	if (lifetime > 0) {
		if (get_residual_energy() == 0) {		
			LOG_MESSAGE("[LT] Control: %d, Data: %d, Data fwd: %d\n", 
					tcp_output_count-data_message_count-data_fwd_count, data_message_count, data_fwd_count);
			LOG_MESSAGE("[LT] ICMP: %d, TCP_OUTPUT: %d\n",
					icmp_count, tcp_output_count);
			LOG_MESSAGE("[LT] DIO:%d, DAO: %d, DIS: %d, DIO_ACK: %d, Total: %d\n", 
					dio_count, dao_count, dis_count, dio_ack_count, total_count1);
			LOG_MESSAGE("[LT] DAO_ACK:%d, DAO_FWD: %d, DAO_ACK_FWD: %d, LSA: %d, Total: %d\n",
					dao_ack_count, dao_fwd_count,dao_ack_fwd_count, LSA_count, total_count2 );
			LOG_MESSAGE("[LT] CSMA_Transmission: %d, CXMAC_Transmission: %d, CXMAC_Collision: %d\n", 
					csma_transmission_count, rdc_transmission_count, rdc_collision_count);
			LOG_MESSAGE("Lifetime of this node ended here!!!\n");

		 	NETSTACK_MAC.off(0);
			dead = 1;
		}
	}
	lifetime = get_residual_energy();
#endif /* PS_COUNT */
	if(seq_id > (latest_id + 1)) // Update latest id here
	{
		latest_id = (seq_id-1);
//		count_index = latest_id % BUF_SIZE;
//		printf("load %d at %d\n",id_count[latest_id],latest_id);
		int temp_load = id_count[latest_id] * 256;
		if(avg_est_load == -1) {
			avg_est_load = temp_load;
			init_phase = 0; // After first load update, move into routing mode
		}
		else {
			avg_est_load = ((uint32_t)avg_est_load * ALPHA +
					(uint32_t)temp_load * (ALPHA_SCALE - ALPHA)) / ALPHA_SCALE;
		}
//		printf("load %d avg_est_load %d\n",id_count[latest_id],avg_est_load);
	}
  PRINTF("app: DATA id:%04d from:%03d\n",
         seq_id,myaddr);
//  printf("send_packet!\n");
#if ZOUL_MOTE
	rpl_parent_t *p2 = nbr_table_head(rpl_parents);
	
	if (p2 != NULL) {
		rpl_parent_t *preferred_parent2 = p2->dag->preferred_parent;
		if (preferred_parent2 != NULL) {
			uip_ds6_nbr_t *nbr2 = rpl_get_nbr(preferred_parent2);
		 	radio_temp = nbr2->ipaddr.u8[8]>128 ? 'L':'S';
			parent_temp = nbr2->ipaddr.u8[15];
		}
	} else {
		parent_temp = 0;
		radio_temp = 'M';
	}
  sprintf(buf,"DATA id:%04d from:%03dX E:%d P:%c %d",seq_id,myaddr,(int)get_residual_energy(),\
			 radio_temp, parent_temp);
  uip_udp_packet_sendto(client_conn, buf, 50,
                        &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
#else

	sprintf(buf,"DATA id:%04d from:%03dX",seq_id,myaddr);

	uip_udp_packet_sendto(client_conn, buf, 50,
                        &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
#endif

}
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Client IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
    	myaddr=uip_ds6_if.addr_list[i].ipaddr.u8[15];
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
	uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;

  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
#if DUAL_RADIO
  uip_ipaddr_t long_ipaddr;

  uip_ip6addr(&long_ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&long_ipaddr, &uip_long_lladdr);
  uip_ds6_long_addr_add(&long_ipaddr, 0, ADDR_AUTOCONF);


#endif
/* The choice of server address determines its 6LoPAN header compression.
 * (Our address will be compressed Mode 3 since it is derived from our link-local address)
 * Obviously the choice made here must also be selected in udp-server.c.
 *
 * For correct Wireshark decoding using a sniffer, add the /64 prefix to the 6LowPAN protocol preferences,
 * e.g. set Context 0 to fd00::.  At present Wireshark copies Context/128 and then overwrites it.
 * (Setting Context 0 to fd00::1111:2222:3333:4444 will report a 16 bit compressed address of fd00::1111:22ff:fe33:xxxx)
 *
 * Note the IPCMV6 checksum verification depends on the correct uncompressed addresses.
 */
 
#if 0
/* Mode 1 - 64 bits inline */
   uip_ip6addr(&server_ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 1);
#elif 1
/* Mode 2 - 16 bits inline */
  uip_ip6addr(&server_ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);
#else
/* Mode 3 - derived from server link-local (MAC) address */
  uip_ip6addr(&server_ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0x0250, 0xc2ff, 0xfea8, 0xcd1a); //redbee-econotag
#endif
}

void polling(void* ptr){
	// printf("polling\n");
	process_poll(&udp_client_process);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer arrival,routing;
  static struct ctimer backoff_timer;
#if TRAFFIC_MODEL == 1 // Poisson traffic
  static clock_time_t poisson_int;
  float rand_num;
#endif
#if WITH_COMPOWER
  static int print = 0;
#endif

  PROCESS_BEGIN();
	SENSORS_ACTIVATE(button_sensor);
	led_end = 0;
	dead = 0;
	avg_est_load = -1; // Initial value -1
	init_phase = 1; // Init phase start

  PROCESS_PAUSE();

  set_global_address();
#if TRAFFIC_MODEL == 0
  PRINTF("UDP client process started nbr:%d routes:%d period:%d\n",
			NBR_TABLE_CONF_MAX_NEIGHBORS, UIP_CONF_MAX_ROUTES, PERIOD);
#elif TRAFFIC_MODEL == 1
  PRINTF("UDP client process started nbr:%d routes:%d poisson_avg:%d\n",
			NBR_TABLE_CONF_MAX_NEIGHBORS, UIP_CONF_MAX_ROUTES, ARRIVAL_RATE);
#endif

  print_local_addresses();
	 /* NETSTACK_MAC.off(1); */

  /* new connection with remote host */
  client_conn = udp_new(NULL, UIP_HTONS(UDP_SERVER_PORT), NULL); 
  if(client_conn == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }
  udp_bind(client_conn, UIP_HTONS(UDP_CLIENT_PORT)); 

  PRINTF("Created a connection with the server ");
  PRINT6ADDR(&client_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n",
	UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));
//  PRINTF("ON TIME %d\n",DEFAULT_ON_TIME);
//  PRINTF("OFF TIME %d\n",DEFAULT_OFF_TIME);
//  conn_backup = client_conn;
	

#if WITH_COMPOWER
  powertrace_sniff(POWERTRACE_ON);
#endif
	
	join_instance = 0;
//	PROCESS_WAIT_EVENT_UNTIL(join_instance == 1);
	static struct ctimer client_poll;
	while(join_instance == 0){
	//	printf("join_instance: %d\n",join_instance);
		ctimer_set(&client_poll,(3ul * CLOCK_SECOND),&polling,NULL);
		PROCESS_YIELD();
#ifdef ZOUL_MOTE
		if(ev == sensors_event && data == & button_sensor) {
			printf("*************** RESET LOG MESSAGE **************************\n");
			cfs_remove("log_message");
			log_reinit();
		}
#endif
	}
	ctimer_stop(&client_poll);

//	printf("process_start\n");
#if ZOUL_MOTE
	led_end = 1;
#endif
//	etimer_set(&routing,60 * CLOCK_SECOND);
#if TRAFFIC_MODEL == 0 // Periodic
  etimer_set(&arrival, SEND_INTERVAL);
#elif TRAFFIC_MODEL == 1 // Poisson traffic
  rand_num=random_rand()/(float)RANDOM_RAND_MAX;
  //  printf("rand_num %f\n",rand_num);
  poisson_int = (-ARRIVAL_RATE) * logf(rand_num) * CLOCK_SECOND;
  if(poisson_int == 0)
	  poisson_int = 1;
  //  printf("poisson %d\n",poisson_int);
  etimer_set(&arrival, poisson_int);
#endif

  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
		}
#ifdef ZOUL_MOTE
    if(ev == sensors_event && data == & button_sensor) {
			printf("*************** RESET LOG MESSAGE **************************\n");
			cfs_remove("log_message");		
			log_reinit();
		}
#endif

    if(ev == serial_line_event_message && data != NULL) {
      char *str;
      str = data;
      if(str[0] == 'r') {
        uip_ds6_route_t *r;
        uip_ipaddr_t *nexthop;
        uip_ds6_defrt_t *defrt;
        uip_ipaddr_t *ipaddr;
        defrt = NULL;
        if((ipaddr = uip_ds6_defrt_choose()) != NULL) {
          defrt = uip_ds6_defrt_lookup(ipaddr);
        }
        if(defrt != NULL) {
          PRINTF("DefRT: :: -> %02d", defrt->ipaddr.u8[15]);
          PRINTF(" lt:%lu inf:%d\n", stimer_remaining(&defrt->lifetime),
                 defrt->isinfinite);
        } else {
          PRINTF("DefRT: :: -> NULL\n");
        }

        for(r = uip_ds6_route_head();
            r != NULL;
            r = uip_ds6_route_next(r)) {
          nexthop = uip_ds6_route_nexthop(r);
          PRINTF("Route: %02d -> %02d", r->ipaddr.u8[15], nexthop->u8[15]);
          /* PRINT6ADDR(&r->ipaddr); */
          /* PRINTF(" -> "); */
          /* PRINT6ADDR(nexthop); */
          PRINTF(" lt:%lu\n", r->state.lifetime);

        }
      }
    }

    if(etimer_expired(&arrival)) {
#if TRAFFIC_MODEL == 0
    	etimer_restart(&arrival);
        ctimer_set(&backoff_timer, SEND_TIME, send_packet, NULL);
#elif TRAFFIC_MODEL == 1
        ctimer_set(&backoff_timer, random_rand() % (poisson_int), send_packet, NULL);

	rand_num=random_rand()/(float)RANDOM_RAND_MAX;
	//	printf("rand_num %f\n",rand_num);

        poisson_int = (-ARRIVAL_RATE) * logf(rand_num) * CLOCK_SECOND;
	//    	printf("poisson %d\n",poisson_int);
        if(poisson_int == 0)
      	  poisson_int = 1;
    	etimer_set(&arrival, poisson_int);
#endif

#if WITH_COMPOWER
      if (print == 0) {
	powertrace_print("#P");
      }
      if (++print == 3) {
	print = 0;
      }
#endif

    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

#!/bin/bash

CONTITIKI=~/Desktop/Double-MAC/lanada

if [ $1 == "h" ]
then
    echo "USAGE: weight periodic0/poisson1 period rate ENHANCED CHECK_RATE OF0/LTMAX"
    exit 1
fi

if [ $7 == 1 ]
then
    sed -i 's/\#define RPL_CONF_OF rpl_of0/\#define RPL_CONF_OF rpl_ltmax2_of/g' $CONTIKI/platform/zoul/contiki-conf.h
else
    sed -i 's/\#define RPL_CONF_OF rpl_ltmax2_of/\#define RPL_CONF_OF rpl_of0/g' $CONTIKI/platform/zoul/contiki-conf.h
fi

sed -i 's/\#define RESIDUAL_ENERGY_MAX 2000000/\#define RESIDUAL_ENERGY_MAX 1000000000/g' $CONTIKI/core/sys/residual.h
sed -i 's/\#define RESIDUAL_ENERGY_MAX 4000000/\#define RESIDUAL_ENERGY_MAX 1000000000/g' $CONTIKI/core/sys/residual.h

echo "#define RPL_ENERGY_MODE 0
#define RPL_LIFETIME_MAX_MODE 0	// Child information is saved in each node
#define RPL_LIFETIME_MAX_MODE2 $7 // Improving LT MAX MODE

#define PROB_PARENT_SWITCH 0

/* Distributed weight update problem solutions */
#define MODE_DIO_WEIGHT_UPDATED 0
#define MODE_LAST_PARENT	0 // Tx Last parent info. in dio_ack
#define MODE_PARENT_UPDATE_IN_ROUND 0 // Update parent only when round is synchronized // Not implemented yet

/* OF best parent selection strategy */
#define OF_MWHOF	0 // Minimum Weight Hysteresis Objective Function

/* Metric ratio between weight and rank */
//#define ALPHA 2
/* Weight ratio between long and short*/
#define LONG_WEIGHT_RATIO $1

/* Weight ratio between rank and parent's degree */
#define ALPHA	1
#define ALPHA_DIV	1

#undef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE $6

/* Sink's infinite energy */
#define SINK_INFINITE_ENERGY	1
#if RPL_LIFETIME_MAX_MODE2
uint8_t tree_level; // The candidates set of the most loaded node
#endif

/* Using strobe cnt, reducing idle listening while Tx preamble */
#ifdef ZOUL_MOTE
#define STROBE_CNT_MODE 0
#else
#define STROBE_CNT_MODE		0
#endif

/* To determine valid parent set, only valid parents are considered as a parent set */
#define PARENT_REDUCTION_MODE	0
#define VALID_PARENT_RATIO	0

/* Enabling Data ACK */
#define DATA_ACK      1
#define ACK_WEIGHT_INCLUDED		1
uint8_t data_btb; // Back to back data Tx

/* Energy log */
#define RPL_ICMP_ENERGY_LOG		0

/* Data aggregation shceme enabled or not */
#define DATA_AGGREGATION 1	// not implemented yet

/* Dual routing converge */
#define DUAL_ROUTING_CONVERGE 	0

/* Allow only a update per round */
#define SINGLE_UPDATE_ROUND	0

/* Energy consumption X during routing */
#define ROUTING_NO_ENERGY 0
#define ENERGY_CONV_TIME (900ul * CLOCK_SECOND)

/* LSA-MAC, implemeted on cxmac
 * Preamble free short broadcast after long broadcast, dual broadcast is included in LSA-MAC
 * Only long duty cylce, long preamble */
#if DUAL_RADIO
#define LSA_MAC	1
#define LSA_R	0
#define LSA_ENHANCED $5
#else	/* DUAL_RADIO */
#define LSA_MAC 0
#define LSA_R 0
#endif /* DUAL_RADIO */

#define SERVER_NODE 1

#define TRAFFIC_MODEL $2 // 0: Periodic, 1: Poisson
#if TRAFFIC_MODEL == 0
#define PERIOD $3
#elif TRAFFIC_MODEL == 1
#define ARRIVAL_RATE $4 // Mean value, 1/lambda
#endif

uint8_t dead;
uint8_t join_instance;

#if DUAL_ROUTING_CONVERGE
uint8_t long_duty_on;
uint8_t short_duty_on;
#define CONVERGE_TIME	(100ul * CLOCK_SECOND) // Convergence time in second
#endif

#if LSA_R
uint8_t LSA_converge;
uint8_t LSA_SR_preamble;
uint8_t LSA_lr_child;
uint8_t LSA_message_input;
uint8_t LSA_message_flag;
uint8_t LSA_broadcast_count;
#define CONVERGE_MODE	2

/* CONVERGE_MODE 1 */
#define MAX_LSA_RETRANSMISSION 3
#define LSA_CONVERGE_TIME	(900ul * CLOCK_SECOND) // Convergence time in second
#define LSA_MESSAGE_TIME	(100ul * CLOCK_SECOND) // Convergence time in second
#define LSA_BROADCAST_TIME	(1ul * CLOCK_SECOND) // Convergence time in second

/* CONVERGE_MODE 2 */
uint8_t simple_convergence;
#define SIMPLE_CONV_TIME (1800ul * CLOCK_SECOND)


#ifdef RPL_LIFETIME_MAX_MODE
#undef RPL_LIFETIME_MAX_MODE
#endif /* RPL_LIFETIME_MAX_MODE */
#define RPL_LIFETIME_MAX_MODE 0	// Child information is saved in each node
#define RPL_LIFETIME_MAX_MODE2 0 // Is it fine?
#endif /* LSA_R */

#if RPL_ENERGY_MODE
uint8_t remaining_energy;
uint8_t alpha;
#define LONG_ETX_PENALTY 5

#elif RPL_LIFETIME_MAX_MODE || RPL_LIFETIME_MAX_MODE2
#ifdef ZOUL_MOTE
#define RPL_ETX_WEIGHT 	1
#else
#define RPL_ETX_WEIGHT 	1
#endif
uint8_t my_weight;
uint8_t my_sink_reachability;
uint8_t my_parent_number;
uint8_t init_phase; // It is in init_phase while it is 1
#if PARENT_REDUCTION_MODE
uint8_t my_valid_parent_number;
#endif
#define LOAD_SCALE 	100
#define LOAD_ALPHA	90
uint8_t parent_update; /* Update parent only once for each data_id */
#endif /* RPL_ENERGY_MODE */
#define MAX_NUM_NODE 50
#define BUF_SIZE 5000
extern int id_array[MAX_NUM_NODE];
extern uint8_t id_count[BUF_SIZE];
int latest_id;
int count_index;
int avg_est_load; // Exponentially Weighted Moving Average with est_load


//#if LSA_MAC
#ifdef ZOUL_MOTE
#define SHORT_SLOT_LEN	(RTIMER_ARCH_SECOND / 160 * 4) // Short on time slot length in rtimer
#define BEFORE_SHORT_SLOT	(RTIMER_ARCH_SECOND / 160 * 0)
#else
#define SHORT_SLOT_LEN	(RTIMER_ARCH_SECOND / 160 * 2) // Short on time slot length in rtimer
#endif
//#endif

/*-----------------------------------------------------------------------------------------------*/
#define DETERMINED_ROUTING_TREE	0

#if DETERMINED_ROUTING_TREE
#define MAX_NODE_NUMBER 30

#endif /* ROUTING_TREE */" > ./param.h

/* Log levels */
#include "log_message.h"
#include "lanada/param.h"
#include "sys/residual.h"
#include "net/rpl/rpl.h"
#include "net/mac/dualmac/dualmac.h"

#ifdef COOJA
#include "net/linkaddr.h"
FILE *log_fp;
#else	/* COOJA */
int log_file;
#endif	/* COOJA */


void log_initialization(void){
#if PS_COUNT
	csma_drop_count = 0;
	rdc_collision_count = 0;
	rdc_retransmission_count = 0;
	rdc_transmission_count = 0;
	csma_transmission_count = 0;
	control_message_count = 0;
	data_message_count = 0;
	data_fwd_count = 0;
	dio_count = 0;
	dis_count = 0;
	dao_count = 0;
	dao_fwd_count = 0;
	dao_ack_count = 0;
	dao_ack_fwd_count = 0;
	LSA_count = 0;
	dio_ack_count = 0;
	icmp_count = 0;
	tcp_output_count = 0;
#endif

#if SIMULATION_SETTING
	printf("\n\nDUAL_RADIO: %d\nADDR_MAP: %d\nRPL_LIFETIME_MAX_MODE: %d\nLONG_WEIGHT_RATIO: %d\nSTROBE_CNT_MODE: %d\n", \
			DUAL_RADIO, ADDR_MAP, DUAL_RPL_RECAL_MODE, \
			LONG_WEIGHT_RATIO, STROBE_CNT_MODE);
	printf("DETERMINED_ROUTING_TREE: %d\nRESIDUAL_ENERGY_MAX: %d\n", \
			DETERMINED_ROUTING_TREE, RESIDUAL_ENERGY_MAX);
#ifdef COOJA
	printf("DISSIPATION_RATE: %d, %d, %d, %d, %d, %d, %d\n",DISSIPATION_RATE[0],DISSIPATION_RATE[1],\
			DISSIPATION_RATE[2],DISSIPATION_RATE[3],DISSIPATION_RATE[4],DISSIPATION_RATE[5],DISSIPATION_RATE[6]);
#endif

#if TRAFFIC_MODEL == 0
	printf("PERIOD: %d\n", PERIOD);
#elif TRAFFIC_MODEL ==1
	printf("POISSON_RATE: %d\n", ARRIVAL_RATE);
#endif
#endif
	

#ifdef COOJA
	char filename[100];
	sprintf(filename, "/home/user/Desktop/Debug_log/log_message%d.txt",linkaddr_node_addr.u8[1]);
	printf("\nOpening the file for cooja\n\n");
	log_fp = fopen(filename, "w");
#else	/* COOJA */
	
	static int fd = 0;
	static int block_size = MAX_BLOCKSIZE;
  char *filename = "log_message";
  int len;
  int offset = 0;
  char buf[MAX_BLOCKSIZE];

  fd = cfs_open(filename, CFS_READ);
          
  printf("Reading the log\n\n\n");
  
  if(fd < 0) {
    printf("Can't open the log file.\n");
  }   
  else {
    printf("LOG_START:----------------------------------------------------------\n");
    while(1) {
      cfs_seek(fd, offset, CFS_SEEK_SET);
      len = cfs_read(fd, buf, block_size);
      offset += block_size;
      if(len <= 0) {
        cfs_close(fd);
        break;
      }   
      printf("%s", buf);
    }   
    printf("\nLOG_END----------------------------------------------------------\n");
  }

	printf("\nOpening the file for z1/firefly\n\n");
	log_file = cfs_open("log_message", CFS_WRITE | CFS_APPEND);
#endif /* COOJA */
}

void log_reinit(void){
#ifdef COOJA

#else
	log_file = cfs_open("log_message", CFS_WRITE | CFS_APPEND);
#endif
}

void log_finisher(void){
#ifdef COOJA
	printf("\nClosing the file for cooja\n\n");
	fclose(log_fp);
#else
	printf("\nClosing the file for z1/firefly\n\n");
	cfs_close(log_file);
#endif
}

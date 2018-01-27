/* Log levels */
#include <stdio.h>
#include "cfs/cfs.h"
#include "sys/residual.h"
#include <stdlib.h>
#include <string.h>

#ifdef COOJA
extern FILE *log_fp;
#else	/* COOJA */
extern int log_file;
#endif	/* COOJA */

#define LOG_LEVEL		2
#define SIMULATION_SETTING	1
#define PS_COUNT 1
#define MAX_BLOCKSIZE 200

#if SIMULAITON_SETTING
extern const energy_t DISSIPATION_RATE;
#endif

#if PS_COUNT
/* This status log takes large size of memory, maybe we should remove this part for z1/firefly */
int csma_drop_count, rdc_retransmission_count, rdc_collision_count, rdc_transmission_count, csma_transmission_count;
int control_message_count, data_message_count, data_fwd_count;
int dio_count, dis_count, dao_count, dao_fwd_count, dao_ack_count, dao_ack_fwd_count, dio_ack_count, LSA_count;
int icmp_count, tcp_output_count;
#endif

void log_initialization(void);
void log_finisher(void);


#if LOG_LEVEL == 0
#define LOG_MESSAGE(...) printf(__VA_ARGS__)

#elif	LOG_LEVEL == 1
#ifdef COOJA
#define LOG_MESSAGE(...) do{\
	fprintf(log_fp, __VA_ARGS__);\
	fflush(log_fp);\
}while(0)
#else	/* COOJA */
#define LOG_MESSAGE(...) do{\
		char *log  = (char*) malloc(sizeof(char)*MAX_BLOCKSIZE);\
		sprintf(log,__VA_ARGS__);\
		printf("[log_success %d]\t",cfs_write(log_file, log, strlen(log)));\
		free(log);\
	}while(0)
#endif	/* COOJA */

#elif LOG_LEVEL == 2
#ifdef COOJA
#define LOG_MESSAGE(...) do{\
		printf(__VA_ARGS__);\
		fprintf(log_fp, __VA_ARGS__);\
		fflush(log_fp);\
	}while(0)

#else /* COOJA */
#define LOG_MESSAGE(...) do{\
		char *log  = (char*) malloc(sizeof(char)*MAX_BLOCKSIZE);\
		sprintf(log,__VA_ARGS__);\
		printf("[log_success %d]\t",cfs_write(log_file, log,strlen(log)));\
		free(log);\
		printf(__VA_ARGS__);\
	}while(0)
//		printf("log_success %d\n",cfs_write(log_file, __VA_ARGS__,MAX_BLOCKSIZE));
#endif	/* COOJA */

#elif LOG_LEVEL == 3
#define LOG_MESSAGE(...)
#endif

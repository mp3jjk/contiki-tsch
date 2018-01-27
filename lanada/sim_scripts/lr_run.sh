#!/bin/bash

JOONKI=0


if [ $JOONKI -eq 0 ]
then
    CONTIKI=/media/user/Harddisk/Double-MAC
else
    CONTIKI=~/Desktop/Double-MAC
fi

echo "Long range simulation"
sed -i 's/\#define DUAL_RADIO 0/\#define DUAL_RADIO 1/g' $CONTIKI/platform/cooja/contiki-conf.h
sed -i 's/\#define TCPIP_CONF_ANNOTATE_TRANSMISSIONS 1/\#define TCPIP_CONF_ANNOTATE_TRANSMISSIONS 0/g' $CONTIKI/platform/cooja/contiki-conf.h

topology=$1
TRAFFIC_MODEL=$2
PERIOD=$3
ARRIVAL_RATE=$4
ALPHA=$5
STROBE_CNT=$6
LONG_WEIGHT=$7
LSA_R=$8
LR_range=$9
PARENT_REDUCTION=${10}
REDUCTION_RATIO=${11}
DATE=${12}
DATA_ACK=${13}
LSA_MAC=${14}
ALPHA_DIV=${15}
CHECK=${16}
LSA_ENHANCED=${17}
ROUTING_NO_ENERGY=${18}
ONLY_LONG=${19}
SEED_NUMBER=${20}
MRM=${21}
LT_PERCENT=${22}

sed -i "11s/.*/    <randomseed>$SEED_NUMBER<\/randomseed>/" $CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\.csc 

if [ $topology == "36grid_mrm2_cnt" ]
then
    sed -i "1124s/.*/var death = $LT_PERCENT;\&\#xD;/g" $CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\.csc 
elif [ $topology == "50random_mrm2_cnt" ]
then
    sed -i "1488s/.*/var death = $LT_PERCENT;\&\#xD;/g" $CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\.csc 
elif [ $topology == "34cluster_mrm2_cnt" ]
then
    sed -i "1072s/.*/var death = $LT_PERCENT;\&\#xD;/g" $CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\.csc
fi

#DIR=$DATE\_topo$topology\_traffic$TRAFFIC_MODEL\_period$PERIOD\_alpha$ALPHA\_$ALPHA_DIV\_mrm$MRM\_seed$SEED_NUMBER
if [ $TRAFFIC_MODEL -eq 0 ]
then
    DIR=$DATE\_topo$topology\_traffic$TRAFFIC_MODEL\_period$PERIOD\_alpha$ALPHA\_$ALPHA_DIV\_seed$SEED_NUMBER
else
    DIR=$DATE\_topo$topology\_traffic$TRAFFIC_MODEL\_rate$ARRIVAL_RATE\_alpha$ALPHA\_$ALPHA_DIV\_seed$SEED_NUMBER
fi

#    cd $DATE\_topo$topology\_traffic$TRAFFIC_MODEL\_period$PERIOD\_alpha$ALPHA\_$ALPHA_DIV\_seed$SEED_NUMBER
mkdir $DIR
cd $DIR
#    mkdir $DATE\_topo$topology\_traffic$TRAFFIC_MODEL\_rate$ARRIVAL_RATE\_alpha$ALPHA\_$ALPHA_DIV\_seed$SEED_NUMBER
#    cd $DATE\_topo$topology\_traffic$TRAFFIC_MODEL\_rate$ARRIVAL_RATE\_alpha$ALPHA\_$ALPHA_DIV\_seed$SEED_NUMBER

if [ $ONLY_LONG -eq 0 ]
then
    sed -i 's/\#define ONLY_LONG 1/\#define ONLY_LONG 0/g' $CONTIKI/platform/cooja/contiki-conf.h
else
    sed -i 's/\#define ONLY_LONG 0/\#define ONLY_LONG 1/g' $CONTIKI/platform/cooja/contiki-conf.h
    LSA_ENHANCED=0
fi 


../param.sh $LONG_WEIGHT $ALPHA $STROBE_CNT $LSA_R $TRAFFIC_MODEL $PERIOD $ARRIVAL_RATE $PARENT_REDUCTION $REDUCTION_RATIO $DATA_ACK $LSA_MAC $ALPHA_DIV $CHECK $LSA_ENHANCED $ROUTING_NO_ENERGY

IN_DIR=lr\_weight$LONG_WEIGHT\_LR_range$LR_range\_L$ONLY_LONG\_check$CHECK\_strobe$STROBE_CNT\_lsa_en$LSA_ENHANCED\_rou$ROUTING_NO_ENERGY\_LT$LT_PERCENT
if [ ! -e $IN_DIR ]
then
    mkdir $IN_DIR
#    mkdir lr\_weight$LONG_WEIGHT\_LR_range$LR_range\_L$ONLY_LONG\_check$CHECK\_strobe$STROBE_CNT\_lsa$LSA_R\_lsa_mac$LSA_MAC\_lsa_en$LSA_ENHANCED\_rou$ROUTING_NO_ENERGY
fi
cd $IN_DIR
#cd lr\_weight$LONG_WEIGHT\_LR_range$LR_range\_L$ONLY_LONG\_check$CHECK\_strobe$STROBE_CNT\_lsa$LSA_R\_lsa_mac$LSA_MAC\_lsa_en$LSA_ENHANCED\_rou$ROUTING_NO_ENERGY
echo "#########################  We are in $PWD  ########################"

HERE=$PWD
cd $CONTIKI/lanada
make clean TARGET=cooja
cd $HERE

if [ $MRM -eq 0 ]
then
	if [ ! -e COOJA.testlog ]
	then
	#	cd $CONTIKI/tools/cooja
			 java -mx512m -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=$CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\.csc -contiki="$CONTIKI"
	#		ant run_nogui -Dargs=/home/user/Desktop/Double-MAC/lanada/sim_scripts/scripts/0729_36grid_2X.csc
	#	cd $HERE
	fi
else
	if [ ! -e COOJA.testlog ]
	then
		cd $CONTIKI/tools/cooja_mrm$MRM
		ant run_nogui -Dargs=$CONTIKI/lanada/sim_scripts/scripts/$topology\_$LR_range\.csc	
		mv build/COOJA.testlog $HERE
		cd $HERE
	fi
fi

if [ ! -e report_summary.txt ]
then
    ../../pp.sh
fi
cd ../..

echo "Simulation finished"

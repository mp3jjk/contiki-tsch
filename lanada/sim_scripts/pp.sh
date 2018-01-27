#!/bin/bash

# programmed by Joonki Hong

node_count=2
MAX_NODE_NUMBER=1
DELAY=0
ENERGY=0
PRR=0
COLLISION=0


if [ -e parsing ]
then
 rm -rf parsing
fi

mkdir parsing

echo "Parsing the raw data"


while [ $node_count -le $MAX_NODE_NUMBER ]
do

	# Sink node receive message 
	if [ -n "`cat COOJA.testlog | grep "0$node_count\X"`" ]
	then
		#echo "from $node_count"
		cat COOJA.testlog | grep "0$node_count\X" > parsing/from$node_count.txt
	fi

	# Source node send message
	if [ -n "`cat COOJA.testlog | grep ":$node_count:app"`" ]
	then
		#echo "app send $node_count"
		cat COOJA.testlog | grep ":$node_count:app" > parsing/node_send$node_count.txt
	fi

	# Periodic status review message for each node
	if [ -n "`cat COOJA.testlog | grep ":$node_count:\[PS\]"`" ]
	then
		#echo "PS $node_count"
		cat COOJA.testlog | grep ":$node_count:\[PS\]" > parsing/PS$node_count.txt
	fi
	
	# Tx, Rx statistic for each node
	if [ -n "`cat COOJA.testlog | grep "Contiki_$node_count "`" ]
	then
		#echo "Stat $node_count"
		cat COOJA.testlog | grep "Contiki_$node_count " > parsing/Stat$node_count.txt
	fi

	let node_count=$node_count+1
done

# Lifetime end node status review
if [ -n "`cat COOJA.testlog | grep "\[LT\]"`" ]
then
	#echo "LT"
	cat COOJA.testlog | grep "\[LT\]" > parsing/LT.txt
fi


if [ -n "`cat COOJA.testlog | grep "AVG"`" ]
then
	#echo "AVG"
	cat COOJA.testlog | grep "AVG" > parsing/AVG.txt
fi

########################################################################################
echo "Processing the parsed data"

if [ $DELAY == "1" ]
then
	if [ -e delay ]
	then
	 rm -rf delay
	fi
	mkdir delay
fi

if [ $ENERGY == "1" ]
then
	if [ -e energy ]
	then
	 rm -rf energy
	fi
	mkdir energy
fi

if [ $PRR == "1" ]
then
	if [ -e PRR ]
	then
	 rm -rf PRR
	fi
	mkdir PRR
fi

if [ $COLLISION == "1" ]
then
	if [ -e collision ]
	then
	 rm -rf collision
	fi
	mkdir collision
fi

node_count=2
while [ $node_count -le $MAX_NODE_NUMBER ]
do

	if [ ! -e parsing/node_send$node_count.txt ]
	then
		break
	fi

	echo "Working on node number: $node_count"
######################### Delay ######################
	if [ $DELAY == "1" ]
	then
		if [ -e delay/packet_delay$node_count.txt ]
		then
			rm delay/packet_delay$node_count.txt
		fi
		
		tot_delay=0
		count_delay=0

		while read line
		do
			time=`echo "$line" | cut -d':' -f1`
			part1=`echo "$line" | cut -d' ' -f3`
			part2=`echo "$part1" | cut -d':' -f2`
			#echo $time $part2
			recv=`cat parsing/from$node_count.txt | grep "id:$part2 "`
			#echo recv "$recv"
			if [ -n "$recv" ]
			then
				time_recv=`echo "$recv" | cut -d':' -f1`
				#echo time_recv $time_recv
				#echo time $time
				let "delay=$time_recv-$time"
				echo packet id $part2 delay: $delay >> delay/packet_delay$node_count.txt
				let "tot_delay=$tot_delay+$delay"
				let "count_delay=$count_delay+1"
			else
				echo packet id $part2 loss!!! >> delay/packet_delay$node_count.txt
			fi

		done < parsing/node_send$node_count.txt
		
		avg_delay=`echo "$tot_delay / $count_delay"|bc`
		echo "<AVG delay: $avg_delay us>" >> delay/avg_packet_delay.txt
	fi

######################### Energy consumption #####################
	if [ $ENERGY == "1" ]
	then
		if [ -e energy/node_energy$node_count.txt ]
		then
			rm energy/node_energy$node_count.txt
		fi

		energy=0
		count=1
		while read DISSIPATION
		do
			if [ -n "`echo "$DISSIPATION" | grep DISSIPATION_RATE`" ]
			then
				break
			fi
		done < COOJA.testlog

		DISSIPATION=`echo "$DISSIPATION" | cut -d':' -f4`
		#echo $DISSIPATION

		while read line
		do
			part1=`echo "$line" | cut -d' ' -f2`
			part2=`echo "$line" | cut -d' ' -f3`
			part3=`echo "$line" | cut -d' ' -f5`
			
			case "$part1" in
				"ON" ) 	multi=`echo "$DISSIPATION" | cut -d',' -f1`;;

				"TX" )  multi=`echo "$DISSIPATION" | cut -d',' -f2`;;

				"RX" ) 	multi=`echo "$DISSIPATION" | cut -d',' -f3`;;

				"ONL" ) multi=`echo "$DISSIPATION" | cut -d',' -f4`;;

				"TXL" ) multi=`echo "$DISSIPATION" | cut -d',' -f5`;;

				"RXL" ) multi=`echo "$DISSIPATION" | cut -d',' -f6`;;

				* )  multi=-1 ;;
			esac

			if [ "$part3" == "100.00" ]
			then
				multi=0;
			fi
			
			#echo $multi 
			if [ "$multi" -ne -1 ]
			then
				increment=`echo "$part2*$multi"|bc`
				#echo "$increment"
				energy=`echo "$energy+$increment"|bc`
			fi
			
			echo -n "$part1: $part3% | " >> energy/node_energy$node_count.txt

			if [ "$part1" == "INTL" ]
			then
				let "count=$count+1"
				echo "Energy: $energy" >> energy/node_energy$node_count.txt
				energy=0
			fi

		done < parsing/Stat$node_count.txt
	fi
###################### PRR ######################

	if [ $PRR == "1" ]
	then
		#if [ -e PRR/PRR.txt ]
		#then
		#	rm PRR/PRR.txt
		#fi

		recv=0
		send=0
		while read line
		do
			let "recv=$recv+1"
		done < parsing/from$node_count.txt

		while read line
		do
			let "send=$send+1"
		done < parsing/node_send$node_count.txt

		PRR_value=`echo "scale=3;$recv/$send*100"|bc`
		echo "<PRR: $PRR_value %>" >> PRR/PRR.txt
	fi

#################### Node collision ratio ##########################
	if [ $COLLISION == "1" ]
	then
		#if [ -e collision_ratio.txt ]
		#then
		#	rm collision_ratio.txt
		#fi

		while read line
		do
			if [ -n "`echo "$line" | grep CSMA_Transmission`" ]
			then
				part1=`echo "$line" | cut -d',' -f1`
				part2=`echo "$part1" | cut -d':' -f4`
				part3=`echo "$line" | cut -d' ' -f7`
				part4=`echo "$line" | cut -d',' -f2`
				part5=`echo "$part4" | cut -d':' -f2`
			fi
		done < parsing/PS$node_count.txt
		#echo $part2 $part3 $part5

		transmission_overhead=`echo "scale=3;$part5/$part2*100"|bc`
		echo "<Transmission overhead: $transmission_overhead %> <Collision: $part3 >" >> collision/collision_ratio.txt
		
	fi

	let "node_count=$node_count+1"
done

##########################################################################################

# Report summary
echo 
if [ -e report_summary.txt ]
then
	rm report_summary.txt
fi

if [ -e temp.txt ]
then
	rm temp.txt
fi

node_count=2
while [ $node_count -le $MAX_NODE_NUMBER ]
do

	if [ ! -e parsing/node_send$node_count.txt ]
	then
		break
	fi
	
	echo "NODE$node_count" >> temp.txt

	final_energy=`tail -1 energy/node_energy$node_count.txt | cut -d'|' -f10`
	echo "<$final_energy>" >> energy/final_energy.txt

	let "node_count=$node_count+1"

done

max_energy=0
while read line
do
	part1=`echo $line | cut -d' ' -f3`
	part2=`echo $part1 | cut -d'>' -f1`
	#echo $part2
	if [ $part2 -gt $max_energy ]
	then
		max_energy=$part2
	fi
done < energy/final_energy.txt

while read line
do
	part1=`echo $line | cut -d' ' -f3`
	part2=`echo $part1 | cut -d'>' -f1`
	percent=`echo "scale=3;$part2/$max_energy*100"|bc`
	#echo $percent

	echo "<Energy $percent %>" >> energy/percent_energy.txt
done < energy/final_energy.txt


paste temp.txt delay/avg_packet_delay.txt energy/percent_energy.txt PRR/PRR.txt  > report_summary.txt
# collision/collision_ratio.txt

#!/bin/bash

for DIR in $1*
do
	if [ -d $DIR ]
	then
		traffic=`echo $DIR | cut -d'_' -f3`
		echo "############# $DIR ###############"
		cd $DIR
		for dir in *
		do
			cd $dir
			tot_delay=0
			node_count=0
			while read line
			do
				delay=`echo "$line" | cut -d' ' -f3`
				let "tot_delay=$tot_delay + $delay"
				let "node_count=$node_count + 1"
			done < delay/avg_packet_delay.txt
			let "avg_delay=$tot_delay / $node_count /1000"
			echo $traffic $dir : $avg_delay ms
			cd ..
		done
		echo 
	fi
	cd ..
done



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
			tot_prr=0
			node_count=0
			while read line
			do
				prr=`echo "$line" | cut -d' ' -f2`
				tot_prr=`echo "scale=3;$tot_prr+$prr"|bc`
				let "node_count=$node_count + 1"
			done < PRR/PRR.txt
			avg_prr=`echo "scale=3;$tot_prr / $node_count"|bc`
			echo $traffic $dir : $avg_prr %
			cd ..
		done
		echo 
	fi
	cd ..
done



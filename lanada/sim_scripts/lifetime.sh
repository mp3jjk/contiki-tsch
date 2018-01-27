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
			part1=`grep -r ":Lifetime"`
			lifetime=`echo "$part1" | cut -d':' -f2`
			if [ -n "$lifetime" ]
			then
				let "lifetime=${lifetime} / 1000000"
			fi
			node=`echo "$part1" | cut -d':' -f3`
			echo $dir : node$node $lifetime
			cd ..
		done
		echo 
	fi
	cd ..
done



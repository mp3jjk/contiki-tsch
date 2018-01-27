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

			node=`echo "$part1" | cut -d':' -f3`
			echo $dir : node$node
			if [ -n "$lifetime" ]
			then
			for temp in ${lifetime}
				do	
					let "temp = $temp /1000000"
					echo $temp
				done
			fi
			cd ..
		done

		echo 
	fi
	cd ..
done



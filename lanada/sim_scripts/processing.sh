for DIR in *
do
	if [ -d $DIR -a -n "`echo "$DIR" | grep traffic0`" ]
	then
		echo "------------------- Entering to DIR $DIR ----------------------"
		cd $DIR

		for dir in *
		do
			if [ -d $dir ]
			then
				echo "@@@@@@@@@@@@@@@@@ Entering to dir $dir @@@@@@@@@@@@@@@@@@@@@"
				cd $dir
				#if [ ! -e report_summary.txt ]
				#then
					../../pp.sh
				#fi
				cd ..
			fi
		done

	cd ..
	fi
done


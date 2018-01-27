#!/bin/bash 

MODE=$1
ARG=$2

if [ $MODE == "lifetime" ]
then
	./lifetime.sh $ARG > temp.txt
elif [ $MODE == "delay" ]
then
	./delay.sh $ARG > temp.txt
elif [ $MODE == "prr" ]
then
	./prr.sh $ARG > temp.txt
fi

# Parse category information to temp3.txt file 
grep "#" temp.txt | awk '{print $2}' > temp2.txt
awk -F 'seed' '{print $1}' temp2.txt | awk '!x[$0]++' > temp3.txt 
# sed 's/_//g' temp.txt | sed '/#/d' > temp4.txt

# Loop with category
while read category
do
	# echo Loop for category $category
	count=-1
	cat_count=0
	max_count=0
	while read line 
	do
		# Check the start of category
		if [[ $line =~ "$category" ]]
		then
			count=0
			let "cat_count=$cat_count+1"
			continue
		# Check the end of category
		elif [ -z "$line" ]
		then
			# echo
			if [ $count -gt $max_count ]
			then
				max_count=$count
			fi
			count=-1
			continue
		fi

		# Insdie the category 
		if [ $count -ge 0 ]
		then
			# Save value in add
			add=`echo $line | cut -d ' ' -f4`
			# echo Add: $add

			# Save the value name
			if [ $MODE == "lifetime" ]
			then
				base=`echo $line | cut -d ' ' -f1`
			else
				base=`echo $line | cut -d ' ' -f2`
			fi
			y[$count]=$base

			# Sum values 
			if [ $MODE == "prr" ]
			then
				if [ -z ${x[$count]} ]
				then
					x[$count]=0
				fi
				x[$count]=`echo "scale=3;${x[$count]}+$add"|bc`
			else
				let " x[$count] = ${x[$count]}+$add " 
			fi
			# echo Sum value: ${x[$count]}

			# echo "$count-th variable is ${x[$count]}"
			let "count=$count+1"
		fi
	done < temp.txt
	count=0
	echo "################ $category ################"
	# echo max_count: $max_count
	while [ $count -lt $max_count ]
	do
		# Taking the average
		# echo Total sum value: ${x[$count]}
		if [ $MODE == "prr" ]
		then
			x[$count]=`echo "scale=3;${x[$count]}/$cat_count"|bc`
		else
			let "x[$count]=${x[$count]}/$cat_count"
		fi
		echo "${y[$count]} average: ${x[$count]}"
		x[$count]=0
		let "count=$count+1"
	done
	echo
done < temp3.txt

# rm temp*.txt

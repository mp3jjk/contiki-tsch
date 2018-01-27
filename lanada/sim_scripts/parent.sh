#!/bin/bash

MAX_NODE_NUMBER=50
node_count=2
while [ $node_count -le $MAX_NODE_NUMBER ]
do
	if [ ! -e parsing/node_send$node_count.txt ]
	then
		break
	fi

	parent=`tail -1 "parsing/PS$node_count.txt" | cut -d':' -f4`
	
	echo "$node_count : $parent"
	let "node_count = $node_count + 1"
done

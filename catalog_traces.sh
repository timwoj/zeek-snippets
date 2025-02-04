#!/usr/bin/env bash

for i in $(find ${TRACES} -type f | sort); do
    ts_output=$(tshark -T fields -e frame.protocols -Qnr $i 2>/dev/null | sort | uniq )
    if [ ${#ts_output} != 0 ]; then
	echo $i
	echo "==================="
	printf "%s\n" "${ts_output}"
	echo
    fi
done

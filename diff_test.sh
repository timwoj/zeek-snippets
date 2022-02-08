#!/bin/sh

ZEEK_PATH="/Users/tim/Desktop/projects/zeek-master"
ZEEK_HOME="${ZEEK_PATH}/testing/btest"
TEST_NAME=$1
REMOVE_TS="${ZEEK_PATH}/testing/Scripts/diff-remove-timestamps"
REMOVE_UID="${ZEEK_PATH}/testing/Scripts/diff-remove-uids"

for f in `ls -1 ${ZEEK_HOME}/.tmp/${TEST_NAME}`; do

    if [ ! -f ${ZEEK_HOME}/Baseline/${TEST_NAME}/${f} ]; then
	continue
    fi
    
#    cat ${ZEEK_HOME}/Baseline/${TEST_NAME}/${f} | sort | ${REMOVE_TS} > /Users/tim/Desktop/old
#    cat ${ZEEK_HOME}/.tmp/${TEST_NAME}/${f} | sort | ${REMOVE_TS} > /Users/tim/Desktop/new

    cat ${ZEEK_HOME}/Baseline/${TEST_NAME}/${f} | sort | ${REMOVE_TS} | ${REMOVE_UID} > /Users/tim/Desktop/old
    cat ${ZEEK_HOME}/.tmp/${TEST_NAME}/${f} | sort | ${REMOVE_TS} | ${REMOVE_UID} > /Users/tim/Desktop/new
    
    echo "#############################"
    echo $f
    echo "#############################"
    diff -b /Users/tim/Desktop/old /Users/tim/Desktop/new

done

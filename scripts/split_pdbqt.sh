#!/bin/bash

lnum=`grep -n MODEL $1 | head -n 2 | tail -n 1 | awk -F ":" '{print $1}'` 
lnum=$(($lnum-1))
#echo $lnum

split -l $lnum  -d $1 ${1}_

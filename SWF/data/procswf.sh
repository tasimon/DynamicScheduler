#!/bin/bash
# note we give a 1 waittime and queue priority
awk '$1!=";" {print $2,1,$4,$5,1}' $1 > $1.input

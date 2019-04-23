#!/bin/bash

ME= 'jpf'

IPCS_S=`ipcs -s |grep $ME | awk '{print $2}'`
IPCS_M=`ipcs -m |grep $ME | awk '{print $2}'`
IPCS_Q=`ipcs -q |grep $ME | awk '{print $2}'`

for id in $IPCS_M; do
  ipcrm -m $id;
done

for id in $IPCS_S; do
  ipcrm -s $id;
done

for id in $IPCS_Q; do
  ipcrm -q $id;
done

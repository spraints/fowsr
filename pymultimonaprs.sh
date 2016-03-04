#!/bin/sh

# Delete the log and dat file to perform complete read out, then call fowsr

wsr="/usr/bin/fowsr -fm"
dat="/var/log/fowsr/fowsr.dat"
LOG="/var/log/fowsr/pymultimonaprs.log"

rm -f $LOG

$wsr

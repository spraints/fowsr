#!/bin/sh
#/ Usage: ./fake-fowsr.sh -c
#/ Behaves similarly to the real fowsr, but with unreliable data.

set -e
sleep 98

echo 'usb_set_debug: Setting debugging level to 1 (on)' >/dev/stderr

dtime=`date '+DTime %d-%m-%Y %H:%M:00'`
etime=`date '+ETime %s'`
cat <<EOF
${dtime}
${etime}
RHi 54.0
Ti 16.3
RHo 71.0
To -0.7
RP 1004.3
WS 0.0
WG 0.0
DIR 270.0
Rtot 0.3
state 00

EOF

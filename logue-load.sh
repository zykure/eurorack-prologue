#!/bin/sh
set -e

if [ "x$1" == "x" ]; then
    echo "Usage: $0 -s <SLOT> -u <FILE> [...]"
    echo "For other options, check output of './logue-cli --help'".
    echo
fi

echo "Load with params: $@";

temp_file=$(mktemp)

in_port=$(./logue-cli probe -l | grep 'minilogue xd' | grep in | awk '{print $2}' | sed 's/.$//' | tail -n 1)
out_port=$(./logue-cli probe -l | grep 'minilogue xd' | grep out | awk '{print $2}' | sed 's/.$//' | tail -n 1)
amidi_port=$(amidi -l | grep 'minilogue xd' | awk '{print $2}' | tail -n 1)

echo "Device input: $in_port / output: $out_port (amidi port = $amidi_port)"

./logue-cli load -d -i $in_port -o $out_port $@  > $temp_file
echo "Created file: $temp_file"

tail -n1 $temp_file| sed 's/,//g' | sed 's/}//g' | sed 's/{//g' | sed 's/>//g' | sed 's/^ *//g' > "$temp_file.sysex"
echo "Created file: $temp_file.sysex"

amidi -p $amidi_port -S `cat $temp_file.sysex`
echo "MIDI transfer successful"


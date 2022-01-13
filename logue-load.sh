#!/bin/sh

echo "Load with params: $@";

temp_file=$(mktemp)

echo "Tmp file $temp_file"

in_port=$(./logue-cli probe -l | grep 'minilogue xd' | grep in | awk '{print $2}' | sed 's/.$//' | tail -n 1)
out_port=$(./logue-cli probe -l | grep 'minilogue xd' | grep out | awk '{print $2}' | sed 's/.$//' | tail -n 1)
amidi_port=$(amidi -l | grep 'minilogue xd' | awk '{print $2}' | tail -n 1)

./logue-cli load -d -i $in_port -o $out_port -u $@  > $temp_file
tail -n1 $temp_file| sed 's/,//g' | sed 's/}//g' | sed 's/{//g' | sed 's/>//g' | sed 's/^ *//g' > "$temp_file.sysex"
amidi -p $amidi_port -S `cat $temp_file.sysex` && echo "MIDI transfer successful"

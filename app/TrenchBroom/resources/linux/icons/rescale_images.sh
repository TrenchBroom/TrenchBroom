#!/bin/bash
ORIGINAL="tb_icon_orig.png"
EXT="${ORIGINAL##*.}"

for SIZE in 512 256 192 128 96 72 64 48 42 36 32 24 22 16 8;
do
    convert "$ORIGINAL" -resize "$SIZEx$SIZE" icon_"$SIZE.$EXT"
done

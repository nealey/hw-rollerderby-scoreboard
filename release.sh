#! /bin/sh

now=$(date +%Y%m%d)

make all
mkdir -p bin
for i in *.hex; do
	cp $i bin/scoreboard-$now-${i#scoreboard-}
done

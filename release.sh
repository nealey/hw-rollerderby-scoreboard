#! /bin/sh -e

version=$(git describe --tags --exact-match)

make all
mkdir -p bin
for i in *.hex; do
	cp $i bin/scoreboard-$version-${i#scoreboard-}
done

#!/bin/bash
#
# cd ./build
# bash ../test.sh

make

EXECPATH="./bin"
SOURCEPATH="../image/Source"
OUTPUTPATH="../image/Result"
EXPECTEDPATH="../image/ExpectedResult"

for fullPath in $SOURCEPATH/*.{jpg,ppm}; do
	fileName=$(basename "$fullPath")
	fileNameNoExt="${fileName%.*}"
	ext="${fileName##*.}"
	echo "$fullPath $OUTPUTPATH/$fileNameNoExt.jpg"
	timeout 3s "$EXECPATH/combinaison" "$fullPath" "$OUTPUTPATH/$fileNameNoExt.jpg"
	if [ $ext = "ppm" ]
	then
		resExt="pgm"
	else
		resExt="bmp"
	fi
	"$EXECPATH/evaluation" "$OUTPUTPATH/$fileNameNoExt.jpg" "$EXPECTEDPATH/$fileNameNoExt.$resExt"
done

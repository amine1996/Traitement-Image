#!/bin/bash
#
# cd ./build
# bash ../test.sh

make

EXECPATH="./bin"
SOURCEPATH="../image/Source"
OUTPUTPATH="../image/Result"
EXPECTEDPATH="../image/ExpectedResult"

for fullpath in $SOURCEPATH/*.{jpg,ppm}; do
	filename=$(basename "$fullpath")
	filenamenoext="${filename%.*}"
	ext="${filename##*.}"
	echo "$fullpath $OUTPUTPATH/$filenamenoext.jpg"
	timeout 3s "$EXECPATH/combinaison" "$fullpath" "$OUTPUTPATH/$filenamenoext.jpg"
	if [ $ext = "ppm" ]
	then
		resExt="pgm"
	else
		resExt="bmp"
	fi
	"$EXECPATH/evaluation" "$OUTPUTPATH/$filenamenoext.jpg" "$EXPECTEDPATH/$filenamenoext.$resExt"
done

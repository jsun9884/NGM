#!/bin/bash

FILES=
CKSUM_FILES=
UNPACK_DIR=unpacked
INSTALL_DIR=/root/upgrade

function unpack() {
	tar -xf ../$1
	if [ $? -ne 0 ]
	then
		return -2
	fi
	return 1
}

function checkCrc() {
	local S1=`cat $1 | cksum`
	local S2=`cat $1.cksum`
	if [ "$S1"="$S2" ];
	then
		return 1
	fi
	return 0
}

function checkFiles() {
	CKSUM_FILES=`ls *.cksum`
	local FILE=
	for f in ${CKSUM_FILES}
	do
		FILE="${f%%.cksum} "
		$(checkCrc $FILE)
		if [ $? -ne 1 ];
		then
			echo $FILE -- failed
			return 0
		else
			echo $FILE -- pass
		fi
	done
	return 1
}

rm -Rf ${UNPACK_DIR}
mkdir ${UNPACK_DIR}
cd ${UNPACK_DIR}

echo "Unpacking..."
unpack $1
if [ $? -ne 1 ];
then
	echo "Unpack upgrade failed..."
	exit -2;
fi

echo "Checking cksums..."
checkFiles
if [ $? -ne 1 ];
then
	echo "Check files failed..."
	exit -1
fi

rm -Rf ${INSTALL_DIR}
cp -Rf ../${UNPACK_DIR} ${INSTALL_DIR}
echo "Success!"
exit 0

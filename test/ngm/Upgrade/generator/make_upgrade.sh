#!/bin/bash

source ./version.sh

FILES="ngm \
	Configuration.xml \
	imx6q-var-dart.dtb \
	ifmetric \
	logrotate \
	logrotate.conf \
	rsyslog \
	WebUI.tar.gz \
	download.sh \
	upgrade.sh"

PKG_FILES=
CKSUM_FILES=

#VERSION=0.9.3
UPGRADE_FILE=oce-upgrade-$VERSION.tar.gz
UPGRADE_BIN=oce-upgrade-$VERSION.bin

function generateCksum() {
        cat $1 | cksum > $1.cksum
}

function generateCksumFiles() {
	for f in ${FILES}
	do
		generateCksum ${f}
		PKG_FILES+="${f} "
		CKSUM_FILES+="${f}.cksum "
	done
}

function makeTarball() {
	tar -czf ../$UPGRADE_FILE $PKG_FILES $CKSUM_FILES
}

function checkCrc() {
	local S1=`cat $1 | cksum`
	local S2=`cat $1.cksum`
	if [ $S1=$S2 ];
	then
		return 1;
	fi
	return 0;
}

cd files
generateCksumFiles
makeTarball
#mv -f ../${UPGRADE_FILE} ../${UPGRADE_BIN}


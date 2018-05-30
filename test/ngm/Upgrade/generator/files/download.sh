#!/bin/bash

UPGRADE_PKG=upgrade.tar.gz
wget -t 1 -T 40 "$1" -O ${UPGRADE_PKG}
./unpack_upgrade.sh ${UPGRADE_PKG}
exit $?


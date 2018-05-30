#!/bin/bash

UPGRADE_CFG=Configuration.xml.new
wget "$1" -O ${UPGRADE_CFG}
exit $?


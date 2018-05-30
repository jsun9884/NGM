#!/bin/bash

VERSION=10.05

echo "export VERSION=${VERSION}" > generator/version.sh

echo "#pragma once" > ../Version.h
echo "#define FW_VERSION \"${VERSION}\"" >> ../Version.h


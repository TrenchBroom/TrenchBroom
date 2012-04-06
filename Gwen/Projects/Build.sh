#!/bin/bash

UNAME=`uname`

if [ "$UNAME" == "Darwin" ]; then
 
	PREMAKE="premake4-osx"
	OSFOLDER="macosx"
fi
	
if [ "$UNAME" == "Linux" ]; then

	PREMAKE="premake4"
	OSFOLDER="linux"
fi

chmod 777 ./${PREMAKE}

./${PREMAKE} clean
./${PREMAKE} gmake

if [ "$UNAME" == "Darwin" ]; then
./${PREMAKE} xcode3
./${PREMAKE} xcode4
fi

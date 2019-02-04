#!/bin/bash 
set -ev 

mkdir build
CFLAGS=-Wno-missing-braces meson . build
ninja -C build/ \
	test \
	aaf-talker \
	aaf-listener \
	crf-talker \
	crf-listener \
	cvf-talker \
	cvf-listener \
	ieciidc-talker

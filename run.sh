#!/bin/bash
size=(8 16 32 64 128 256 512 1024 2048 4096 8196 16384)
for s in "${size[@]}"
	do
			./green_benchmark $s;
	done
for s in "${size[@]}"
	do
			./pthread_benchmark $s;
	done
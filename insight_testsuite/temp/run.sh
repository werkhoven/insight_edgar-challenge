#!/bin/bash

gcc -o 	./src/parselog 		./src/parselog.c 	\
	./src/match_ip.c 	./src/trim_sessions.c 	\
	./src/update_time.c 	./src/parse_line.c	\
	
	
./src/parselog 	./input/log.csv 			\
		./input/inactivity_period.txt 		\
		./output/sessionization.txt

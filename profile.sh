#!/bin/bash

gcc -o ./src/parselog ./src/parselog.c
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./src/parselog ./input/log.csv ./input/inactivity_period.txt ./output/sessionization.txt

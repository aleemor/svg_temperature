#!/bin/bash
clear
gcc -o svg_temp svg_temp.c
./svg_temp
open my_rect.svg

#!/bin/bash

g++ -o Pong Pong.cpp -lSDL2 -lSDL2_ttf -lSDL2_image

read -n1 -r -p "Press any key to continue..." key
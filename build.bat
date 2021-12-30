@echo off

g++ -o Pong.exe Pong.cpp -O3 -std=c++11 "pong icon.res" -mwindows -lSDL2 -lSDL2_main -lSDL2_image -lSDL2_ttf
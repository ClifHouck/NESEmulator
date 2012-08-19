#!/bin/bash

g++ -lglut $(sdl-config --cflags --libs) -std=c++0x NesApp.cpp Cpu65XX.cpp main.cpp PPU.cpp -o nesemu

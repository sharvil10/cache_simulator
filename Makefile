CC=g++
CFLAGS=-I.

sim_cache: cache.cpp trace.cpp simulator.cpp main.cpp 
	$(CC) -o sim_cache *.cpp *.hpp

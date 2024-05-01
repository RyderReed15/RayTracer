CC=g++
CFLAGS= -O3 -march=native -I /share/eigen-3.4.0/ -I /usr/include/eigen3/ -I /usr/local/include/eigen3
DEPS = io.h 

main: main.cpp
	$(CC) $(CFLAGS) -o main main.cpp io.cpp 

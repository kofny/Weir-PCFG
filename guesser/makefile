CC=g++
XFLAG =-Wall -ansi -pedantic
CFLAG = -O3 -std=c++11 -no-pie
all: pcfg_manager


pcfg_manager: guesser/pcfg_manager.cpp
		$(CC) $(XFLAG) $(CFLAG) $< -o $@


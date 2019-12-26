CC=g++
XFLAG =-Wall -ansi -pedantic
CFLAG = -O3 -no-pie
main: pcfg_manager checkPasswords clean-o


pcfg_manager: pcfg_manager.o 
	$(CC) $(CFLAG) pcfg_manager.o -o pcfg_manager
	
checkPasswords: checkPasswords.o
	$(CC) $(CFLAG) checkPasswords.o -o checkPasswords

pcfg_manager.o: pcfg_manager.cpp
	$(CC) $(CFLAG) -c pcfg_manager.cpp

checkPasswords.o: checkPasswords.c
	$(CC) $(CFLAG) -c checkPasswords.c

.PHONY: clean clean-o
clean:
	rm -f pcfg_manager 
	rm -f checkPasswords

clean-o:
	rm -f *.o

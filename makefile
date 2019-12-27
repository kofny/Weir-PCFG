include guesser/makefile
include trainer/makefile

.PHONY: clean
clean:
	rm -f pcfg_manager
	rm -f train.py
	rm -f *.o
	rm -f *.a

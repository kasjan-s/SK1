TARGET: ileczekam czekamnaudp

CC	= cc
CFLAGS	= -Wall -O2
LFLAGS	= -Wall


ileczekam: ileczekam.o err.o
	$(CC) $(LFLAGS) $^ -o $@

czekamnaudp: czekamnaudp.o err.o
	$(CC) $(LFLAGS) $^ -o $@

.PHONY: clean TARGET
clean:
	rm -f ileczekam czekamnaudp *.o *~ *.bak

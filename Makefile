CC = c99
CFLAGS = -O2

OBJS = advent350.o advtext.o miscdata.o motion.o travel.o util.o verbs.o vocab.o

all : advent350 frawd

advent350 : $(OBJS)
	$(CC) -o advent350 $(OBJS)

advent350.o util.o verbs.o : advconfig.h advconst.h advdecl.h
motion.o : advconst.h advdecl.h
travel.o vocab.o : advconst.h
advtext.o : advconfig.h

frawd : frawd.c
	$(CC) $(CFLAGS) -o frawd frawd.c

clean :
	rm -f advent350 frawd $(OBJS)

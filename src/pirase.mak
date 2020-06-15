# Make file for RPI version of rase library

FILES = time.o sched.o message.o timer.o netfnip.o parse.o log.o rase_setjmp.o
CFLAGS = -c
AFLAGS = -cvr
pirase.a: $(FILES)   

.c.o:
	gcc $(CFLAGS) $*.c
	ar $(AFLAGS) pirase.a $*.o

.s.o:
	as  $*.s -o $*.o
	ar $(AFLAGS) pirase.a $*.o

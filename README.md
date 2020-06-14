# pirase
Port of RASE library to Rasberry Pi

This library contains functions used to create real-time systems on DOS and WIN32.

Currently for the RPI these are:

sched.c:      C non-premptive multi threading within an executable. Very light, very fast

message.c:    Synchronised message passing between threads.

time.c:       Functions for handling time as a float

timer.c:      Support for polled timers, as many as you like.

parse.c:      String parsers principally for using .ini files

netfnip:      Support for UDP servers, UDP clients and transactions between them

rase_setjmp.a: My own setjmp, longjmp function as linux chose to mangle theirs. Faster anyway

rase32.h:     Header file for the above functions

pirase.mak:   Makes the library

test.c:       Quick demo of multi-threading

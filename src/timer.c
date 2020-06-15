/* Windows95 and NT timer routine */
/* Uses timeSetEvent in winmm.dll for basic timing */

// resolution 1 mSec

#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "rase32.h"


#define DOSTIC 55  // Equivalent to DOS tic 18.2 per sec (nearly)

// Read the CLOCK_MONOTONIC
unsigned long processtics(void)
{  unsigned long pi_tics ;

	struct timespec time_now;
   clock_gettime(CLOCK_MONOTONIC, &time_now);

   pi_tics = ((time_now.tv_sec & 0x3fffff) * 1000) +
               time_now.tv_nsec / 1000000;
   return pi_tics ;
}


int time_out(unsigned long tics, time_o *var )
{
   unsigned long tp, pi_tics ;

   pi_tics = processtics() ;
   if (tics)
     { var->tick = pi_tics;
       var->time = tics;
     }
   else
     {
      if (pi_tics < var->tick)
        tp = 0xfa00000 - var->tick + pi_tics ;
      else
        tp = pi_tics - var->tick ;

      var->trem = var->time - tp;
      if( tp > var->time )
       return(1);
     }
   return(0) ;
}



unsigned long time_2go( unsigned long tics, time_o *var )
{
   unsigned long tn, tp, pi_tics ;

   pi_tics = processtics() ;
   if (tics)
    {
	   var->tick = pi_tics;
	   var->time = pi_tics;
    }
   else
    {
	   tn = pi_tics;
	   if( tn < var->tick )
	      tp = 0xfa000000 - var->tick + tn ;
	   else
	    tp = tn - var->tick ;

      var->trem = var->time - tp;
      if( tp > var->time )
	    return(0) ;
    }
   return(var->trem);
}


/* wait for tick seconds. resolution is 10mSec */
void wait_delay(long tick )
{
	time_o t;

	time_out(tick * 10, &t );
	while( !time_out( 0, &t ) ) rase_unlock();
}

#ifdef test
void main()
{
  printf("One second\n") ;
  wait_delay(100) ;
  printf("Ok\n") ;
}
#endif

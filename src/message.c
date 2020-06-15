/****************************************************************************/
/*                Task intercomuication (Message) handlers                  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rase32.h"


#define DMIN(x,y)   ((x) < (y) ? (x) : (y))
#define MAXMESS 15

void rase_unlock(void) ;

struct message_buff { void *ptr; unsigned size; };
static struct message_buff message[MAXMESS] = { {NULL,0},{NULL,0},{NULL,0},
                                                {NULL,0},{NULL,0},{NULL,0},
                                                {NULL,0},{NULL,0},{NULL,0},
                                                {NULL,0},{NULL,0},{NULL,0},
                                                {NULL,0},{NULL,0},{NULL,0} };

/* send a message to another task (activity) */
void rase_send( int mp, unsigned size, void *ptr )
{
   if( mp >= MAXMESS )
   {
      plogf("SEND : Maximum number of Message ports exceeded > %d\n",MAXMESS);
      plogf("Port number %d, buffer size %d\n",mp,size);
      exit(1);
   }
   
   /* fill message port slot with pointer to a copy of the data and its size */
   if( (message[mp].ptr = malloc(size)) == NULL )
   {
      plogf("Out of memory for Message SEND buffer\n");
      plogf("Port number %d, buffer size %d\n",mp,size);
      exit(1);
   }
   memcpy(message[mp].ptr,ptr,size);
   message[mp].size = size;

   /* reshedule here until receive picks up the message */
	while( message[mp].ptr != NULL ) rase_unlock();
}

/* receive a message from another task (activity) */
void rase_receive( int mp, unsigned size, void *ptr )
{
   if( mp >= MAXMESS )
   {
      plogf("RECEIVE : Maximum number of Message ports exceeded > %d\n",MAXMESS);
      plogf("Port number %d, buffer size %d\n",mp,size);
      exit(1);
   }

   /* wait untill message arrives at port slot */ 
   while( message[mp].ptr == NULL ) rase_unlock();

   /* copy over message data */ 
   memcpy( ptr, message[mp].ptr, DMIN(size,message[mp].size) );

   /* signal that message has been received and free message slot */
   free(message[mp].ptr);
   message[mp].ptr = NULL;
}

/* Test if a message is waiting to be received at port 'mp' */
int rase_messat( int mp )
{
   if( mp >= MAXMESS )
   {
      plogf("MESSAT : Maximum number of Message ports exceeded > %d\n",MAXMESS);
      plogf("Port number %d\n",mp);
      exit(1);
   }

   if( message[mp].ptr == NULL )
       return(0);                  /* no message return False */
   else
       return(1);                  /* message waiting return True */
}

/****************************************************************************/


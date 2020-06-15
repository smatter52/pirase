/******************* Scheduler **************************************/
/* This version is for Borland C++ and Windows NT
    Uses stack space reserved by recursion
*/

#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include "rase32.h"

#pragma warn -pro
#pragma warn -stu

#define TSTACKSIZE 1024

/* Globals for task() */
rase_jmp_buf envroot ;        /* root environment */
int      tasknum = -1;        /* Number of tasks */

volatile int      taskindx ;      /* GLOBAL Task indexer */
struct   taskdata  *taskdata;      /* pointer to task descriptors */

void output_taskdata(int taskindx ,struct taskdata *tp)
{  
   ptracef("Rase task %d", taskindx) ;
   ptracef("%p %p %p", tp->stk_ptr, tp->stk_top, tp->env[0].j_sp) ;
}

/****************************************************************************/

/* Re-shedule ie call next task in round robin */
/* name change from unlock() to prevent name collision */
void rase_unlock( void )
{
  int i ;

  if( tasknum == -1 ) return; /* no tasks running so return */

  if( *((short *)taskdata[taskindx].stk_top) != 0x55AA )
   {
      plogf("Stack over flow in task %d",taskindx);
      exit(1);
   }

  if (taskdata[taskindx].state != -1)
      taskdata[taskindx].state = 0 ;
  if (!rase_setjmp(taskdata[taskindx].env))
    {
      for (i=1; i <= tasknum; i++)
        { if (++taskindx >= tasknum)
             taskindx = 0 ;
          if (!taskdata[taskindx].state)
            { taskdata[taskindx].state = 1 ;
#ifdef debug
              output_taskdata(taskindx, taskdata+taskindx) ;
#endif
              rase_longjmp(taskdata[taskindx].env, -1) ;
            }
         }
       rase_longjmp(envroot, -1) ; /* terminate task environments */
     }
}

/****************************************************************************/

/* recursivly defines task environments then runs in round robin */

static void task_run(unsigned int flag)
{
  static int kbcnt ;
  static char *stackbase ;

  char stackspace[TSTACKSIZE] ;

  if (taskindx < tasknum)
  {  if (flag == 0)
      {
        kbcnt = (taskdata[taskindx].stacksize >> 12) ;
        flag = 1 ;
        stackbase = &(stackspace[TSTACKSIZE-4]) ;
      }
     if (flag > 0 && kbcnt > 0)
      { kbcnt-- ;
        task_run(1) ;    // Recusive call to reserve more stack ;
      }

	  if (rase_setjmp(taskdata[taskindx].env))
     {                   /* Will arrive here by longjmp */
        (*taskdata[taskindx].tfp)() ;      /* Start task */
        taskdata[taskindx].state = -1 ;    /* terminated */
        rase_unlock() ;
        plogf("Multi-threading error - terminating\n") ;
        exit(1) ;
     }
     else
     {
		  /* Task state to waiting to run */
		  taskdata[taskindx].state = 0 ;
        /* Modify env value for stack */
       taskdata[taskindx].env[0].j_sp  = (unsigned)stackbase - 8 ;
       taskdata[taskindx].stk_ptr = stackbase ;
       taskdata[taskindx].stk_top = &stackspace[0] ;
       stackspace[0] = 0xaa ;
       stackspace[1] = 0x55 ;

		  taskindx++;
        task_run(0) ;  /* Recursive call for next task env */
     }
  }
  else                            /* Run tasks in round robin */
  {
      /* Start the ball rolling with task 1*/
		taskindx = 0 ;
		rase_longjmp(taskdata[taskindx].env, -1) ;
  }
}

/****************************************************************************/

/* Init task descriptors and start tasks */

void rase_task( int count, ... )
{
   va_list arg_ptr;

   if( tasknum != -1 ) return;

   taskdata = calloc( count, sizeof( struct taskdata ) );
   if( taskdata == NULL )
      { plogf("Out of memory creating task stack descriptor\n");
        exit(1) ;
      }

   va_start(arg_ptr,count);
   for( tasknum = 0; tasknum < count; tasknum++ )
   {
       taskdata[tasknum].tfp = (void *) va_arg(arg_ptr,void*);
       taskdata[tasknum].stacksize = (unsigned) va_arg(arg_ptr,unsigned);
       taskdata[tasknum].actno = tasknum;
   }
   va_end(arg_ptr);
   taskindx = 0 ;
   if (!rase_setjmp(envroot)) /* define task environments recursivly */
      task_run(0) ;           /* then run them to extinction */

   free(taskdata) ;      // free the task environment
   tasknum = -1;
}

/* Kill all threads */
void kill_alltasks()
{ if (tasknum != -1)
    rase_longjmp(envroot, -1) ;
}



#ifdef test
int main(void)
{
  extern int case0(), case1(), case2() ;
  rase_task(3, case0, 4 KB, case1, 4 KB, case2, 4 KB) ;
  return 0 ;
}
int case0()
{
   printf("** First thread part 1 **\n" ) ;
   rase_unlock() ;
   printf("** First thread part 2 ** \n") ;
}
int case1()
{
   printf("** Second thread part 1 **\n" ) ;
   rase_unlock() ;
   printf("** Second thread part 2 **\n" ) ;
}
int case2()
{
   printf("** Third thread part 1 **\n" ) ;
   rase_unlock() ;
   printf("** Third thread part 2 **\n" ) ;
}

#endif

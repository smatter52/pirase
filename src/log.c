// Logging funtions for system use
// System critcal eroor messages are logger to err.log
// Trace messages are logged to trace.log which is cleared at startup

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>


static short traceoutf = 2 ;
static short logoutf = 2 ;

static FILE *tfdp = NULL;
static FILE *lfdp = NULL;



/* ******************************************************** */
// Tracing
void closetrace(void)
{ if (tfdp != NULL)
   fclose(tfdp) ;
}

void ptracef(char *szFormat, ...)
{ char szBuffer[256] ;
  va_list args ;
  int num ;

  if (traceoutf == 2 && tfdp == NULL)
    { if ((tfdp = fopen("trace.log","w")) == NULL)
       traceoutf = 1 ; // printf mode
      else
       atexit(closetrace) ;
    }

   va_start(args, szFormat) ;
   num = vsprintf(szBuffer, szFormat, args) ;
   if (*(szBuffer + num) == '\n') *(szBuffer + num) = 0 ;

   switch (traceoutf) {
   case 1:
    printf("%s\n", szBuffer) ;
    break ;
   case 2:
    fprintf(tfdp,"%s\n", szBuffer) ;
    fflush(tfdp) ;
    break ;
   case 3:
   break ;
   }
}

void settracemode(int mode)
{ traceoutf = (short)mode ; }


/* ******************************************************** */
// Logging

void closelog(void)
{ if (lfdp != NULL)
   fclose(lfdp) ;
}

void plogf(char *szFormat, ...)
{ char szBuffer[256] ;
  va_list args ;
  time_t longtime ;
  struct tm *now ;
  int num, num2;

  if (logoutf == 2 && lfdp == NULL)
    { if ((lfdp = fopen("err.log","a")) == NULL)
       logoutf = 1 ; // printf mode
      else
       atexit(closelog) ;
    }

   time(&longtime) ;
   now = localtime(&longtime) ;
   if (now->tm_year >= 100) now->tm_year -= 100 ;
   num = sprintf(szBuffer,"%02d:%02d:%02d %02d/%02d/%02d : ",
                    now->tm_hour, now->tm_min, now->tm_sec,
                    now->tm_mday, now->tm_mon+1,now->tm_year) ;
//   pArguments = (char *) &szFormat + sizeof(szFormat) ;
   va_start(args, szFormat) ;
   num2 = vsprintf(szBuffer + num, szFormat, args) ;
   num += num2 ;
   if (*(szBuffer + --num) == '\n') *(szBuffer + num) = 0 ;

   switch (logoutf) {
   case 1:
    printf("%s\n", szBuffer) ;
    break ;
   case 2:
    fprintf(lfdp,"%s\n", szBuffer) ;
    fflush(lfdp) ;
    break ;
   case 3:
    break ;
   }
}

void setlogmode(int mode)
{ logoutf = (short)mode ; }


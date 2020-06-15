#include <time.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "rase32.h"


/*************************************************************************/
// Calculate seconds since 00:00:00 GMT on the 1st JAN 1970 from CMOS clock
time_t cmtime(time_t *tp)
{ return(time(tp)) ;
}

long dbldate(double *dbldate)
{
	extern char *strspc();
	struct tm *t;
	time_t ltime;
	double d1,d2,d3;

	cmtime(&ltime);
	t = localtime(&ltime);
	d1 = 10000 * ((double)t->tm_year);
	d2 = 100 * ((double)t->tm_mon+1);
	d3 = (double)t->tm_mday;
	*dbldate = d1+d2+d3 ;
	return ltime ;
}

long dbltime(double *dbltime)
{
	struct tm *t;
	time_t ltime;
	double t1,t2,t3;

	cmtime(&ltime);
	t = localtime(&ltime);
	t1 = 10000 * ((double)t->tm_hour);
	t2 = 100 * ((double)t->tm_min);
	t3 = (double)t->tm_sec;
	*dbltime = t1+t2+t3 ;
   return ltime ;
}

long dbldatetime(double *dbldatetime)
{
	struct tm *t;
	time_t ltime;
	double t1,t2,t3,d1,d2,d3;

	cmtime(&ltime);
	t = localtime(&ltime);
	t1 = 10000 * ((double)t->tm_hour);
	t2 = 100 * ((double)t->tm_min);
	t3 = (double)t->tm_sec;
	d1 = 10000 * ((double)t->tm_year);
	d2 = 100 * ((double)t->tm_mon+1);
	d3 = (double)t->tm_mday;
	*dbldatetime = t1+t2+t3+(d1+d2+d3)*1000000 ;
   return ltime ;
}

char *strtime(time_t ltime)
{
	struct tm *t;
	static char tim[9];

   if (ltime == 0)
     cmtime(&ltime);
	t = localtime(&ltime);

	sprintf(tim,"%02d:%02d:%02d",t->tm_hour,t->tm_min,t->tm_sec);
	return(tim);
}

char *strdate(time_t ltime)
{
	struct tm *t;
	static char date[10];

	if (ltime == 0)
	 cmtime(&ltime);
	t = localtime(&ltime);
	t->tm_year %= 100;       /* dont allow year > 99 */

	sprintf(date,"%02d/%02d/%02d",t->tm_mday,t->tm_mon+1,t->tm_year);
	return(date);
}

// Version for full dates
char *strdate2(time_t ltime)
{
	struct tm *t;
	static char date[12];

	if (ltime == 0)
	 cmtime(&ltime);
	t = localtime(&ltime);
	t->tm_year += 1900;

	sprintf(date,"%02d/%02d/%04d",t->tm_mday,t->tm_mon+1,t->tm_year);
	return(date);
}


/* converts the date/time in the form of a double 901005125020 to a string */
char *datim(dt)
double dt;
{  static char fill[16] ;
	char *tim(double) ;
	char *dat(double) ;

	strcpy(fill,tim(dt)) ;
	strcat(fill," ") ;
	strcat(fill,dat(dt)) ;
	return(fill);
}

/* converts the time in the form of a double 901005125020 to a string */
char *tim(dt)
double dt;
{
	static char fill[10] = { "  :  " };
	char *p,str[15];
	int n;
	sprintf(str,"%013.f",dt);
	p = str + strlen(str) - 6 ;
	for(n=0;n<4;n+=3)
	{
     fill[n]	 = *p++;
     fill[n+1] = *p++;
	}
	return(fill);
}

/* converts the date in the form of a double 901005125020 to a string */
char *dat(dt)
double dt;
{
	static char fill[10] = { "  /  /  " };

	char *p,str[15];
	int n;
	sprintf(str,"%012.f",dt);
	p = str + strlen(str) - 12;
	for(n=6;n>=0;n-=3)
	{
     fill[n]	 = *p++;
     fill[n+1] = *p++;
	}
	return(fill);
}

/* convert strings date and time to a double. Flag sets default time */
double dttod(char *time, char *date, int flag)
{
	double atof();
	int n;
	char dts[16],*p,str[20];

	p = dts;
	if (*date == 0)
	  return(0) ;
	else
	  strcpy(str,date) ;
	if (flag)
	 { if (*time == 0)
		 strcat(str,"24:00:00") ;
		else
		 { strcat(str,time) ;
			strcat(str,":59") ;
		 }
	 }
	else
	 { if (*time == 0)
		 strcat(str,"00:00:00") ;
		else
		 { strcat(str,time) ;
			strcat(str,":00") ;
		 }
	 }
	for(n=6;n>=0;n-=3)
	{   if (n == 6)                  // Date 2000 fix
		  if (str[n] >= '0' && str[n] <= '7')
			*p++ = '1' ;
		 *p++ = str[n];
		 *p++ = str[n+1];
	}
	for(n=8;n<15;n+=3)
	{
		 *p++ = str[n];
		 *p++ = str[n+1];
   }
   *p = 0 ;
   return(atof(dts));
}

// Time stamp the process to trace file
void stampnow(int index)
{ time_t longtime ;
  struct tm *now ;

  cmtime(&longtime) ;
  now = localtime(&longtime) ;

  ptracef("#%d: %s", index, asctime(now)) ;
}



// Parsing functions for ini file use

#include <stdio.h>
#include <string.h>

/* Get a str to delimiter */
char *getstr(char *outs, char *ins, char delimit)
{  if (*ins != 0)
    { do *outs++ = *ins++ ;
      while (*ins != delimit && *ins != 0) ;
    }
   *outs = 0 ;
   return(ins) ;
} 
  
/* Get a token from a string to a string. Return pointer beyond
   end of token or NULL at end of input string */
char *gettok(char *si, char *st)
{
  if (si == NULL)
   return(NULL) ;

  while ((*si == ' ' || *si == '\t' || *si == '\n') && *si != 0)
    si++ ;
  while (*si != ' ' && *si != '\t' && *si != '\n' && *si != 0)
    *st++ = *si++ ;
  *st = 0 ;
  return((*si) ? si : NULL) ;
}


/* Match first token in a string and return pointer beyond token if matched
   else NULL */
char *matchftok(char *si, char *token)
{ char st[80], *gettok() ;

  if ((si = gettok(si, st)) != NULL && !strcmp(token, st))
    return(si) ;
  else
    return(NULL) ;
}

/* Find first line token in a file */
char *findftoken(FILE *fd, char *token, char *linein)
{ char *upstr ;
  char *matchftok() ;

  while (fgets(linein, 82, fd) != NULL)
    { if ((upstr = matchftok(linein, token)) != NULL)
	return(upstr) ;
    }
  fseek(fd, 0L, 0) ;
  return(NULL) ;
}

int getinistring(char *file, char *section, char *key, char *param, int size)
{ FILE *fd ;
  char linein[82], token1[82], token2[82], *sk, *sp ;
 
  *param = 0 ;

  if ((fd = fopen(file, "r")) == NULL)
   return 1 ;


  if (findftoken(fd, section, linein) == NULL)
   { fclose(fd) ;
     return 2 ;
   }

  while (fgets(linein, 82, fd) != NULL)
   {  gettok(linein, token1) ;
      sk = token1 ;
      if (*sk == '[' && *(sk+strlen(sk)-1) == ']')
       { fclose(fd) ;
         return 3 ;
       }
      sp = getstr(token2, sk, '=') ;
      if (!strcmp(token2, key))
        { sp++ ;
          if (size == 0)
            strcpy(param, sp) ;
          else
           { strncpy(param, sp, size) ;
             *(param + size - 1) = 0 ;
           }
          fclose(fd) ;
          return 0 ;
        }

   }

  fclose(fd) ;
  return 3;
}



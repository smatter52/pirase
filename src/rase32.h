/* **************************************************** */
/* RASE32 definitions */

#pragma option -a4

#define TRUE       1
#define FALSE      0

#define KB         *1024

#define WAIT_DELAY(x) wait_delay(x)

#define SEC     *18  /* no of DOS ticks in a second actually 18.2 */
#define SECOND  *18
#define SECONDS *18

#define MIN     *1092
#define MINUTE  *1092  /* DOS ticks in a minute */
#define MINUTES *1092

#include <time.h>
#include "rase_setjmp.h"

/* structure for scheduler task descriptor */
struct taskdata {
		rase_jmp_buf env ;    /* Thread environment */
		void (*tfp)() ;       /* Task function pointer */
		int  state ;          /* 0 Waiting 1 running -1 terminated */
		int  actno ;          /* activity number */
		char  *stk_ptr ;      /* inital stack pointer for task */
		char  *actname ;      /* pointer to activity name */
		char  *stk_top ;      /* pointer to top of stack */
      unsigned int  stacksize ;  /* At least this number of bytes */
      };


/* typedef struct for time out function varables */
typedef struct { unsigned long tick; /* system clock ticks 18.2 per second */
                 unsigned long time; /* time to wait in clock ticks */
                 unsigned long trem; /* time remaining */
               } time_o;


long   dbldate(double *),
       dbltime(double *),
       dbldatetime(double *) ;
double dttod(char *, char *, int) ;
 

char   *strdate(long),
       *strdate2(long),
       *strtime(long),
       *datim(double),
       *tim(double),
       *dat(double) ;

int    psprintf(char *,const char *,...),
       pscanf(char *,const char *,...),
       messat( int ),
       time_out( unsigned long, time_o * );
int    rase_messat( int mp ) ;
void   rase_send( int, unsigned, void * ),
       rase_receive( int, unsigned, void * ),
       rase_task( int, ... ),
       rase_unlock( void ),
       kill_alltasks(void),
       wait_delay(long) ;
       void stampnow(int index) ;
#ifdef __WIN32__
void timer_on(void) ;
void timer_off(void) ;
#endif

/* *************************************************** */
/* Definition for C persistent structure database */
/* AJM Beddow */

#pragma ZTC align 4

#define DDKEY8_1  0x2f    // 8 bit byte aligned
#define DDCKEY8_1  0x31    // 8 bit byte aligned .dd word encrypted
#define DDKEY16_2  0x3f    // 16 bit word aligned
#define DDKEY16_4  0x38    // 16 bit dword aligned
#define DDKEY32_2  0x4f    // 32 bit word aligned
#define DDKEY32_4  0x48    // 32 bit dword aligned
#define DDCKEY32_2 0x5f    // 32 bit encrypted .dd word aligned
#define DDCKEY32_4 0x58    // 32 bit encrypted .dd dword aligned
#define DDCMAC32_2 0x6f    // 32 bit Mac encrypted .dd word aligned
#define DDCMAC32_4 0x68    // 32 bit Mac encrypted .dd dword aligned

#define NUMDSDAT 10
#define DSTRUCTNAM 32

/* Database structure descriptor */
struct dsdesc {
   char *name ;                   /* Name pointer */
   unsigned short type ;          /* Type index */
   unsigned short maxtypesize ;   /* Max size of any type in the structure */
   long size ;                    /* Size of object */
   struct dbdesc **slist ;        /* Pointer to Null terminated list
                                     of pointers to descriptors */
   struct dsdesc *prev ; /* link to previous entry defined in def file */
	} ;

/* Database descriptor */
struct dbdesc {
   char *name ;                   /* Name pointer */
   unsigned short type ;          /* Type index */
   unsigned short pad ;
   unsigned long maxind ;         /* Maximum size of array */
   struct dsdesc *dsdescptr ;     /* Pointer to structure def (if any) */
   } ;

/* Domain definition entry structure */
struct domdef {
	char    name[DSTRUCTNAM] ;      /* Domain name */
	unsigned short   stream ;       /* Stream mask (memory attribute)  */
	unsigned short   pad ;
	unsigned long    itemnum ;      /* No of items in domain */
	unsigned long    byteoff ;      /* Byte offset into stream */
	unsigned long    itemsize ;     /* Item size size in bytes */
		} ;

/* Domain definition header */
struct domdefhead {
	unsigned short    ddkey ;         /* dd file ident */
	unsigned short    ver ;           /* Version */
	unsigned long     no_entrys ;     /* Number of entries */
	unsigned long    dsize ;          /* Data base size */
	unsigned long    dsize2 ;         /* Data base size upper word */
	unsigned long    no_dstruct ;    /* Number of database structures */
	unsigned long    chk_sum ;       /* De-crypt checksum */
	} ;

/* Structure offset and dimension pairs */
struct dsdat {
      unsigned short type ;
      unsigned short  pad ;
      unsigned long maxind ;
      unsigned long size ;
      unsigned long offset ;
      } ;

/* Control structure for each database structure defined */
struct dsctrl {
     unsigned short lock ;
     unsigned short read_ref ;
     unsigned short write_ref ;
     char *read_fn_name ;
     void (*read_fn_ptr)(void) ;
     char *write_fn_name ;
     void (*write_fn_ptr)(void) ;
     char *upd_fn_name ;
     void (*upd_fn_ptr)(void) ;
     } ;

/* Database structure definintion entry structure */
struct dstructdef {
   char name[DSTRUCTNAM] ;         /* Structure name */
   unsigned short nest ;           /* Structure nest level 1-10 */
   unsigned long domindx ;          /* Domain control table index */
   struct dsdat dsdat[NUMDSDAT] ;  /* Structure offset and dimension pairs
                                           for each level */
   } ;

/* Domain control table entry */
struct domentry {
	char    *name;                  /* Domain name */
	unsigned short stream ;         /* Stream no */
	unsigned short pad ;
	unsigned long itemnum ;         /* No of items in domain */
	unsigned long byteoff ;         /* Byte offset into stream */
	unsigned long itemsize ;        /* Item size size in bytes */
	void     *ldata ;               /* Pointer to local data in memory */
	struct  domentry *next ;        /* Link to next for hash table */
		} ;

/* Database stucture name table entry */
struct dstructentry {
       char *name ;                     /* Stucture name (non-dimensioned) */
       unsigned short nest ;            /* 1-65536 nest level */
       unsigned short pad ;
       struct dsdat *dsdatlist ;        /* Pointer to db structure data
  					   for each nest level */
       struct dsctrl dsctrl ;		/* Control structures for events */
       struct domentry *domentryptr ;   /* Pointer to domain control structure */
       struct dstructentry *next ;      /* Link to next for hash table */
       } ;


/* Macros */
#define DENTRY(n)        (domtab + (n))
#define SENTRY(n)        (dstab + (n))
#define DBGET(V,W,X,Y,Z) (dbget(lookup(#V)->domentryptr, (unsigned int)W, (unsigned int) X, (unsigned int) Y, (void *)Z))
#define DBPUT(V,W,X,Y,Z) (dbput(lookup(#V)->domentryptr, (unsigned int)W, (unsigned int) X, (unsigned int) Y, (void *)Z))

/* Function definitions */
#ifdef __cplusplus
extern "C" {
#endif

struct dstructentry *lookup(char *) ;
union  database *dbget(struct domentry *, unsigned int, unsigned int, unsigned int, void *),
                *dbsget(void *, struct dstructentry *, ...),
                *dbs_get(void *, struct dstructentry *, unsigned int *)  ;

void dbput(struct domentry *, unsigned int, unsigned int, unsigned int, void *),
     dbsput(void *, struct dstructentry *, ...) ,
     dbs_put(void *, struct dstructentry *, unsigned int *) ;
int  flush_domain(struct domentry *), flush_item(struct domentry *, unsigned int) ;

// Logging functions
// Mode 1: printf, 2: fprintf to file, 3: OutputDebug String (WIN32)
void ptracef(char *szFormat, ...) ;
void settracemode(int mode) ;
void plogf(char *szFormat, ...) ;
void setlogmode(int mode) ;

long open_dd( char * );
void close_dd(void) ;
void flush_db(void) ;
int creatdb(char *dbname) ;
int opendb(char *dbname) ;
void closedb(void) ;
void closedb_nf(void) ;
int getinistring(char *file, char *section, char *key, char *param, int size) ;
int opendf(char *name) ;
void outp(int port_address, int value) ;
int inp(int port_address) ;

#ifdef __cplusplus
}
#endif


/* Function Prototypes */
#ifdef __cplusplus
extern "C" {
#endif

struct netdgserver *install_dgserver(char *, void *(*)(void *), int) ;
void remove_all_dgservers(void) ;
int net_transact(char *, void *, int, void *, int) ;
int OpenNetClient(char *) ;
int CloseNetClient(void) ;
void net_comms(void) ;
// additional for winsock
int OpenNetClientIP(unsigned short port) ;
int net_transactip(unsigned long serveraddr, unsigned short port, void *bufin, int bufinlen,
                   void *bufout, int bufoutlen) ;
struct netdgserver *install_dgserverip(unsigned short port, void *(*rpc)(void *),
                                     int replysize) ;
int remove_dgserver(char *netname) ;

#ifdef __cplusplus
}
#endif




/* Transaction */
#define NetTransact(b,c,d) net_transact((b),&(c),sizeof(c),&(d),sizeof(d))
#define NetTransact_p(b,c,d) net_transact((b),(c),sizeof((*(c))),(d),sizeof((*(d))))

/* DG Server */
#define Install_DGserver(a,b,c) install_dgserver((a),(void *(*)(void *))(b),(c))



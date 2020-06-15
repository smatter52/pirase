/* ********************************************* */
/* Datagram Server functions for Windows sockets */

/* This is compatible with the NetBios interface
   The 'netbios name' must be defined in Hosts for the IP address
   and Services for the port for net_transact(). OpenNetClient(),
   install_dgserver() and remove_dgserver() will assume a numeric name
   to be a port else look it up in Services.   
*/

#define __RPI__

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "netfnip.h"
#include "rase32.h"
#ifdef mem
  #include <mem.h>
#else
/* Redefine mem package functions to stdlib calls */
 #include "no_mem.h"
#endif

#define NETTIMEOUT 10
#define NETTRY 1
#define NETTIMEOUTERR 30
#define DGRAMSIZE 1024

unsigned long processtics(void);    // In timer.c

 /* Pointer to list of sok  descriptors */
struct netdgserver *netdgserverlist = NULL ;

/* Client socket id */

/* Client socket id */
SOCKET client_sok = INVALID_SOCKET ;

/* Client port in inet byte order (big endian)*/
u_short client_port = -1 ;

/* Timeout value  in tics (18.2/sec)*/
unsigned short nettimeout = NETTIMEOUT ;
/* Try number ( must be > 0) */
unsigned short nettry = NETTRY ;
/* Holds information on last error */
int last_error ;
#ifndef __RPI__
// WSAstartupinfo
  WSADATA wsda;
#endif
// soc init flag
int sokinit = 0 ;


// Local timer
#ifdef DEBUG
u_short timeout_(u_short tics)
{ return 1 ; }
#else
 #ifdef __WIN32__
static u_short timeout_(u_short tics)
{  static long timer_start ;
   static long tim_tics ;
   long ticsgone ;
   long win_tics ;

   win_tics = GetTickCount() / 53 ;
   if (tics)
     { tim_tics = (long) tics ;
       timer_start = win_tics ;
       return(tics) ;
     }
   else
     { ticsgone = win_tics - timer_start ;
       if (ticsgone >= tim_tics)
         return(0) ;
       else
         return (u_short)(tim_tics - ticsgone) ;
     }
}
 #endif
 #ifdef __RPI__
static u_short timeout_(u_short tics)
{  static long timer_start ;
   static long tim_tics ;
   long ticsgone ;
   unsigned long pi_tics ;

   pi_tics = processtics() / 53 ;
   if (tics)
     { tim_tics = (long) tics ;
       timer_start = pi_tics ;
       return(tics) ;
     }
   else
     { ticsgone = pi_tics - timer_start ;
       if (ticsgone >= tim_tics)
         return(0) ;
       else
         return (u_short)(tim_tics - ticsgone) ;
     }
}
 #else        // DOS and DOS386
u_short timeout_(u_short tics)
{  static long timer_start ;
   static long tim_tics ;
   static long ticsgone ;
#ifdef DOS386
   long *timer_low = (long *)((char *)_x32_zero_base_ptr + TIMER_LOW) ;
#else
   long far *timer_low = MK_FP(0, TIMER_LOW) ;
#endif
   long dos_tics ;

   dos_tics = *timer_low ;
   if (tics)
     { 
       tim_tics = (long) tics ;
       timer_start = dos_tics ;
       return(tics) ;
     }
   else
     { if ((dos_tics - timer_start) < 0)      // Check for midnight reset
        { tim_tics -= (ticsgone + dos_tics) ;
          timer_start = dos_tics ;
        }
       ticsgone = dos_tics - timer_start ;
       if (ticsgone >= tim_tics)
         return(0) ;
       else
         return (u_short)(tim_tics - ticsgone) ;
     }
}
 #endif
#endif



/* Exception Handling */
void netl_error(char *szFormat, ...)
{
#ifdef NETDLL
  MessageBox(NULL, szBuffer, "Netlib error", MB_OK) ;
#else
  char szBuffer[256] ;
  va_list args ;

  va_start(args, szFormat) ;
  vsprintf(szBuffer, szFormat, args) ;
  plogf("Netfnip: %s", szBuffer) ;
#endif
}

/* Set network timeout and retry paranmeters */
void NetParam( int timeout, int retry )
{
	if( timeout == 0 )
   	nettimeout = NETTIMEOUT ;
   else
   	nettimeout = (short) timeout ;
	if( retry == 0 )
   	nettry = NETTRY ;
   else
   	nettry = (u_short)retry ;
}


/* *********************************************************** */
/* Net name list search functions */
/* Search the net name descriptor list for an entry by port */
struct netdgserver *netaddr(u_short port)
{  struct netdgserver *srchptr ;

   srchptr = netdgserverlist ;
   while (srchptr->port == port &&(srchptr = srchptr->prev) != NULL) ;
   return(srchptr) ;
}

/* Search the net name descriptor list for an entry by socket*/
struct netdgserver *netid(SOCKET sok)
{  struct netdgserver *srchptr ;

   srchptr = netdgserverlist ;
   while (srchptr->sok != sok && (srchptr = srchptr->prev) != NULL) ;
   return(srchptr) ;
}

// Open/close windows sockets. Use reference counting to determine when
// to call WSAStartup() and WSACleanup()
#ifndef __RPI__
int OpenSockDLL()
{  WORD wVersionRequested = 0x101 ;
   int stat = 0 ;
   
   if (sokinit == 0)
     { stat = WSAStartup(wVersionRequested, &wsda);
       if (stat == 0) sokinit++ ;
     }
   else
    sokinit++ ;

   return(stat) ;
}

int CloseSockDLL()
{  int stat = 0 ;

   if (sokinit > 0)
    { sokinit-- ;
      if (sokinit == 0)
        stat = WSACleanup() ;
    }
   return(stat) ;
}
#endif

/* Open a network client (master)
   Sets up the global client_sok.
   Notes:
   port is in inet byte order (big endian)
   only one client per machine */
int OpenNetClientIP(u_short port)
{  SOCKET sok ;
   SOCKADDR_IN addr ;
   unsigned long nonblock = 1 ;

   if (client_sok != INVALID_SOCKET)
    { netl_error("Client already open") ;
      return(-1) ;
    }
   if (((sok = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))) == INVALID_SOCKET)
     {
#ifdef __RPI__
       last_error = sok ;
#else
       last_error = WSAGetLastError() ;
#endif
       return(-1) ;
     }
#ifdef __RPI__
    if (ioctl(sok, FIONBIO, &nonblock) == INVALID_SOCKET)
       last_error = sok ;
#else
    if (ioctlsocket(sok, FIONBIO, &nonblock) == INVALID_SOCKET)
      last_error = WSAGetLastError() ;
#endif
   addr.sin_family = AF_INET ;
   addr.sin_port = port;
   addr.sin_addr.s_addr = INADDR_ANY ;

   if(bind(sok, (struct sockaddr *) &addr, sizeof(addr)) == INVALID_SOCKET)
     {
#ifdef __RPI__
       last_error = sok ;
#else
       last_error = WSAGetLastError() ;
#endif
     }

   client_sok = sok ;
   return 0 ;
}


/* Open a name network client, NetBios compatible */
int OpenNetClient(char *netname)
{ 
  struct servent *sentry_p ;
  u_short port ;

  if (client_sok != INVALID_SOCKET)
    { netl_error("Client already open") ;
      return(-1) ;
    }
  if (OpenSockDLL() != 0)
    { netl_error("Unable to open socket library %s", netname) ;
      return(-1) ;
    }
  if ((port = htons((u_short)atoi(netname))) == 0)
    { if ((sentry_p = getservbyname(netname, NULL)) == NULL)
       {
#ifndef __RPI__
         last_error = WSAGetLastError() ;
         CloseSockDLL() ;
#endif
         netl_error("Unable to get service name %s", netname) ;
         return 0 ;
       }
      port = sentry_p->s_port ;
    } 

  client_port = port ;

  return(OpenNetClientIP(client_port)) ;
}



int CloseNetClient(void)
{
   if (client_sok == INVALID_SOCKET)
     return -1 ;
   else
    { client_sok = INVALID_SOCKET ;
#ifdef __RPI__
		close(client_sok) ;
#else
		closesocket(client_sok) ;
      CloseSockDLL() ;
#endif
      return 0 ;
    }
}


/* ******************************************************************* */
/* Transact a message with an ip address and port.
   Uses client_sok. This must have been set up first
   by OpenNetClient().
   Will filter any errant in-bound datagrams. Note this function is not
   re-entrant and is locked against re-entry.
   Note serveraddr and port are in inet order (big endian).
*/

int net_transactip(unsigned long serveraddr, u_short port, void *bufin, int bufinlen,
            void *bufout, int bufoutlen)
{
  int cnt ;
  int dgstat = 0, ret ;
  static char transact_lock = 0 ;
  SOCKADDR_IN remote_addr, reply_addr ;
  int reply_addr_len;

  nettry = NETTRY ;
#ifndef NETDLL
  while (transact_lock)
    rase_unlock() ;
#endif
  transact_lock = 1 ;
// Clear any spurious input

  while (recv(client_sok, bufin, bufinlen, 0) != INVALID_SOCKET) ;

  remote_addr.sin_family = AF_INET ;
  remote_addr.sin_port = port ;
  remote_addr.sin_addr.s_addr = serveraddr ;

  ret = sendto(client_sok, bufout, bufoutlen, 0,
              (struct sockaddr *)&remote_addr, sizeof(remote_addr)) ;
  if (ret == INVALID_SOCKET)
     {
#ifdef __RPI__
       last_error = ret ;
#else
       last_error = WSAGetLastError() ;
#endif
       transact_lock = 0 ;
       return(-1) ;
     }

  timeout_(nettimeout) ;
  reply_addr_len = sizeof(SOCKADDR_IN) ;
  for(cnt = 0; cnt < nettry; cnt++)
    { 
      while(((ret = recvfrom(client_sok, bufin, bufinlen, 0,
            (struct sockaddr *) &reply_addr, &reply_addr_len)) == INVALID_SOCKET)
             && timeout_(0) > 0)
#ifndef NETDLL
        rase_unlock() ;
#endif
#ifdef __WIN32__
        Sleep(2);
#endif
      if (ret != INVALID_SOCKET)
       {  // printf("%s %s\n", inet_ntoa(remote_addr.sin_addr), inet_ntoa(remote_addr.sin_addr)) ;
          if (remote_addr.sin_addr.s_addr == reply_addr.sin_addr.s_addr ||
              remote_addr.sin_addr.s_addr == INADDR_BROADCAST)
          { dgstat = 0 ;
            break ;
          }       
         cnt-- ;
       }
      else
       { if (nettry - cnt > 1)
          { timeout_(nettimeout) ;
            printf("Sendto Retry\n") ;
            ret = sendto(client_sok, bufout, bufoutlen, 0,
                  (struct sockaddr *) &remote_addr, sizeof(remote_addr)) ;
            if (ret == INVALID_SOCKET)
              {
#ifdef __RPI__
              last_error = ret ;
#else
              last_error = WSAGetLastError() ;
#endif
                dgstat = -1 ;
              }

          }
       }
    }
  if (cnt >= nettry)
     {
       if (!dgstat)
         dgstat = NETTIMEOUTERR ;
     }


  transact_lock = 0 ;
  return(dgstat) ;
}

/* Transact a message, Net Bios compatible */
int net_transact(char *netname, void *bufin, int bufinlen,
                 void *bufout, int bufoutlen)
{  u_long ip_addr ;
   struct servent *sentry_p ;
   struct hostent *hentry_p ;

   if ((hentry_p = gethostbyname(netname)) == NULL)
     {
#ifndef __RPI__
       last_error = WSAGetLastError() ;
#endif
       netl_error("Unable to get host name %s", netname) ;
       return(-1) ;
     }
   ip_addr = (*((IN_ADDR *)*hentry_p->h_addr_list)).s_addr ;

   if ((sentry_p = getservbyname(netname, NULL)) == NULL)
     {
#ifndef __RPI__
        last_error = WSAGetLastError() ;
#endif
        netl_error("Unable to get service name %s", netname) ;
       return(-1) ;
     }

   return(net_transactip(ip_addr, sentry_p->s_port,
                         bufin, bufinlen, bufout, bufoutlen)) ;
}
/* *********************************************************************** */
/* Install a datagram server on a given port.
   The i_address and port descriptor is entered in a linked list.
   Return pointer to descriptor if successful

   Note port is in inet order (big endian)
*/
struct netdgserver *install_dgserverip(u_short port, void *(*rpc)(void *),
                                     int replysize)
{  struct netdgserver *ep ;
   SOCKADDR_IN addr ;
   SOCKET sok ;
   unsigned long nonblock = 1 ;

/* Malloc descriptor space */
    if ((ep = (struct netdgserver *)mem_malloc(sizeof(struct netdgserver))) == NULL)
      return(NULL) ;
/* Clear error count and last_dgstat */
   ep->errcnt = 0 ;
   ep->last_dgstat = 0 ;
/* Set up Remote procedure call and replysize */
#ifndef NETDLL
    ep->rpc =  rpc ;
#else
    ep->rpc = (void *(__stdcall *)(void *))rpc ;
#endif
    if (replysize < DGRAMSIZE)
      ep->replysize = (short) replysize ;
    else
      ep->replysize = DGRAMSIZE ;

    /* Malloc space for Datagram buffer */
    if ((ep->dgbufin = (char *)mem_malloc(DGRAMSIZE)) == NULL)
      { mem_free(ep) ;
        return(NULL) ; 
      }

/* Install socket */
   if (((sok = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))) == INVALID_SOCKET)
     {
#ifdef __RPI__
       last_error = sok ;
#else
       last_error = WSAGetLastError() ;
#endif

       mem_free(ep->dgbufin) ;
       mem_free(ep) ;
       return(NULL) ;
     }
#ifdef __RPI__
	if (ioctl(sok, FIONBIO, &nonblock) == INVALID_SOCKET)
      { last_error = sok ;
#else
	if (ioctlsocket(sok, FIONBIO, &nonblock) == INVALID_SOCKET)
      { last_error = WSAGetLastError() ;
#endif
        mem_free(ep->dgbufin) ;
        mem_free(ep) ;
        return(NULL) ;
      }

	addr.sin_family = AF_INET ;
	addr.sin_port = port ;
	addr.sin_addr.s_addr = INADDR_ANY ;

   if (bind(sok, (struct sockaddr *) &addr, sizeof(addr)) == INVALID_SOCKET)
     {
#ifdef __RPI__
       last_error = sok ;
#else
       last_error = WSAGetLastError() ;
#endif
       mem_free(ep->dgbufin) ;
       mem_free(ep) ;
       return(NULL) ;
     }

/* Set up id */
   ep->sok = sok  ;
   ep->port = port ;

/* and link in */
   if (netdgserverlist == NULL)
     { netdgserverlist = ep ;
       ep->prev = NULL ;
     }
   else
     { ep->prev = netdgserverlist ;
       netdgserverlist = ep ;
     }

   return(ep) ;
}


/* Install a datagram server, compatible with NetBios */
struct netdgserver *install_dgserver(char *netname, void *(*rpc)(void *),
                                     int replysize)
{
   struct servent *sentry_p ;
   u_short port ;

   if (OpenSockDLL() != 0)
     { netl_error("Unable to open socket library %s", netname) ;
       return(NULL) ;
     }

   if ((port = htons((u_short)atoi(netname))) == 0)
    { if ((sentry_p = getservbyname(netname, NULL)) == NULL)
       {
#ifndef __RPI__
         last_error = WSAGetLastError() ;
#endif
         netl_error("Unable to get service name %s", netname) ;
         return(NULL) ;
       }
      port = sentry_p->s_port ;
    }

   return(install_dgserverip(port, rpc, replysize)) ;
}


/* ****************************************************************** */ 
/* Remove all UDP servers servers */

void remove_all_dgservers(void)
{ char *ptr ;

  if (netdgserverlist != NULL)
    { do
       { 
         if (netdgserverlist->errcnt)
          { netl_error("%d errors occured on dgserver on port %d\n", netdgserverlist->errcnt,
                    htons(netdgserverlist->port)) ;
            netl_error("Last error was %d\n", netdgserverlist->last_dgstat)  ;
          }
         mem_free(netdgserverlist->dgbufin) ;
#ifdef __RPI__
			close(client_sok) ;
#else
			closesocket(client_sok) ;
			CloseSockDLL() ;
#endif
         ptr = (char *) netdgserverlist ;
         netdgserverlist = netdgserverlist->prev ;
         mem_free(ptr) ;
       }
      while (netdgserverlist != NULL) ;
    }
}

/* ***************************************************************** */
/* Remove a named dgserver by name or port number.
   Note names must start with and alpha character
*/
int remove_dgserver(char *netname)
{
   struct servent *sentry_p ;
   struct netdgserver *list_p, *prev_p ;
   u_short port ;
   int ret = -1 ;

   if ((port = htons((u_short)atoi(netname))) == 0)
    { if ((sentry_p = getservbyname(netname, NULL)) == NULL)
       {
#ifndef __RPI__
         last_error = WSAGetLastError() ;
#endif
         netl_error("Unable to get service name %s", netname) ;
         return 0 ;
       }
      port = sentry_p->s_port ;
    } 

   prev_p = NULL ;
   list_p = netdgserverlist ;
   if (list_p != NULL)
    { do
       { if (port == list_p->port)
          { if (list_p->errcnt)
             { netl_error("%d errors occured on dgserver on port %d\n", netdgserverlist->errcnt,
                           htons(netdgserverlist->port)) ;
               netl_error("Last error was %d\n", netdgserverlist->last_dgstat)  ;
             }
            mem_free(list_p->dgbufin) ;
#ifdef __RPI__
				close(client_sok) ;
#else
				closesocket(client_sok) ;
				CloseSockDLL() ;
#endif
	         if (prev_p == NULL)
             netdgserverlist = NULL ;
            else
             prev_p->prev = list_p->prev ;
            mem_free((char *) list_p) ;
            ret = 0 ;
            break ;
          }
         prev_p = list_p ;
         list_p = list_p->prev ;
       }
      while (list_p != NULL) ;
    }
   return(ret) ;
}


/* ****************************************************************** */
/* Son of comms, net_comms(). This is the datagram server function that
   services transaction requests. It must be called regularly to service
   all pending transaction requests                                    */
void net_comms(void)
{ struct netdgserver *netdgserverptr ;
  int ret ;
  int iRemoteAddrLen ;
  void *replyptr ;
  SOCKADDR_IN remote_addr ;

/* Track list serving any pending datagrams by calling the server funtion
   with a void pointer to the datagram.
   The server function will return a pointer to its reply message which is
   returned to the transaction master.
*/
  netdgserverptr = netdgserverlist ;
  while(netdgserverptr != NULL)
   {
    iRemoteAddrLen = sizeof(remote_addr) ;
    if ((ret = recvfrom(netdgserverptr->sok, netdgserverptr->dgbufin,
          DGRAMSIZE, 0, (struct sockaddr *) &remote_addr, &iRemoteAddrLen))
          != INVALID_SOCKET)
      if (ret > 0)
      { 
        replyptr = (*netdgserverptr->rpc)(netdgserverptr->dgbufin) ;
        ret = sendto(netdgserverptr->sok, replyptr, netdgserverptr->replysize, 0,
                     (struct sockaddr *) &remote_addr, sizeof(remote_addr));
        if (ret == INVALID_SOCKET)
         { netdgserverptr->errcnt++ ;
#ifdef __RPI__
           netdgserverptr->last_dgstat = (u_short) ret ;
#else
           netdgserverptr->last_dgstat = (u_short) WSAGetLastError() ;
#endif
         }
      }

     netdgserverptr = netdgserverptr->prev ;
   }
}



#ifdef NETDLL
// serveraddr and port are in little endian order
int _export __stdcall NetlTransactIP(unsigned long serveraddr, u_short port,
                         void *bufin, int bufinlen, void *bufout, int bufoutlen)
{ if (client_sok == INVALID_SOCKET)
   return -1 ;
  else
   return net_transactip(htonl(serveraddr), htons(port),
                         bufin, bufinlen, bufout, bufoutlen) ;
}

int _export __stdcall NetlTransact(chat *netname,
                         void *bufin, int bufinlen, void *bufout, int bufoutlen)
{ if (client_sok == INVALID_SOCKET)
   return -1 ;
  else
   return net_transact(netname, bufin, bufinlen, bufout, bufoutlen) ;
}

int _export __stdcall NetlOpenServerIP(u_short port,
                                 void *(__stdcall *rpc)(void *), int replysize)
{
  if (install_dgserverip(htons(port), (void *(*)(void *))rpc, replysize) == NULL)
    return -1 ;
  else
    return 0 ;
}
int _export __stdcall NetlOpenServer(char *netname,
                                 void *(__stdcall *rpc)(void *), int replysize)
{
  if (install_dgserverip(netname, (void *(*)(void *))rpc, replysize) == NULL)
    return -1 ;
  else
    return 0 ;
}

void _export __stdcall NetlCloseServers(void)
{ remove_all_dgservers() ; }

int _export __stdcall NetlOpenClientIP(u_short port)
{ return OpenNetClientIp(htons(port)) ; }

int _export __stdcall NetlOpenClient(char *netname)
{ return OpenNetClientIp(netname) ; }

int _export __stdcall NetlCloseClient(void)
{ return CloseNetClient() ;}

void _export __stdcall NetlComms(void)
{
  net_comms() ;
#ifdef __WIN32__
  Sleep(2) ;
#endif
}

// Alternative for VBA callbacks which cannot return a structure pointer
// These funtions are called with a additional pointer to a buffer for the
// reply. Note if Server is initialised with reply size 4 or less the status is
// returned directly and no buffer pointer is required
void _export __stdcall NetlVBAComms(void)
{ struct netdgserver *netdgserverptr ;
  unsigned int rpcstat ;
  static char replybuf[DGRAMSIZE];
  int (__stdcall *vbarpc1)(void *) ;
  int (__stdcall *vbarpc2)(void *, char *) ;
  int ret ;
  int iRemoteAddrLen ;
  SOCKADDR_IN remote_addr ;

  netdgserverptr = netdgserverlist ;
  while(netdgserverptr != NULL)
   {
    iRemoteAddrLen = sizeof(remote_addr) ;
    if ((ret = recvfrom(netdgserverptr->sok, netdgserverptr->dgbufin,
          DGRAMSIZE, 0, (struct sockaddr *) &remote_addr, &iRemoteAddrLen))
          != INVALID_SOCKET)
      if (ret > 0)
      {  if (netdgserverptr->replysize <= 4)
         { vbarpc1 = (int (__stdcall *)(void *))netdgserverptr->rpc ;
           rpcstat = (*vbarpc1)(netdgserverptr->dgbufin) ;
         }
        else
         { vbarpc2 = (int (__stdcall *)(void *, char *))netdgserverptr->rpc ;
           rpcstat = (*vbarpc2)(netdgserverptr->dgbufin, replybuf) ;
         }
        if (netdgserverptr->replysize <= 4)
         { *((int *)replybuf) = rpcstat ;
            ret = sendto(netdgserverptr->sok, replybuf, sizeof(long), 0,
                     (struct sockaddr *) &remote_addr, sizeof(remote_addr));
         }
        else
         { *((int *)replybuf) = rpcstat ;
            ret = sendto(netdgserverptr->sok, replybuf, netdgserverptr->replysize, 0,
                     (struct sockaddr *) &remote_addr, sizeof(remote_addr));
         }
        if (ret == INVALID_SOCKET)
         { netdgserverptr->errcnt++ ;
           netdgserverptr->last_dgstat = (u_short) WSAGetLastError() ;
         }
      }

     netdgserverptr = netdgserverptr->prev ;
   }

  Sleep(1) ;
}


void _export __stdcall NetlParam( int timeout, int retry )
{
	if( timeout == 0 )
   	nettimeout = NETTIMEOUT ;
   else
   	nettimeout = timeout ;
	if( retry == 0 )
   	nettry = NETTRY ;
   else
   	nettry = retry ;
}

void _export __stdcall NetlOpenSockDLL(void)
{ OpenSockDLL() ; }

void _export __stdcall NetlCloseSockDLL(void)
{ CloseSockDLL() ; }

int _export __stdgall NetlGetLastError(void)
{ return last_error ; }

#endif


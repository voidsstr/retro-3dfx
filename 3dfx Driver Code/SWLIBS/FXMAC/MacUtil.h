#include <string.h>


#ifndef __MACUTIL_H__
#define __MACUTIL_H__


extern void strupr( char* cptr );
void MacCheckBoardsInSystem(void);
void SstSetupMac(void);
void SstCleanupMac(void);
void ErrorMacCallback( char* inMessage );


#endif /* __MACUTIL_H__ */

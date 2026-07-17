/* 
 * etypes.h
 *
 * Types from ediag.h
 *
 */

#ifndef MDC_ETYPES_H
#define MDC_ETYPES_H

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE  1
#endif
#ifndef BOOL
#define BOOL WORD
#endif

#ifndef NULL
#define NULL 0
#endif


// Industry standard data types and others
#define	FAR
#ifndef BYTE
typedef	unsigned char			BYTE;
#endif
#ifndef WORD
typedef	unsigned short			WORD;
#endif
#ifndef DWORD
typedef	unsigned long			DWORD;
#endif
#ifndef BOOL
typedef	WORD						BOOL;
#endif
#ifndef LPBYTE
typedef	BYTE						FAR	*LPBYTE;
#endif
#ifndef LPWORD
typedef	WORD						FAR	*LPWORD;
#endif
#ifndef LPDWORD
typedef	DWORD						FAR	*LPDWORD;
#endif
#ifndef LPBOOL
typedef	BOOL						FAR	*LPBOOL;
#endif
#ifndef LPSTR
typedef	char						FAR	*LPSTR;
#endif
#ifndef SEGOFF
typedef	DWORD						SEGOFF;
#endif
#ifndef LPVOID
typedef	void						FAR	*LPVOID;
#endif

// Industry standard macros
#ifndef LOBYTE
#define LOBYTE(w)					((BYTE)(w))
#endif
#ifndef HIBYTE
#define HIBYTE(w)					((BYTE)(((WORD)(w) >> 8) & 0xFF))
#endif
#ifndef LOWORD
#define LOWORD(l)					((WORD)((DWORD)(l) & 0xFFFF))
#endif
#ifndef HIWORD
#define HIWORD(l)					((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))
#endif

#ifndef __max
#define	__max(a,b)					(((a) > (b)) ? (a) : (b))
#endif
#ifndef __min
#define	__min(a,b)					(((a) < (b)) ? (a) : (b))
#endif


#endif /* MDC_ETYPES_H */

/*
** hash.h
**
** Hector Yee
** yee@3dfx.com yhy1@cornell.edu
**
** Implements a hash table like data structure
** used to cache LFB, Textures and state variables
**
*/

#if !defined(HASH__INCLUDED_)
#define HASH__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif /* _MSC_VER >= 1000 */

#ifdef __cplusplus
extern "C" {
#endif

/* some appropriate prime number */
/* don't want user selectable in case they choose non primes */
#define TABLE_SIZE 1021


/* our data items */
typedef struct _ITEM {
	unsigned long signature;		/* crc sig */
	unsigned long MD4Digest[4];		/* MD4 cryptodigest */	
	unsigned long token;			/* item unique ID */
	struct _ITEM *next;
} ITEM;

typedef struct _HashTable {
	long numitems;	
	ITEM* table[TABLE_SIZE];
} HashTable;

unsigned long getcrc_buffer(void *buffer, long size);
void    crcgen( void );
HashTable*  MakeHashTable();
void AddItem(HashTable *h, unsigned long sig,unsigned long *digest, int place);
unsigned long find(HashTable *h, void *data, long length);
void *getData(HashTable *h, unsigned long token);
void KillHashTable(HashTable *h);

#ifdef __cplusplus
extern "C" {
#endif

#endif /* HASH__INCLUDED_ */

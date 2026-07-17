/*
** hash.c
**
** Hector Yee
** yee@3dfx.com yhy1@cornell.edu
**
** Implements a hash table like data structure
** used to cache LFB, Textures and state variables
**
** To use ... just MakeHashtable, and then
** use find on your data. if an item is found, token will be returned
** else it will make a new token and return that.
**
*/

#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned long crcTable[256];

unsigned long getcrc_buffer(void *buffer, long size)
/* Modified from Craig Bruce's C code */
/* http://www.cyberus.ca/~csbruce/unix/ */

{
    /* caution! crc is calculated backwards in this version,for speed */

    register unsigned long crc;
    int     c;

    crc = 0xFFFFFFFF;
    while(size) {
        size--;
        c=* ((unsigned char *) buffer + size);
        crc = ((crc>>8) & 0x00FFFFFF) ^ crcTable[ (crc^c) & 0xFF ];
    }
    return( crc^0xFFFFFFFF );
}

void crcgen( void )
/* Modified from Craig Bruce's C code */
/* http://www.cyberus.ca/~csbruce/unix/ */

{
    unsigned long crc, poly;
    int     i, j;

    poly = 0xEDB88320L;
    for (i=0; i<256; i++) {
        crc = i;
        for (j=8; j>0; j--) {
            if (crc&1) {
                crc = (crc >> 1) ^ poly;
            } else {
                crc >>= 1;
            }
        }
        crcTable[i] = crc;
    }
}

HashTable*  MakeHashTable()
/* Makes the hash table and initializes it */
{
    HashTable* h;

    crcgen();
    h=(HashTable*) malloc(sizeof(HashTable));
    memset((char*)h, 0, sizeof(HashTable));

    return h;
}

void KillHashTable(HashTable *h)
{
    int i;
    ITEM * now;
    ITEM * next;

    for (i=0; i<TABLE_SIZE; i++)
    {
        now=next=h->table[i];
        while (next!=NULL)
        {
            next=now->next;
            free(now);
            now=next;
        }        
    }
    free(h);
    h=NULL;
}

__inline int hashfunction(unsigned int id)
{
    return (id % TABLE_SIZE);
}

__inline void AddItem(HashTable *h, unsigned long sig,unsigned long *digest, int place)
/* adds item with signature in hashtable */
{
    ITEM * newitem;

    h->numitems++;
    /* create new item */
    newitem=(ITEM *) malloc(sizeof(ITEM));
    newitem->signature=sig;
    newitem->token=h->numitems;

    /* put it in hash table at head of linked list */
    newitem->next=h->table[place];
    h->table[place]=newitem;
}


unsigned long find(HashTable *h, void *data, long length)
/* searches for item with signature sig and returns relevant token */
/* CREATES NEW TOKEN IF ITEM IS NOT IN DATABASE!                   */
{
    unsigned long sig;
    unsigned long digest[4];    
    int place;
    ITEM * temp;

    sig=getcrc_buffer(data,length);
    place=hashfunction(sig);

    temp=h->table[place];

    while (temp)
    {
        if (temp->signature==sig)
            return temp->token;
        temp=temp->next;
    }    

    AddItem(h,sig,&digest[0],place);
    return h->numitems;
}


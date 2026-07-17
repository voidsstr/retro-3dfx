/* -*-c++-*- */
/* $Header: textdict.c, 2, 10/11/00 8:55:12 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   textdict.c
**
** Description: Support for text dictionary functions.
**              THIS FILE IS NOT USED!
**
** $Revision: 2$
** $Date: 10/11/00 8:55:12 PM$
**
** $History: textdict.c $
** 
** *****************  Version 3  *****************
** User: Michael      Date: 1/15/99    Time: 7:02a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 2  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/


/*****************************************************************************
 *                                                                           *
 * textdict.c                                                                *
 *                                                                           *
 * Written by F. Weigel (Intelligraphics) for 3Dfx                           *
 *                                                                           *
 * Text dictionary functions.                                                *
 *                                                                           *
 *****************************************************************************/


#include "h3.h"
#include "thunk32.h"

#include "textout.h"


/* Compute hash key. This is (key mod number of entries). The return value
 * will be used as an index, so it must be between 0 and the maximum number
 * of entries. The mod function accomplishes this.
 */
static int h(DICTIONARY *dict, long key)
    {
    DEBUG_FIX

    return (int)(key % dict->max_entries);
    }


/* Look up key in dictionary. */
static int hash_lookup(DICTIONARY *dict, long key)
    {
    int hash;
    int i;

    DEBUG_FIX

    /* Compute the hash of the key (0..max_entries) */
    hash = h(dict, key);

    /* If the entry is a direct hit, record this fact. If it is a collision,
     * record this as well. If the entry is not in use, don't bother
     * gathering statistics.
     */
    if ( GET_STATUS(dict->data[hash]) == INUSE )
        if ( dict->data[hash].key == key )
            {
            #if STATISTICS
                ++dict->direct_hits;
            #endif
            return hash;
            }
#   if STATISTICS
        else
            {
            ++dict->collisions;
            }
#   endif

    /* Check the computed index (hash) to see if this is the entry that we
     * want. If it is not the entry, then we have a collision. Move to the
     * next entry and check that one. If the entry has never been used,
     * the desired key can't possibly be further along, so fail.
     */
    for ( i = hash; i < dict->max_entries; ++i )
        {
        if ( GET_STATUS(dict->data[i]) == NEVERUSED )
            return NIL;
        if ( (GET_STATUS(dict->data[i]) == INUSE) &&
             (dict->data[i].key == key) )
            return i;
        }

    /* The entry was not found between the computed hash index and the end
     * of the dictionary, and we did not hit a NEVERUSED entry. Start searching
     * from the beginning of the dictionary.
     */
    for ( i = 0; i < hash; ++i )
        {
        if ( GET_STATUS(dict->data[i]) == NEVERUSED )
            return NIL;
        if ( (GET_STATUS(dict->data[i]) == INUSE) &&
             (dict->data[i].key == key) )
            return i;
        }

    /* Not found, and we didn't cross a NEVERUSED slot */
    return NIL;
    }


/* Return the size of a dictionary in bytes, given maximum entries. */
int size_of_dictionary(int n)
    {
    return n * sizeof(DICTIONARY_ENTRY) + sizeof(DICTIONARY);
    }


/* Initialize a dictionary */
void initialize_dictionary(DICTIONARY *dict, int max)
    {
    int i;

    DEBUG_FIX

    dict->max_entries = max;
    dict->delete_count = 0;
    dict->n_entries = 0;

#   if STATISTICS
        dict->direct_hits = 0;
        dict->collisions = 0;
        dict->high_water = 0;
#   endif

    /* Initialize all the entries. */
    for ( i = 0; i < max; ++i )
        {
        /* This should optimize into constant store to memory */
        dict->data[i].key = 0;
        SET_VALUE(dict->data[i], 0);
        SET_STATUS(dict->data[i], NEVERUSED);
        }
    }


/* Insert new item into dictionary, returns 1 if ok, else 0 if full. */
int insert_into_dictionary(DICTIONARY *dict, long key, long val)
    {
    int hash, i;

    DEBUG_FIX

    /* Is the currently key in the dictionary? If so, just replace the Value
     * with the new value.
     */
    hash = hash_lookup(dict, key);
    if ( hash != NIL )
        {
        SET_VALUE(dict->data[hash], val);
        return 1;
        }

    /* This is a new key. Hash it to an index, and then look for the first
     * entry not used or never used and put the (key, value) there.
     */
    hash = h(dict, key);
    for ( i = hash; i < dict->max_entries; ++i )
        if ( (GET_STATUS(dict->data[i]) == NOTINUSE) ||
             (GET_STATUS(dict->data[i]) == NEVERUSED) )
            {
InsertEntry:
            dict->data[i].key = key;
            SET_VALUE(dict->data[i], val);
            SET_STATUS(dict->data[i], INUSE);
            ++dict->n_entries;

#           if STATISTICS
                if ( dict->n_entries > dict->high_water )
                    dict->high_water = dict->n_entries;
#           endif

            return 1;
            }

    /* We couldn't find an entry between the hash index and the end of the
     * dictionary. Search again from the start of the dictionary.
     */
    for ( i = 0; i < hash; ++i )
        if ( (GET_STATUS(dict->data[i]) == NOTINUSE) ||
             (GET_STATUS(dict->data[i]) == NEVERUSED) )
            goto InsertEntry;

    /* There aren't any free entries in the dictionary. The insert failed. */
    return 0;
    }


/* Delete item from dictionary. */
void delete_from_dictionary(DICTIONARY *dict, long key)
    {
    int hash;

    DEBUG_FIX

    hash = hash_lookup(dict, key);
    if ( hash != NIL )
        {
        ++dict->delete_count;
        --dict->n_entries;
        dict->data[hash].key = 0;
        SET_STATUS(dict->data[hash], NOTINUSE);
        }
    }


/* Return item from dictionary */
int member_of_dictionary(DICTIONARY *dict, long key, long *val)
    {
    int hash;

    DEBUG_FIX

    hash = hash_lookup(dict, key);
    if ( hash != NIL )
        {
        *val = GET_VALUE(dict->data[hash]);
        return 1;
        }
    return 0;
    }


/* Iterate over the dictionary */
void iterate_dictionary(DICTIONARY *dict, DICT_FUNC f, void *c)
    {
    int i;

    DEBUG_FIX

    for ( i = 0; i < dict->max_entries; ++i )
        if ( GET_STATUS(dict->data[i]) == INUSE )
            if ( !f(dict, dict->data[i].key, GET_VALUE(dict->data[i]), c) )
                return;
    }

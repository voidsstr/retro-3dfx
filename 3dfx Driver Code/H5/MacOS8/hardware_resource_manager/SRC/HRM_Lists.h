/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Header: HRM_Lists.h, 3, 10/11/00 8:35:09 PM, Brent$
** $Log: 
**  3    3dfx      1.1.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    MacOS Dev Tree1.1         01/28/00 Kenneth Dyke    Fix line endings.
**  1    MacOS Dev Tree1.0         01/28/00 Kenneth Dyke    
** $
** 
** 2     8/23/99 2:36p Kcd
** Surface, memory management, and AGP support.
** 
** 1     7/02/99 3:19p Kcd
** HRM linked list support.
**
*/
#ifndef HRM_LISTS_H
#define HRM_LISTS_H 1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Node_s Node_t;

/* My old favorite Amiga style doubly-linked list stuff */

struct Node_s
{
	Node_t *succ;
	Node_t *prev;
};

typedef struct List_s List_t;

/* I know someone is going to look at these next two definitions (List_s & NewList)
 * and scratch their head, so here's the deal:
 *
 * The list header saves space by embedding & overlapping the head and tail sentinel
 * nodes.  Normally we'd have something like this:
 *
 *     		Head		Node		Node		Tail
 * succ		----------->----------->-----------> 0
 * prev      0  <-----------<-----------<-----------
 *
 * The List_s structure just overlaps the 0's from the head & tail nodes.
 *
 * That's it!  The only thing complicated from other systems is that nodes with 
 * 0 succ or 0 prev are not valid nodes.
 */
 
struct List_s
{
	Node_t *head;
	Node_t *tail;
	Node_t *tailPred;
};

/* Okay, enough of that. */

// rcf These were'nt here, and it seems as if they should be...
void NewList(List_t *list);
void AddHead(List_t *list, Node_t *node);
void AddTail(List_t *list, Node_t *node);
Node_t *Remove(Node_t *node);
Node_t *RemHead(List_t *list);
Node_t *RemTail(List_t *list);
void Insert(Node_t *before, Node_t *after);

#ifdef __cplusplus
}
#endif

#endif

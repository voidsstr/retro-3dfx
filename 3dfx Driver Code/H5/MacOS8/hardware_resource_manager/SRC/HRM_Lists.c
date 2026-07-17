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
** $Header: HRM_Lists.c, 3, 10/11/00 8:35:09 PM, Brent$
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

#include "hrm_lists.h"

void NewList(List_t *list)
{
	list->head = (Node_t *)&list->tail;
	list->tail = 0;
	list->tailPred = (Node_t *)list;
}

void AddHead(List_t *list, Node_t *node)
{
	Node_t *head = list->head;
	
	node->succ = head;
	node->prev = head->prev;
	head->prev = node;
	list->head = node;
}

void AddTail(List_t *list, Node_t *node)
{
	Node_t *tail = list->tailPred;
	
	node->succ = tail->succ;
	node->prev = tail;
	list->tailPred = node;
	tail->succ = node;
}

Node_t *Remove(Node_t *node)
{
	node->succ->prev = node->prev;
	node->prev->succ = node->succ;
	return node;
}

Node_t *RemHead(List_t *list)
{
	if(list->head->succ)
		return Remove(list->head);
	return 0;
}

Node_t *RemTail(List_t *list)
{
	if(list->tailPred->prev)
		return Remove(list->head);
	return 0;
}

void Insert(Node_t *before, Node_t *after)
{
  after->succ = before->succ;
  before->succ = after;
  after->succ->prev = after;
  after->prev = before;
}

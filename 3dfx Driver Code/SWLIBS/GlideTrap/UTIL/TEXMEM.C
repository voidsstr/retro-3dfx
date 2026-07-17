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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
*/

/*
**  texmem.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <trap.h>
#include <texmem.h>

TexMemList *tmu_map[MAX_TMUS];

//#define SPEW(A) printf##A
#define SPEW(A)

void AddTextureData(int tmu, int start, int size, void *data)
{
  int end = start+size-1;
  TexMemList   *node,  *prev,  *cur;
  StartEndList *vnode, *vprev, *vcur;

  if ((tmu < 0) || (tmu >= MAX_TMUS)) {
    printf("MAX_TMUS too small in AddTextureData!  Ack... barf...\n");
    exit(0);
  }

  if (size <= 0)
    return;

  SPEW(("new %d,%d (data=%x)\n", start, end, ((unsigned char*)data)[0]));

  // Walk the linked-list from front-to-back.  At each node, subtract
  // out from vis->start/end any part that overlaps with the new data.
  // Delete nodes that become completely obscured.

  for ( prev=NULL, cur=tmu_map[tmu] ; cur ; ) {

    for ( vprev=NULL, vcur=cur->vis ; vcur ; ) {

      if ((vcur->end   < start) ||
          (vcur->start > end)) {

        // zero overlap, just move on to the next StartEndList node
        SPEW(("  vs %d,%d - no overlap\n", vcur->start, vcur->end));
        vprev = vcur;
        vcur  = vcur->next;

      } else if ((vcur->start >= start) &&
                 (vcur->end   <= end)) {

        // complete overlap, remove vcur from StartEndList list
        SPEW(("  vs %d,%d (%2x) - complete overlap\n", vcur->start, vcur->end, ((char*)cur->data)[0]));
        if (vprev) {
          vprev->next = vcur->next;
          free(vcur);
          vcur = vprev->next;
        } else {
          cur->vis = vcur->next;
          free(vcur);
          vcur = cur->vis;
        }

      } else if ((vcur->start < start) &&
                 (vcur->end   > end)) {

        // this is the funky one, we just split an existing texture
        // right down the middle - yowsa!

        SPEW(("  vs %d,%d (%2x) - funky overlap, create %d,%d %d,%d\n",
                          vcur->start, vcur->end, ((char*)cur->data)[0],
                          vcur->start, start-1,
                          end+1, vcur->end));

        vnode = (StartEndList*)malloc(sizeof(StartEndList));
        vnode->start = end+1;
        vnode->end   = vcur->end;
        vnode->next  = vcur->next;

        vcur->end  = start-1;
        vcur->next = vnode;

        vprev = vnode;
        vcur  = vnode->next;

      } else if ((vcur->start <  start) &&
                 (vcur->end   >= start)) {

        // partial overlap, truncate visible portion of cur and continue
        SPEW(("  vs %d,%d (%2x) - truncate end to %d,%d\n",
                          vcur->start, vcur->end, ((char*)cur->data)[0],
                          vcur->start, start-1));
        vcur->end = start-1;
        vprev = vcur;
        vcur  = vcur->next;

      } else if ((vcur->end   >  end) &&
                 (vcur->start <= end)) {

        // partial overlap, truncate visible portion of cur and continue
        SPEW(("  vs %d,%d (%2x) - truncate start to %d,%d\n",
                          vcur->start, vcur->end, ((char*)cur->data)[0],
                          end+1, vcur->end));
        vcur->start = end+1;
        vprev = vcur;
        vcur  = vcur->next;

      } else {

        // this better not ever happen
        printf("Hey!  Got into an illegal state in AddTextureData()\n");
        exit(0);
      }
    }

    // Check to see if this node has any visible segments.
    // If not, nuke it.
    if (!cur->vis) {
      free(cur->data);
      if (prev) {
        prev->next = cur->next;
        free(cur);
        cur = prev->next;
      } else {
        tmu_map[tmu] = cur->next;
        free(cur);
        cur = tmu_map[tmu];
      }
    } else {
      prev = cur;
      cur  = cur->next;
    }
  }

  // add new node at the end of the list

  node = (TexMemList*)malloc(sizeof(TexMemList));
  node->data  = data;
  node->start = start;
  node->size  = size;
  node->next  = NULL;
  node->vis   = (StartEndList*)malloc(sizeof(StartEndList));
  node->vis->start = start;
  node->vis->end   = end;
  node->vis->next  = NULL;

  if (!prev) {
    tmu_map[tmu] = node;
  } else {
    prev->next = node;
  }
}

void FreeAllTextureData(void)
{
  int           i, count;
  TexMemList   *cur, *tm_next;
  StartEndList *vis, *se_next;

  for ( count=0, i=0 ; i<MAX_TMU ; i++ ) {
    for ( cur=tmu_map[i] ; cur ; ) {
      for ( vis=cur->vis ; vis ; ) {
        se_next = vis->next;
        free(vis);
        vis = se_next;
      }
      free(cur->data);

      tm_next = cur->next;
      free(cur);
      cur = tm_next;

      count++;
    }
    tmu_map[i] = NULL;
  }
}

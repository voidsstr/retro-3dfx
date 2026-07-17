#ifndef __CONVEXHULL_H__
#define __CONVEXHULL_H__

#include "revtypes.h"

// for objects
#define ANTI_ALIAS		0x00000001
#define PRE_CALC		0x00000002
#define USE_ALPHA		0x00000004
#define DO_DRAW			0x80000000


typedef struct _edge
{
	GrVertex v0;
	GrVertex v1;
} AAEdge;

typedef struct _edge_info
{
	AAEdge edge;
	int marked;
} AAEdgeInfo;

#define MAX_AA_EDGES 4096

extern int edge_count;
extern AAEdge edge_list[];

void AddEdge(GrVertex v0, GrVertex v1);

void ClearEdges(void);

int ConvexHull(AAEdge *edges, int num_edges);

#define REALTIME 1

#endif // __CONVEXHULL_H__

#ifndef __CONVEXHULL_H__
#define __CONVEXHULL_H__

#ifdef USE_GLIDE3
#include "revtypes.h"
#else
#include <glide.h>
#endif // USE_GLIDE3

// for objects
#define ANTI_ALIAS	0x00000001
#define PRE_CALC		0x00000002
#define USE_ALPHA		0x00000004
#define ENVMAP      0x00000008
#define DO_DRAW			0x80000000


typedef struct _edge
{
	GrVertex v0;
	GrVertex v1;
	int material_id;
} AAEdge;

typedef struct _edge_info
{
	AAEdge edge;
	int marked;
} AAEdgeInfo;

#define MAX_AA_EDGES 4096

extern int edge_count;
extern AAEdge edge_list[];

void AddEdge(GrVertex v0, GrVertex v1, int material_id, int remove_duplicates);
void ClearEdges(void);

int ConvexHull(AAEdge *edges, int num_edges);

#ifdef READ_FROM_FILE
#define ResMem FILE
#define mread  fread
#define mgets  fgets
#define mgetc  fgetc
#else // READ_FROM_FILE
typedef struct _res_mem_
{
	unsigned char *start, *end, *curr;
	unsigned long size;
} ResMem;
size_t mread(void *buffer, size_t size, size_t count, ResMem *res_mem);
char *mgets(char *string, int n, ResMem *res_mem);
int mgetc(ResMem *res_mem);
#endif // READ_FROM_FILE

#define REALTIME 1

#endif // __CONVEXHULL_H__

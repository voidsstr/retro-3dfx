#include <glide.h>
#include "convexhull.h"

AAEdge edge_list[MAX_AA_EDGES];

int edge_count;

#ifndef ABS
#define ABS(a)  (((a) < 0) ? (-(a)) : (a))
#endif

void AddEdge(GrVertex v0, GrVertex v1)
{

	if (edge_count >= MAX_AA_EDGES)
	{
		return;
	}

	edge_list[edge_count].v0 = v0;
	edge_list[edge_count++].v1 = v1;

}

void ClearEdges(void)
{
	edge_count = 0;
}

int ConvexHull(AAEdge *edges, int num_edges)
{
	static AAEdgeInfo edge_info[MAX_AA_EDGES], *tmp_edges[MAX_AA_EDGES];
	int i, j, num_different_edges;
	AAEdge *e0, *e1;

	// can't handle more than MAX_AA_EDGES edges
	if (num_edges > MAX_AA_EDGES)
	{
		return -1;
	}

	// make a copy of all edges and mark them all as not duplicates
	for (i=0; i<num_edges; i++)
	{
		edge_info[i].edge.v0 = edges[i].v0;
		edge_info[i].edge.v1 = edges[i].v1;
		edge_info[i].marked = 0;
	}
	// mark all duplicate edges so we can get rid of them later
	// add all unmarked edges to our final list
	num_different_edges = 0;
	for (i=0, e0 = &edges[0]; i<num_edges-1; i++, e0++)
	{
		for (j=i+1, e1 = &edges[i+1]; j<num_edges; j++, e1++)
		{
			// skip this one if it's already marked as a duplicate
			if (edge_info[j].marked)
			{
				continue;
			}
			// if both endpoints have the same x, y, and oow, then mark it as a duplicate
			// NOTE: v0 and v1 might be in opposite order in the 2 edges, so I have to check both ways
			if ((e0->v0.x - e1->v0.x == 0 && e0->v0.y - e1->v0.y == 0 && e0->v0.oow - e1->v0.oow == 0 &&
				 e0->v1.x - e1->v1.x == 0 && e0->v1.y - e1->v1.y == 0 && e0->v1.oow - e1->v1.oow == 0) ||
				(e0->v0.x - e1->v1.x == 0 && e0->v0.y - e1->v1.y == 0 && e0->v0.oow - e1->v1.oow == 0 &&
				 e0->v1.x - e1->v0.x == 0 && e0->v1.y - e1->v0.y == 0 && e0->v1.oow - e1->v0.oow == 0))
			{
				// see if the intensities are very close, in which case
				// we'll mark both edges as duplicates, otherwise, just
				// mark one of them as a duplicate edge
				if (ABS(e0->v0.r - e1->v1.r) < 16.0f) // assumes r=g=b (using intensities only)
				{
					edge_info[i].marked = 1;
					edge_info[j].marked = 1;
				}
				else
				{
					edge_info[j].marked = 1;
				}
			}
		}
		// if this edge wasn't marked by previous iterations, add it to
		// our final list of edges
		if (!edge_info[i].marked)
		{
			edges[num_different_edges].v0 = edge_info[i].edge.v0;
			edges[num_different_edges].v1 = edge_info[i].edge.v1;
			num_different_edges++;
		}
	}
	// we didn't check the last edge, so check it now
	if (!edge_info[num_edges-1].marked)
	{
		edges[num_different_edges].v0 = edge_info[num_edges-1].edge.v0;
		edges[num_different_edges].v1 = edge_info[num_edges-1].edge.v1;
		num_different_edges++;
	}

	return num_different_edges;
}

#if 0
int ConvexHull0(AAEdge *edges, int num_edges)
{
	static EdgeInfo edge_info[MAX_AA_EDGES], *tmp_edges[MAX_AA_EDGES];
	int i, j, k, num_different_edges, num_outside_edges, curr_num_edges;
	float v0x, v0y, v1x, v1y, crossz0, crossz1;
	GrVertex tmp_vert;

	// can't handle more than 1024 edges
	if (num_edges > MAX_AA_EDGES)
	{
		return -1;
	}

	// make a copy of all edges and mark them all as not duplicates
	for (i=0; i<num_edges; i++)
	{
		edge_info[i].edge.v0 = edges[i].v0;
		edge_info[i].edge.v1 = edges[i].v1;
		edge_info[i].marked = 0;
	}
	// mark all duplicate edges so we can get rid of them later
	for (i=0; i<num_edges; i++)
	{
		for (j=i+1; j<num_edges; j++)
		{
			// skip this one if it's already marked as a duplicate
			if (edge_info[j].marked)
			{
				continue;
			}
			// if both endpoints have the same x, y, and oow, then mark it as a duplicate
			// NOTE: v0 and v1 might be in opposite order in the 2 edges, so I have to check both ways
			if ((edges[i].v0.x == edges[j].v0.x && edges[i].v0.y == edges[j].v0.y && edges[i].v0.oow == edges[j].v0.oow &&
					 edges[i].v1.x == edges[j].v1.x && edges[i].v1.y == edges[j].v1.y && edges[i].v1.oow == edges[j].v1.oow) ||
					(edges[i].v0.x == edges[j].v1.x && edges[i].v0.y == edges[j].v1.y && edges[i].v0.oow == edges[j].v1.oow &&
					 edges[i].v1.x == edges[j].v0.x && edges[i].v1.y == edges[j].v0.y && edges[i].v1.oow == edges[j].v0.oow))
			{
				edge_info[j].marked = 1;
			}
		}
	}
	// now copy all edges that weren't marked as duplicate
	num_different_edges = 0;
	for (i=0; i<num_edges; i++)
	{
		if (!edge_info[i].marked)
		{
			edges[num_different_edges].v0 = edge_info[i].edge.v0;
			edges[num_different_edges].v1 = edge_info[i].edge.v1;
			num_different_edges++;
		}
	}

	// make a copy of all edges and mark them all as not inside
	for (i=0; i<num_different_edges; i++)
	{
		edge_info[i].edge.v0 = edges[i].v0;
		edge_info[i].edge.v1 = edges[i].v1;
		edge_info[i].marked = 0;
	}
	// mark all edges that are in between other edges as being inside
	for (i=0; i<num_different_edges; i++)
	{
		// find all edges that have a common end point with edges[i] at v0
		// and put them in the tmp_edges array
		tmp_edges[0] = &edge_info[i];
		curr_num_edges = 1;
		for (j=i+1; j<num_different_edges; j++)
		{
			if (edge_info[i].edge.v0.x == edge_info[j].edge.v0.x &&
					edge_info[i].edge.v0.y == edge_info[j].edge.v0.y &&
					edge_info[i].edge.v0.oow == edge_info[j].edge.v0.oow)
			{
				tmp_edges[curr_num_edges] = &edge_info[j];
				curr_num_edges++;
			}
			else if (edge_info[i].edge.v0.x == edge_info[j].edge.v1.x &&
							 edge_info[i].edge.v0.y == edge_info[j].edge.v1.y &&
							 edge_info[i].edge.v0.oow == edge_info[j].edge.v1.oow)
			{
				// swap v0 and v1 so that v0 of edge_info[j] coincides with v0 of edge_info[i]
				tmp_vert = edge_info[j].edge.v0;
				edge_info[j].edge.v0 = edge_info[j].edge.v1;
				edge_info[j].edge.v1 = tmp_vert;
				tmp_edges[curr_num_edges] = &edge_info[j];
				curr_num_edges++;
			}
		}
		// check all edges with a common end point at v0 to see
		// which edges are on the outside, otherwise, mark them as inside edges
		for (j=0; j<curr_num_edges; j++)
		{
			v0x = tmp_edges[j]->edge.v1.x - tmp_edges[j]->edge.v0.x;
			v0y = tmp_edges[j]->edge.v1.y - tmp_edges[j]->edge.v0.y;
			crossz0 = 0.0f;
			for (k=0; k<curr_num_edges; k++)
			{
				if (j == k)
				{
					continue;
				}
				v1x = tmp_edges[k]->edge.v1.x - tmp_edges[k]->edge.v0.x;
				v1y = tmp_edges[k]->edge.v1.y - tmp_edges[k]->edge.v0.y;
				// compute the z component of the cross product (v0 x v1)
				crossz1 = v0x*v1y - v0y*v1x;
				if (crossz0)
				{
					// see if the cross products don't have the same sign
					// then this edge is an inside edge and will be marked
					if (((*(int *)&crossz0) & 0x80000000) != ((*(int *)&crossz1) & 0x80000000))
					{
						tmp_edges[j]->marked = 1;
						break;
					}
				}
				else
				{
					crossz0 = crossz1;
				}
			}
		}

		// find all edges that have a common end point with edges[i] at v1
		// and put them in the tmp_edges array
		tmp_edges[0] = &edge_info[i];
		curr_num_edges = 1;
		for (j=i+1; j<num_different_edges; j++)
		{
			if (edge_info[i].edge.v1.x == edge_info[j].edge.v1.x &&
					edge_info[i].edge.v1.y == edge_info[j].edge.v1.y &&
					edge_info[i].edge.v1.oow == edge_info[j].edge.v1.oow)
			{
				tmp_edges[curr_num_edges] = &edge_info[j];
				curr_num_edges++;
			}
			else if (edge_info[i].edge.v1.x == edge_info[j].edge.v0.x &&
							 edge_info[i].edge.v1.y == edge_info[j].edge.v0.y &&
							 edge_info[i].edge.v1.oow == edge_info[j].edge.v0.oow)
			{
				// swap v0 and v1 so that v1 of edge_info[j] coincides with v1 of edge_info[i]
				tmp_vert = edge_info[j].edge.v0;
				edge_info[j].edge.v0 = edge_info[j].edge.v1;
				edge_info[j].edge.v1 = tmp_vert;
				tmp_edges[curr_num_edges] = &edge_info[j];
				curr_num_edges++;
			}
		}
		// check all edges with a common end point at v1 to see
		// which edges are on the outside, otherwise, mark them as inside edges
		for (j=0; j<curr_num_edges; j++)
		{
			v0x = tmp_edges[j]->edge.v0.x - tmp_edges[j]->edge.v1.x;
			v0y = tmp_edges[j]->edge.v0.y - tmp_edges[j]->edge.v1.y;
			crossz0 = 0.0f;
			for (k=0; k<curr_num_edges; k++)
			{
				if (j == k)
				{
					continue;
				}
				v1x = tmp_edges[k]->edge.v0.x - tmp_edges[k]->edge.v1.x;
				v1y = tmp_edges[k]->edge.v0.y - tmp_edges[k]->edge.v1.y;
				// compute the z component of the cross product (v0 x v1)
				crossz1 = v0x*v1y - v0y*v1x;
				if (crossz0)
				{
					// see if the cross products don't have the same sign
					// then this edge is an inside edge and will be marked
					if (((*(int *)&crossz0) & 0x80000000) != ((*(int *)&crossz1) & 0x80000000))
					{
						tmp_edges[j]->marked = 1;
						break;
					}
				}
				else
				{
					crossz0 = crossz1;
				}
			}
		}
	}

	// now copy all edges that weren't marked as being inside
	num_outside_edges = 0;
	for (i=0; i<num_different_edges; i++)
	{
		if (!edge_info[i].marked)
		{
			edges[num_outside_edges].v0 = edge_info[i].edge.v0;
			edges[num_outside_edges].v1 = edge_info[i].edge.v1;
			num_outside_edges++;
		}
	}

	return num_outside_edges;
}

#endif
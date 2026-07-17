#include <stdio.h>
#include <glide.h>
#include "clip.h"

#define USE_ASM

#define MAX_INTERMIDIATE_VERTS  18

// extra vertices used when intersections create new vertices
static GrVertex gClippedVerts[MAX_INTERMIDIATE_VERTS], *gCurrFreeClippedVert;

// global variables
float gMinClipX, gMinClipY, gMinClipZ, gMaxClipX, gMaxClipY, gMaxClipZ;
float gMinClipZInv, gMaxClipZInv;

// this table takes a 3-bit sign mask and creates a 6-bit sign mask as follows:
// 000 - 000000 - 0x00
// 001 - 000011 - 0x03
// 010 - 001100 - 0x0c
// 011 - 001111 - 0x0f
// 100 - 110000 - 0x30
// 101 - 110011 - 0x33
// 110 - 111100 - 0x3c
// 111 - 111111 - 0x3f
int gSignMaskTable[8] = {0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f};


// the near_plane is used when clipping vertices with a negative W
// these vertices get clipped to the near clip plane, so the resulting
// W becomes the near plane, so there's no need to compute the interpolation
// of W, and no need for recomputing 1/W, since it'll just be 1/near_plane
// which can be precomputed in this function
void SetClipVolume(float min_x, float max_x, float min_y, float max_y, float min_z, float max_z)
{
	gMinClipX = min_x;
	gMinClipY = min_y;
	gMinClipZ = min_z;
	gMaxClipX = max_x;
	gMaxClipY = max_y;
	gMaxClipZ = max_z;

	gMinClipZInv = 1.0f/gMinClipZ;
	gMaxClipZInv = 1.0f/gMaxClipZ;
}

// Cohen-Sutherland clipping algorithm outcodes
// bit 1: left of window
// bit 2: right of window
// bit 3: below window
// bit 4: above window
// bit 5: behind front plane
// bit 6: beyond back plane
// A picture of the lower 4 bits and the 2D viewport
//         |      |
//    1001 | 1000 | 1010
//      9  |   8  |   a
//   ------+------+------
//         |      |
//    0001 | 0000 | 0010
//      1  |   0  |   2
//   ------+------+------
//         |      |
//    0101 | 0100 | 0110
//      5  |   4  |   6
#ifdef USE_ASM
__declspec(naked) unsigned long ComputeOutcode(const GrVertex *v)
{
	__asm // 23 cycles
	{
		mov			edx, [esp + 4] // edx = v
		push		ebx

		push		edi
		xor			eax, eax // eax = outcodes

		xor			ecx, ecx // ecx = sign mask
		push		esi

		mov			ebx, (GrVertex)[edx].z // ebx = z
		mov			esi, [gMaxClipZ]

		mov			edi, [gMinClipZ]
		cmp			esi, ebx // maxz - z

		adc			eax, eax
		cmp			ebx, edi // z - minz

		adc			eax, eax
		add			ebx, ebx // carry if ebx = z < 0

		adc			ecx, ecx
		mov			esi, [gMaxClipY]

		mov			ebx, (GrVertex)[edx].y // ebx = y
		mov			edi, [gMinClipY]

		cmp			esi, ebx // maxy - y
		mov			esi, [gMaxClipX]

		adc			eax, eax
		cmp			ebx, edi // y - miny

		adc			eax, eax
		add			ebx, ebx // carry if ebx = y < 0

		adc			ecx, ecx
		mov			ebx, (GrVertex)[edx].x // ebx = x

		mov			edi, [gMinClipX]
		cmp			esi, ebx // maxx - x

		adc			eax, eax
		cmp			ebx, edi // x - minx

		adc			eax, eax
		add			ebx, ebx // carry if ebx = x < 0

		adc			ecx, ecx
		mov			edx, (GrVertex)[edx].z // edx = z

		pop			esi
		pop			edi

		sar			edx, 31 // sign mask of z
		mov			ecx, [gSignMaskTable + 4*ecx] // sign mask of zzyyxx

		xor			eax, ecx // outcodes if z > 0
		and			edx, 0xf

		xor			eax, edx
		pop			ebx

		ret
	}
}
#else // USE_ASM
unsigned long ComputeOutcode(const GrVertex *v)
{
	unsigned long res;

	res = 0;

	if (v->x < gMinClipX)
		res |= 0x1;
	else if (gMaxClipX < v->x)
		res |= 0x2;

	if (v->y < gMinClipY)
		res |= 0x4;
	else if (gMaxClipY < v->y)
		res |= 0x8;

	if (v->z < gMinClipZ)
		res |= 0x10;
	else if (gMaxClipZ < v->z)
		res |= 0x20;

	// flip the x, y outcodes if z < 0
	if (v->z < 0)
	{
		res ^= 0xf;
	}

	return res;
}
#endif // USE_ASM

static void Interpolate(GrVertex *new_v, float t, const GrVertex *v0, const GrVertex *v1)
{
	new_v->x = v0->x + t*(v1->x - v0->x);
	new_v->y = v0->y + t*(v1->y - v0->y);
	new_v->oow = v0->oow + t*(v1->oow - v0->oow);
	new_v->a = v0->a + t*(v1->a - v0->a);
	new_v->r = v0->r + t*(v1->r - v0->r);
	new_v->g = v0->g + t*(v1->g - v0->g);
	new_v->b = v0->b + t*(v1->b - v0->b);
	new_v->tmuvtx[0].sow = v0->tmuvtx[0].sow + t*(v1->tmuvtx[0].sow - v0->tmuvtx[0].sow);
	new_v->tmuvtx[0].tow = v0->tmuvtx[0].tow + t*(v1->tmuvtx[0].tow - v0->tmuvtx[0].tow);
	new_v->tmuvtx[1].sow = v0->tmuvtx[1].sow + t*(v1->tmuvtx[1].sow - v0->tmuvtx[1].sow);
	new_v->tmuvtx[1].tow = v0->tmuvtx[1].tow + t*(v1->tmuvtx[1].tow - v0->tmuvtx[1].tow);
}

static int ClipEdgeAgainstPlane(int plane, GrVertex **new_v0, GrVertex **new_v1, const GrVertex *v0, const GrVertex *v1)
{
	int verts_added;
	float t, d0, d1, delta;
	GrVertex vert0, vert1, *new_v;
	const GrVertex *src_v0, *src_v1;
	float new_z, new_z_inv;

	switch (plane)
	{
		case 0: // left
			d0 = v0->x - gMinClipX;
			d1 = v1->x - gMinClipX;
			delta = v1->x - v0->x;
			break;

		case 1: // right
			d0 = gMaxClipX - v0->x;
			d1 = gMaxClipX - v1->x;
			delta = v0->x - v1->x;
			break;

		case 2: // bottom
			d0 = v0->y - gMinClipY;
			d1 = v1->y - gMinClipY;
			delta = v1->y - v0->y;
			break;

		case 3: // top
			d0 = gMaxClipY - v0->y;
			d1 = gMaxClipY - v1->y;
			delta = v0->y - v1->y;
			break;

		case 4: // front
			d0 = v0->z - gMinClipZ;
			d1 = v1->z - gMinClipZ;
			delta = v1->z - v0->z;
			new_z = gMinClipZ;
			new_z_inv = gMinClipZInv;
			break;

		case 5: // back
			d0 = gMaxClipZ - v0->z;
			d1 = gMaxClipZ - v1->z;
			delta = v0->z - v1->z;
			new_z = gMaxClipZ;
			new_z_inv = gMaxClipZInv;
			break;
	}

	if (*(int *)&d0 >= 0 && *(int *)&d1 >= 0) // trivial accept
	{
		*new_v0 = (GrVertex *)v0;
		verts_added = 1;
	}
	else if (*(int *)&d0 < 0 && *(int *)&d1 < 0) // trivial reject
	{
		verts_added = 0;
	}
	else // intersect
	{
		t = 1.0f/delta;

		if (plane > 3) // z planes
		{
			vert0 = *v0;
			vert1 = *v1;

			src_v0 = &vert0;
			src_v1 = &vert1;

			// undo the 1/w divide (back to homogeneous space)
			vert0.x *= vert0.z;
			vert0.y *= vert0.z;
			vert0.tmuvtx[0].sow *= vert0.z;
			vert0.tmuvtx[0].tow *= vert0.z;
			vert0.tmuvtx[1].sow *= vert0.z;
			vert0.tmuvtx[1].tow *= vert0.z;

			vert1.x *= vert1.z;
			vert1.y *= vert1.z;
			vert1.tmuvtx[0].sow *= vert1.z;
			vert1.tmuvtx[0].tow *= vert1.z;
			vert1.tmuvtx[1].sow *= vert1.z;
			vert1.tmuvtx[1].tow *= vert1.z;
		}
		else
		{
			src_v0 = v0;
			src_v1 = v1;
		}

		if (*(int *)&d0 < 0)
		{
			t *= d0;
			*new_v0 = gCurrFreeClippedVert++;
			Interpolate(*new_v0, -t, src_v0, src_v1);
			verts_added = 1;
			new_v = *new_v0;
		}
		else
		{
			t *= d1;
			*new_v0 = (GrVertex *)v0;
			*new_v1 = gCurrFreeClippedVert++;
			Interpolate(*new_v1, t, src_v1, src_v0);
			verts_added = 2;
			new_v = *new_v1;
		}

		if (plane > 3) // z planes
		{
			// redo the 1/w divide with the new w (back to screen space)
			new_v->z = new_z;
			new_v->oow = new_z_inv;
			new_v->x *= new_v->oow;
			new_v->y *= new_v->oow;
			new_v->tmuvtx[0].sow *= new_v->oow;
			new_v->tmuvtx[0].tow *= new_v->oow;
			new_v->tmuvtx[1].sow *= new_v->oow;
			new_v->tmuvtx[1].tow *= new_v->oow;
		}
	}

	return verts_added;
}

int ClipAndDrawTriangle(const GrVertex *v0, const GrVertex *v1, const GrVertex *v2)
{
	static GrVertex *clipped_vert_ptrs[2][MAX_INTERMIDIATE_VERTS];
	GrVertex **src, **dst;
	int i, toggle, plane, verts_added, num_verts, num_clipped_verts;

	gCurrFreeClippedVert = &gClippedVerts[0];

	clipped_vert_ptrs[0][0] = (GrVertex *)v0;
	clipped_vert_ptrs[0][1] = (GrVertex *)v1;
	clipped_vert_ptrs[0][2] = (GrVertex *)v2;
	num_clipped_verts = 3;

	// clip the poly against each of the 6 clip planes
	// clip against the z planes first to get rid of any negative Zs
	for (plane=5, toggle=0; plane>=0; plane--, toggle^=1)
	{
		// toggle back and forth between src and dst
		// filling in dst from src, and then next iteration
		// using the newly filled in dst as src...
		src = &clipped_vert_ptrs[toggle  ][0];
		dst = &clipped_vert_ptrs[toggle^1][0];

		num_verts = 0;
		for (i=num_clipped_verts-1; i>0; i--)
		{
			verts_added = ClipEdgeAgainstPlane(plane, dst, dst+1, *src, *(src+1));
			num_verts += verts_added;
			dst += verts_added;
			src++;
		}

		// we can terminate if no verts were added because the
		// last plane intersection could at most add 2 more verts which doesn't
		// make a polygon
		if (!num_verts)
		{
			return 0;
		}

		// do the last step separately to avoid having to wrap
		num_verts += ClipEdgeAgainstPlane(plane, dst, dst+1, *src, *(src-(num_clipped_verts-1)));

		num_clipped_verts = num_verts;
	}

	// draw clipped poly
#ifdef USE_GLIDE3
	grDrawVertexArray(GR_POLYGON, num_clipped_verts, &clipped_vert_ptrs[0][0]);
#else
	src = &clipped_vert_ptrs[0][0];
	for (i=1; i<num_clipped_verts-1; i++)
	{
		grDrawTriangle(src[0], src[i], src[i+1]);
	}
#endif // USE_GLIDE3

	// to get the number of triangles (num triangles = num verts - 2 for a polygon)
	return num_clipped_verts - 2;
}

int ClipAndDrawLine(const GrVertex *v0, const GrVertex *v1)
{
	GrVertex *clipped_verts[2];
	int plane;

	gCurrFreeClippedVert = &gClippedVerts[0];

	clipped_verts[0] = (GrVertex *)v0;
	clipped_verts[1] = (GrVertex *)v1;

	for (plane=5; plane>=0; plane--)
	{
		if (!ClipEdgeAgainstPlane(plane, &clipped_verts[0], &clipped_verts[1], clipped_verts[0], clipped_verts[1]))
		{
			return 0;
		}
	}

	grDrawLine(clipped_verts[0], clipped_verts[1]);

	return 1;
}

int ClipPolyVerts(GrVertex *clipped_verts, int src_num_verts, const GrVertex *src_verts)
{
	static GrVertex *clipped_vert_ptrs[2][MAX_INTERMIDIATE_VERTS];
	GrVertex **src, **dst;
	int i, toggle, plane, verts_added, num_verts, num_clipped_verts;

	gCurrFreeClippedVert = &gClippedVerts[0];

	for (i=0; i<src_num_verts; i++)
	{
		clipped_vert_ptrs[0][i] = (GrVertex *)&src_verts[i];
	}
	num_clipped_verts = src_num_verts;

	// clip the poly against each of the 6 clip planes
	// clip against the z planes first to get rid of any negative Zs
	for (plane=5, toggle=0; plane>=0; plane--, toggle^=1)
	{
		// toggle back and forth between src and dst
		// filling in dst from src, and then next iteration
		// using the newly filled in dst as src...
		src = &clipped_vert_ptrs[toggle  ][0];
		dst = &clipped_vert_ptrs[toggle^1][0];

		num_verts = 0;
		for (i=num_clipped_verts-1; i>0; i--)
		{
			verts_added = ClipEdgeAgainstPlane(plane, dst, dst+1, *src, *(src+1));
			num_verts += verts_added;
			dst += verts_added;
			src++;
		}

		// we can terminate if no verts were added because the
		// last plane intersection could at most add 2 more verts which doesn't
		// make a polygon
		if (!num_verts)
		{
			return 0;
		}

		// do the last step separately to avoid having to wrap
		num_verts += ClipEdgeAgainstPlane(plane, dst, dst+1, *src, *(src-(num_clipped_verts-1)));

		num_clipped_verts = num_verts;
	}

	// copy the vertices
	for (i=0; i<num_clipped_verts; i++)
	{
		clipped_verts[i] = *clipped_vert_ptrs[0][i];
	}

	return num_clipped_verts;
}

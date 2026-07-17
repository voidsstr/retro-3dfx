
/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
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
**
** $Revision: 4$ 
** $Date: 10/11/00 7:34:42 PM$ 
**
*/
//----- Include file for SDL.EXE
//
#ifndef ulong
#define ulong	unsigned long
#endif

#ifndef uint
#define uint	unsigned int
#endif

//----- file types
#define	SDL		1				// Alias "scene description language" file

#define TIF		16				// 2D formats
#define PIX		32
#define TGA		64
#define SGI		128

//----- misc defs
#define DEFAULT_RED		(0.0f)		// default RGB value (for texture errors)
#define DEFAULT_GREEN	(0.588f)
#define DEFAULT_BLUE	(1.0f)

#define	SDL_FC		1				// flat color
#define SDL_FT		2				// flat texture
#define SDL_GC		4				// gouraud color (not supported)
#define SDL_GT		5				// gouraud texture

//----- SDL parsing structures

typedef struct {				// texture entries

	char	texturename[32];	// internal SDL name
	char	filename[32];		// image file name
	short	xsize, ysize;		// size of image
	float	urep, vrep;			// uv repeat values
	float	uoff, voff;	   		// uv offset values
	float	r0, g0, b0;			// rgbout value (used instead of texture when active = 0)
	short	active;				// 0 = off, 1 = on
	short	type;				// type (see above)

} SDLtexture;

typedef struct {				// shader entries

	char	shadername[32];		// internal SDL name
	float	r0,g0,b0;	   		// color, if not textured
	float	diffuse;			// diffuse value
	short	texture;	   		// texture index, if mapped (-1 if not)
	short	refnum;				// -1 = ! used by anybody, else = final shader #

}	SDLshader;

typedef struct {				// vertice index list

	uint	vi;					// vertice index
	uint	ti;					// UV index

} SDLvertice;

//----- JAY format structures

typedef struct {

	char	objname[34];		// object name (up to 32 characters), NULL term
	ulong	startpoly;			// first poly (index into vertice list)
	ulong	numpolys;			// number of polys in object
	ulong	shader;				// shader number (index into shader list)
	ulong	parent;				// parent object # (index into object list)
	ulong	pivot;				// parent vertice (index into vertice list)
	ulong	startv;				// if object is frame joint, first vert to transform
	ulong	numv;				// " ", # of verts to transform (other verts in obj transformed by other objs)
	float	rx, ry, rz;			// initial obj rotation
	float	tx, ty, tz;			// " " translation
	float	sx, sy, sz;			// " " scale
								
} JOBJ;							// .JAY object

typedef struct {

	ulong	startpoly;			// index into poly list
	ulong	numpolys;			// # polys
	ulong	texture;			// index into texture list

} QOBJ;							// Quick object
								
typedef struct {				
								
	float	vx, vy, vz;			// vertice coord
	float	u, v;				// texture coord
								
} JVERT;						// .JAY vertice
								
typedef struct {				
								
	float	nx, ny, nz;			// vertex normal
								
} JNORM;						// .JAY normal

typedef struct {

	uint	nv;					// # of vertices in poly (3 or 4)
	ulong	p0, p1, p2, p3;		// vertex #s (index into vertice list)
	ulong	n0, n1, n2, n3;		// normal #s (index into normal list)

} JPOLY;						// .JAY format polygon

typedef struct {

	ulong	p0, p1, p2;			// vertex #s

} JTRI;

typedef struct {

	char	type[4];			// type of texture, zero term (FC, FT, GC, GT)

	char	name[258];			// texture filename, zero term
	uint	x, y;				// texture xy size (powers of 2)
	
	float	r0, g0, b0;			// rgb values (0.0 .. 1.0) (flat color goes here)
	float	r1, g1, b1;			// additional rgb values for gouraud colored polys
	float	r2, g2, b2;
	float	r3, g3, b3;

	float	diffuse;			// diffuse setting

} JTEX;							// .JAY TEXTURE

typedef struct {

	char	filename[256];		// texture file name (or '\0' if no texture)
	float	r0, g0, b0;			// RGB color
	float	diffuse;			// diffuse value
	short	type;				// type (see above)

} QTEX;



#if 0

//==================== JBY Binary File Format

unsigned long	numObjs;		// block counts
unsigned long	numVerts;
unsigned long	numNorms;
unsigned long	numPolys;
unsigned long	numTextures;

atrMesh	objects					// ATB format objects (load with atrMeshLoad)

char	texturefile[256];		// texture filespec, ext = .3df

short	type;					// texture type (see above)
float	r0, g0, b0;				// RGB values
float	diffuse;				// diffuse value


//==================== JAY ASCII File Format

//===== Version
//
//  2.1:  Added 3Dfx friendly hacks (see 2.0 for standard format)
//
Version   JAY2.1
Option    3Dfx
Source    c:\alias\sdls\test.sdl
Stamp     Tue Feb 20 19:22:17 1996

//===== Block Counts
//
Objects   1
Vertices  4
Normals   1
Polygons  2
Textures  1

//===== Objects: #  "ObjName"  StartPoly NumPolys  Texture#  ParentObj ParentVert  StartVert NumVerts  RotX Y Z  TransX Y Z  ScaleX Y Z
//
//  StartVert, NumVerts	= Used for frame joints only (refs transformable verts)
//
0   "Flat Square"   0 2   0   0 0   0 0   0.0 0.0 0.0   0.0 0.0 0.0   0.0 0.0 0.0   0

//===== Vertices:  #  x y z  u v
//
//  2.1:  3Dfx:  associates one UV per vertice, listed here rather than with polys.
//        3Dfx:  UVs conform to 3Dfx range standard for non-square textures.
//
0   0.0 0.0 0.0   0.0 0.0
1   10.0 0.0 0.0   1.0 0.0
2   10.0 10.0 0.0   1.0 1.0
3   0.0 10.0 0.0   0.0 1.0

//===== Normals:  #  x y z
//
0   0.0 0.0 1.0

//===== Polygons:  #  NumVerts  Vertices(4)  Normals(4)
//
//  2.1:  3Dfx:  UVs moved to vertice list (one per vertice)
//		  3Dfx:  Texture # moved to object
//
0   3   0 1 2 0   0 0 0 0
1   3   0 2 3 0   0 0 0 0

//===== Textures:
//
// Flat Color       FC  rgb0  diffuse
// Flat Texture     FT  "FileName"  Xpixels Ypixels  diffuse
// Gouraud Color    GC  rgb0  rgb1 rgb2  rgb3  diffuse
// Gouraud Texture  GT  "FileName"  Xpixels Ypixels  rgb0  rgb1  rgb2  rgb3  diffuse
//
0   FT   "box side.tga"   128 128   1.0


#endif

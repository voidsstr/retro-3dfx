#ifndef ANIM_OBJ_H__
#define ANIM_OBJ_H__

#include <glide.h>

typedef struct
{
  float x, y, z;
} AoVert3;

/*
 * Information needed for each vertex.
 * When converting to a AoObject, the only members that are
 * filled in are the "local_vert", "s", and "t".
 * The others are placeholders for the user to use for cache-coherency reasons.
 */
typedef struct
{
  AoVert3	local_vert;
  AoVert3	view_vert;
  GrVertex	screen_vert;
  float		s, t;
} AoVertex;

typedef struct
{
  
} AoObject;

#endif /* ANIM_OBJ_H__ */

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
** $Date: 10/11/00 7:34:46 PM$ 
**
*/

/*
**  vqtoats
**  This reads files in the format of verticies and quadralaterals
**  as used by GMT.  Some example data follows:
**  
** # total polys = 1302, verts=5208
** v 0 -0.1509 0.1523 0.0445    -0.869187 0.489918 0.067036
** v 1 -0.1469 0.1424 0.0942    -0.85616 0.483804 0.18145
** v 2 -0.1415 0.1606 0.0709    -0.489908 0.844071 0.218024
** q 0 0 1 2
** v 3 -0.0801 -0.0146 0.4499    -0.433365 0.575817 0.693275
** v 4 -0.0701 -0.0079 0.4499    0.289442 0.812862 0.50545
** v 5 -0.0868 0.0171 0.4081    0.318097 0.849433 0.421045
** q 3 3 4 5
** 
** Output is a shape node with a single tri set and a single material
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atscenep.h>

#define VQ_DEFAULT_EMAP "ra.3df"
static char *ext = "vq";
static char *converterName = "atsLoadFromVQ";

/* objInit scans the entire file to count vertices and meshes,
 * allocates the space for them, and initializes some defaults.
 * Space for the TriSets is referred-to by a pointer in the 
 * object, but we must allocate space for the vertices.
 * This function returns the number of vertices.
 */

static FxU32 VQ_objInit(const char *name) {
  FILE *inf;
  char buf[1024];

  FxU32 vcount=0;                            // count of vertices
  FxU32 mcount=0;                            // count of quads 
                                             // (will become trimeshes)

  inf = fopen(name, "rb");
  if (!inf){
    atuError( FXTRUE, "ERROR: unable to open %s\n", name);
    return 0;
  }

  while(fgets(buf,sizeof(buf), inf)){
    switch (buf[0]){
    case 'v':
      vcount++;
      break;
    case 'q':
      mcount++;
      break;
    default:
      ;
    }
  }
  fclose(inf);

  printf("\n\t%s has %d vertices and %d quads\n",
         name,vcount,mcount);

  return vcount;
}

static AtsMaterial *setupMaterial(void) {
    AtsMaterial  *mat;
    AtsTexture   *tex;
    AtrImg       *img;

    if( (mat = atsMaterialNew()) == NULL ) {
        atuError(FXFALSE, "%s: Unable to allocate space for the Materials\n", converterName);
        return NULL;
    }

    /* TO DO: Read in the targa and create a texture: TEXUS LIB!!! */

    if (( tex = atsTextureNew()) == NULL ) {
        atuError(FXFALSE, "%s: Unable to allocate space for Material Texture\n", converterName);
        return NULL;
    }

    img = (AtrImg *)atsImageCreateFromFile( VQ_DEFAULT_EMAP );
    if ( img == NULL ) { 
      atuError(FXFALSE, "%s: Unable to load texture %s\n", 
               converterName, VQ_DEFAULT_EMAP);
        atrMaterialSetup(&mat->material, ATR_MAT_GSHADE);
    } else {
        atsTextureAddImage( tex, img );
        atsTextureReset( tex );
        mat->textures[0] = tex;
        mat->material.texture[0] = tex->handle;
        atrMaterialSetup(&mat->material, ATR_MAT_EMAP);
    }

    mat->material.texSClamp[0] = ATR_TEXCLAMP_CLAMP;
    mat->material.texTClamp[0] = ATR_TEXCLAMP_CLAMP;
    mat->material.diffuse.r = 0.1f;
    mat->material.diffuse.g = 0.2f;
    mat->material.diffuse.b = 0.7f;
    mat->material.emissive.r = 0.33f;
    mat->material.emissive.g = 0.33f;
    mat->material.emissive.b = 0.33f;

    return mat;
}

AtsObject *atsLoadFromVQ(const char *filename)
{
    char        buf[1024];
    FILE        *inf;
    AtrTriSet   *tset;
    AtsNode     *shape; 
    AtsMaterial *mat;
    AtmSphere   bsphere; 
    AtrVertex* vertices ;
    FxU32       numVerts;

    if ( atuFileLocate(filename, buf) == NULL ) {
        atuError( FXFALSE, "%s: Not able to open VQ file \"%s\"\n", 
                  converterName, filename );
        return NULL;
    }

    /* Allocate array of vertices large enough to contain complete object */

    if ((numVerts = VQ_objInit(buf)) == 0 ) {
        atuError(FXFALSE, "%s: No geometry\n", converterName);
        return NULL;
    }

    if ((vertices = atrVertexAllocate( numVerts )) == NULL ) {
        atuError(FXFALSE, "%s: could not allocate vertices\n", converterName);
        return NULL;
    }

    if ((inf = fopen( buf, "rb" )) == NULL ) {
        atuError( FXFALSE, "%s: Not able to open VQ file \"%s\"\n", converterName, filename );
        return NULL;
    }

    fgets(buf,sizeof(buf),inf);
    if (strncmp(buf,"# total polys =",15)) {
        atuError(FXFALSE,"%s: bad magic in object file\n", converterName);
        return NULL;
    }

    /*  put all geometry in one triset */

    if ((tset = atsTriSetNew()) == NULL ) {
        atuError(FXFALSE, "%s: could not allocate tri set\n", converterName);
        return NULL;
    }

    atrTriSetBegin(tset);

    while (fgets(buf,sizeof(buf),inf)) {
        if (buf[0] == 'v') {            /* vertex record */
            int iv;
            AtrVertex gv;

            sscanf(buf+1,"%d %g %g %g %g %g %g", 
                   &iv,
                   &gv.x,&gv.y,&gv.z,
                   &gv.i,&gv.j,&gv.k);

            atrVertexAssign(&vertices[iv],&gv);
        }
        else if (buf[0] == 'q') {       /* quad record */
            int v1,v2,v3,v4;

            sscanf(buf+1,"%d %d %d %d",&v1,&v2,&v3,&v4);

            /* Split the quad into two triangles */
            /* Triangle 1 */
            atrTriSetVertex(tset, &vertices[v1]);
            atrTriSetVertex(tset, &vertices[v2]);
            atrTriSetVertex(tset, &vertices[v3]);

            /* Triangle 2 */

            atrTriSetVertex(tset, &vertices[v1]);
            atrTriSetVertex(tset, &vertices[v3]);
            atrTriSetVertex(tset, &vertices[v4]);

        }
        else if (buf[0] == '#') {       /* comment record */
        }
    }

    atrTriSetEnd(tset);

    atrVertexDeallocate(vertices);

    if(!( shape = atsShapeNew())) {
        atuError( FXFALSE, "%s: Not able to allocate shape\n", converterName );
        return NULL;
    }

    if ((mat = setupMaterial()) == NULL ) {
        atuError( FXFALSE, "%s: Not able to allocate shape\n", converterName );
        return NULL;
    }
 
    atsShapeAddPart(shape, (AtsObject *)tset, mat);

    atsComputeBSphere(shape, &bsphere);
   
    atsNodeBSphere(shape, &bsphere);

    atsNodeName(shape, filename);

    fclose(inf);

    return shape;
}


void atsInitVQ(void) {
}

void atsTermVQ(void) {
}

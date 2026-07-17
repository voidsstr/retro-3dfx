/******************************************************************************
 **                                                                                                                                                      **
 **     Module:         RAVE_TC.h                                                                                                **
 **                                                                                                                                              **
 **     Purpose:        Header file for additional RAVE API for texture compositing                      **
 **                                                                                                                                              **
 **               Note: This is a *preliminary* file. the final versions of these data
 **                       structures will change.
 **                       
 **     Copyright (C) 1994-96 Apple Computer, Inc.  All rights reserved.          
 **             OpenGL is a trademark of Silicon Graphics, Inc.                                         **
 **                                                                                                                                              **
 *****************************************************************************/

#ifndef _RAVE_TC_h
#define _RAVE_TC_h

#ifdef __cplusplus
extern "C" {
#endif

/* These are the state variables for setting the textures */
#define kQATag_Texture0         13
#define kQATag_Texture1         17

/* This is the triangle flag indicating that the vertices are of type TQAVTexture_TC */
/* It only applies to drawing textured triangles */
#define kQATriFlags_TC          (1 << 1)

/* This is the first engine specific state variable tag */
/* Use this to set the texture combination mode and other engine specific features */
#define kQATag_3DFXEngineSpecific_Minimum       1000

/* This has the extra fields for the second texture */
typedef struct TQAVTexture_TC
{
        float                           x;              /* X pixel coordinate, 0.0 <= x < width */
        float                           y;              /* Y pixel coordinate, 0.0 <= y < height */
        float                           z;              /* Z coordinate, 0.0 <= z <= 1.0 */
        float                           invW;   /* 1 / w (always required) */
        
        /* rgb are used only when kQATextureOp_Decal is set. a is always required */
        
        float                           r;              /* Red, 0.0 <= r <= 1.0 */
        float                           g;              /* Green, 0.0 <= g <= 1.0 */
        float                           b;              /* Blue, 0.0 <= b <= 1.0 */
        float                           a;              /* Alpha, 0.0 <= a <= 1.0, 1.0 is opaque */

        /* uOverW and vOverW are required by all modes */
        

		  float                           uOverW0; /* u / w */
        float                           vOverW0; /* v / w */
        
        /* kd_r/g/b are used only when kQATextureOp_Modulate is set */
        
        float                           kd_r;   /* Scale factor for texture red, 0.0 <= kd_r */
        float                           kd_g;   /* Scale factor for texture green, 0.0 <= kd_g */
        float                           kd_b;   /* Scale factor for texture blue, 0.0 <= kd_b */
        
        /* ks_r/g/b are used only when kQATextureOp_Highlight is set */
        
        float                           ks_r;   /* Red specular highlight, 0.0 <= ks_r <= 1.0 */
        float                           ks_g;   /* Green specular highlight, 0.0 <= ks_g <= 1.0 */
        float                           ks_b;   /* Blue specular highlight, 0.0 <= ks_b <= 1.0 */
        
        /* This in only required when texture compositing. InvW0 is placed
		   * down here to make this structure still work with standard rave
			* calls. **/

		  float									 invW0;	//Brent, why not just use invW instead?

        float                           invW1;          /* 1 / w (always required) */
        float                           uOverW1;        /* u / w */
        float                           vOverW1;        /* v / w */
        
} TQAVTexture_TC;

#ifdef __cplusplus
}
#endif

#endif /* _RAVE_TC_h */





#ifndef _ddtexfmt_h_
#define _ddtexfmt_h_

/*
** Copyright 1991-1997, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision: 2$
** $Date: 10/11/00 8:02:45 PM$
*/





typedef struct __GLDDrawTextureFormatRec {
    void (*slurpImage)(struct __GLDDrawTextureRec *tex, 
                       struct __GLDDrawMipMapLevelRec *lp);
    void (*slurpSubImage)(struct __GLDDrawTextureRec *tex,
                          struct __GLDDrawMipMapLevelRec *lp,
                          GLint x, GLint y, GLint w, GLint h);
    struct _DDPIXELFORMAT ddFormat;
} __GLDDrawTextureFormat;


/* look up the texture format */
extern __GLDDrawTextureFormat *__glLookupDDrawTextureFormat(GLint format);

/* the texture format structs */
extern __GLDDrawTextureFormat __glDDrawTexFormatLuminance;
extern __GLDDrawTextureFormat __glDDrawTexFormatLuminanceAlpha;
extern __GLDDrawTextureFormat __glDDrawTexFormatRGB;
extern __GLDDrawTextureFormat __glDDrawTexFormatRGB332;
extern __GLDDrawTextureFormat __glDDrawTexFormatRGB5;
extern __GLDDrawTextureFormat __glDDrawTexFormatRGB565;
extern __GLDDrawTextureFormat __glDDrawTexFormatRGBA;
extern __GLDDrawTextureFormat __glDDrawTexFormatRGBA4;
extern __GLDDrawTextureFormat __glDDrawTexFormatARGB4;
extern __GLDDrawTextureFormat __glDDrawTexFormatRGB5_A1;
extern __GLDDrawTextureFormat __glDDrawTexFormatARGB5_1;
extern __GLDDrawTextureFormat __glDDrawTexFormatAlpha;
extern __GLDDrawTextureFormat __glDDrawTexFormatIntensity;
extern __GLDDrawTextureFormat __glDDrawTexFormatColorIndex8;
extern __GLDDrawTextureFormat __glDDrawTexFormatColorIndex16;
extern __GLDDrawTextureFormat __glDDrawTexFormatLuminanceBGRA8;
extern __GLDDrawTextureFormat __glDDrawTexFormatLuminanceAlphaBGRA8;
extern __GLDDrawTextureFormat __glDDrawTexFormatRGB8BGRA8;
extern __GLDDrawTextureFormat __glDDrawTexFormatRGB332BGRA8;
extern __GLDDrawTextureFormat __glDDrawTexFormatRGB5BGRA8;
extern __GLDDrawTextureFormat __glDDrawTexFormatRGB565BGRA8;
extern __GLDDrawTextureFormat __glDDrawTexFormatRGBA8BGRA8;
extern __GLDDrawTextureFormat __glDDrawTexFormatRGBA4BGRA8;
extern __GLDDrawTextureFormat __glDDrawTexFormatARGB4BGRA8;
extern __GLDDrawTextureFormat __glDDrawTexFormatRGB5_A1BGRA8;
extern __GLDDrawTextureFormat __glDDrawTexFormatARGB5_1BGRA8;
extern __GLDDrawTextureFormat __glDDrawTexFormatAlphaBGRA8;
extern __GLDDrawTextureFormat __glDDrawTexFormatIntensityBGRA8;
extern __GLDDrawTextureFormat __glDDrawTexFormatColorIndex8BGRA8;
extern __GLDDrawTextureFormat __glDDrawTexFormatColorIndex16BGRA8;


/* prototypes for the texture formats */

extern void __glDDTexMgrSlurpLuminanceImage(__GLDDrawTexture *tex,
					    __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpLuminanceSubImage(__GLDDrawTexture *tex,
					       __GLDDrawMipMapLevel *lp,
					       GLint x, GLint y, 
					       GLint w, GLint h);

extern void __glDDTexMgrSlurpLuminanceAlphaImage(__GLDDrawTexture *tex,
						 __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpLuminanceAlphaSubImage(__GLDDrawTexture *tex,
						    __GLDDrawMipMapLevel *lp,
						    GLint x, GLint y, 
						    GLint w, GLint h);

extern void __glDDTexMgrSlurpAlphaImage(__GLDDrawTexture *tex,
					__GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpAlphaSubImage(__GLDDrawTexture *tex,
					   __GLDDrawMipMapLevel *lp,
					   GLint x, GLint y,
					   GLint w, GLint h);

extern void __glDDTexMgrSlurpRGBImage(__GLDDrawTexture *tex,
				      __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpRGBSubImage(__GLDDrawTexture *tex,
					 __GLDDrawMipMapLevel *lp,
					 GLint x, GLint y, GLint w, GLint h);

extern void __glDDTexMgrSlurpRGB332Image(__GLDDrawTexture *tex,
					 __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpRGB332SubImage(__GLDDrawTexture *tex,
					    __GLDDrawMipMapLevel *lp,
					    GLint x, GLint y, 
					    GLint w, GLint h);

extern void __glDDTexMgrSlurpRGBA4Image(__GLDDrawTexture *tex,
					__GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpRGBA4SubImage(__GLDDrawTexture *tex,
					   __GLDDrawMipMapLevel *lp,
					   GLint x, GLint y, 
					   GLint w, GLint h);

extern void __glDDTexMgrSlurpARGB4Image(__GLDDrawTexture *tex,
					__GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpARGB4SubImage(__GLDDrawTexture *tex,
					   __GLDDrawMipMapLevel *lp,
					   GLint x, GLint y, GLint w, GLint h);

extern void __glDDTexMgrSlurpRGB5Image(__GLDDrawTexture *tex,
				       __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpRGB5SubImage(__GLDDrawTexture *tex,
					  __GLDDrawMipMapLevel *lp,
					  GLint x, GLint y, GLint w, GLint h);

extern void __glDDTexMgrSlurpRGB565Image(__GLDDrawTexture *tex,
					 __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpRGB565SubImage(__GLDDrawTexture *tex,
					    __GLDDrawMipMapLevel *lp,
					    GLint x, GLint y, 
					    GLint w, GLint h);

extern void __glDDTexMgrSlurpRGBAImage(__GLDDrawTexture *tex,
				       __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpRGBASubImage(__GLDDrawTexture *tex,
					  __GLDDrawMipMapLevel *lp,
					  GLint x, GLint y, 
					  GLint w, GLint h);


extern void __glDDTexMgrSlurpRGBA5_1Image(__GLDDrawTexture *tex,
					  __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpRGBA5_1SubImage(__GLDDrawTexture *tex,
					     __GLDDrawMipMapLevel *lp,
					     GLint x, GLint y, 
					     GLint w, GLint h);

extern void __glDDTexMgrSlurpARGBA_1Image(__GLDDrawTexture *tex,
					  __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpARGB5_1SubImage(__GLDDrawTexture *tex,
					     __GLDDrawMipMapLevel *lp,
					     GLint x, GLint y, 
					     GLint w, GLint h);

extern void __glDDTexMgrSlurpIntensityImage(__GLDDrawTexture *tex,
					    __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpIntensitySubImage(__GLDDrawTexture *tex,
					       __GLDDrawMipMapLevel *lp,
					       GLint x, GLint y, 
					       GLint w, GLint h);

extern void __glDDTexMgrSlurpColorIndex8Image(__GLDDrawTexture *tex,
					      __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpColorIndex8SubImage(__GLDDrawTexture *tex,
						 __GLDDrawMipMapLevel *lp,
						 GLint x, GLint y, 
						 GLint w, GLint h);

extern void __glDDTexMgrSlurpColorIndex16Image(__GLDDrawTexture *tex,
					       __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpColorIndex16SubImage(__GLDDrawTexture *tex,
						  __GLDDrawMipMapLevel *lp,
						  GLint x, GLint y, 
						  GLint w, GLint h);

extern void __glDDTexMgrSlurpLuminanceImageBGRA8(__GLDDrawTexture *tex,
						 __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpLuminanceSubImageBGRA8(__GLDDrawTexture *tex,
						    __GLDDrawMipMapLevel *lp,
						    GLint x, GLint y, 
						    GLint w, GLint h);

extern void __glDDTexMgrSlurpLuminanceAlphaImageBGRA8(__GLDDrawTexture *tex,
						      __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpLuminanceAlphaSubImageBGRA8(__GLDDrawTexture *tex,
							 __GLDDrawMipMapLevel *lp,
							 GLint x, GLint y, 
							 GLint w, GLint h);

extern void __glDDTexMgrSlurpAlphaImageBGRA8(__GLDDrawTexture *tex,
					     __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpAlphaSubImageBGRA8(__GLDDrawTexture *tex,
						__GLDDrawMipMapLevel *lp,
						GLint x, GLint y, 
						GLint w, GLint h);

extern void __glDDTexMgrSlurpRGBImageBGRA8(__GLDDrawTexture *tex,
					   __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpRGBSubImageBGRA8(__GLDDrawTexture *tex,
					      __GLDDrawMipMapLevel *lp,
					      GLint x, GLint y, 
					      GLint w, GLint h);

extern void __glDDTexMgrSlurpRGB332ImageBGRA8(__GLDDrawTexture *tex,
					      __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpRGB332SubImageBGRA8(__GLDDrawTexture *tex,
						 __GLDDrawMipMapLevel *lp,
						 GLint x, GLint y, 
						 GLint w, GLint h);

extern void __glDDTexMgrSlurpRGBA4ImageBGRA8(__GLDDrawTexture *tex,
					     __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpRGBA4SubImageBGRA8(__GLDDrawTexture *tex,
						__GLDDrawMipMapLevel *lp,
						GLint x, GLint y, 
						GLint w, GLint h);

extern void __glDDTexMgrSlurpARGB4ImageBGRA8(__GLDDrawTexture *tex,
					     __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpARGB4SubImageBGRA8(__GLDDrawTexture *tex,
						__GLDDrawMipMapLevel *lp,
						GLint x, GLint y, 
						GLint w, GLint h);

extern void __glDDTexMgrSlurpRGB5ImageBGRA8(__GLDDrawTexture *tex,
					    __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpRGB5SubImageBGRA8(__GLDDrawTexture *tex,
					       __GLDDrawMipMapLevel *lp,
					       GLint x, GLint y, 
					       GLint w, GLint h);

extern void __glDDTexMgrSlurpRGB565ImageBGRA8(__GLDDrawTexture *tex,
					      __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpRGB565SubImageBGRA8(__GLDDrawTexture *tex,
						 __GLDDrawMipMapLevel *lp,
						 GLint x, GLint y, 
						 GLint w, GLint h);

extern void __glDDTexMgrSlurpRGBAImageBGRA8(__GLDDrawTexture *tex,
					    __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpRGBASubImageBGRA8(__GLDDrawTexture *tex,
					       __GLDDrawMipMapLevel *lp,
					       GLint x, GLint y, 
					       GLint w, GLint h);


extern void __glDDTexMgrSlurpRGBA5_1ImageBGRA8(__GLDDrawTexture *tex,
					       __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpRGBA5_1SubImageBGRA8(__GLDDrawTexture *tex,
						  __GLDDrawMipMapLevel *lp,
						  GLint x, GLint y, 
						  GLint w, GLint h);

extern void __glDDTexMgrSlurpARGBA_1ImageBGRA8(__GLDDrawTexture *tex,
					       __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpARGB5_1SubImageBGRA8(__GLDDrawTexture *tex,
						  __GLDDrawMipMapLevel *lp,
						  GLint x, GLint y, 
						  GLint w, GLint h);

extern void __glDDTexMgrSlurpIntensityImageBGRA8(__GLDDrawTexture *tex,
						 __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpIntensitySubImageBGRA8(__GLDDrawTexture *tex,
						    __GLDDrawMipMapLevel *lp,
						    GLint x, GLint y, 
						    GLint w, GLint h);

extern void __glDDTexMgrSlurpColorIndex8ImageBGRA8(__GLDDrawTexture *tex,
						   __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpColorIndex8SubImageBGRA8(__GLDDrawTexture *tex,
						      __GLDDrawMipMapLevel *lp,
						      GLint x, GLint y, 
						      GLint w, GLint h);

extern void __glDDTexMgrSlurpColorIndex16ImageBGRA8(__GLDDrawTexture *tex,
						    __GLDDrawMipMapLevel *lp);
extern void __glDDTexMgrSlurpColorIndex16SubImageBGRA8(__GLDDrawTexture *tex,
						       __GLDDrawMipMapLevel *lp,
						       GLint x, GLint y, 
						       GLint w, GLint h);

#endif /* _ddtexfmt_h_ */


#ifndef _wglglide_h_
#define _wglglide_h_

GLboolean __wglGlideSwapBuffers(__WGLdrawablePrivate *wglPriv);
GLint  __wglGlideGetDisplayMasks(GLint *rMask, GLint *gMask, GLint *bMask);
GLvoid __wglGlideUpdateDrawablePalette(__WGLdrawablePrivate *wglPriv);
GLboolean __wglGlideOpen(GLvoid);
GLboolean __wglGlideClose(GLvoid);
GLvoid __wglGlideUpdateClipList(__WGLdrawablePrivate *wglPriv, 
			 LPDIRECTDRAWCLIPPER lpClipper);
GLvoid __wglInitGlide(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv,
	       GLint bits, __GLbufFallbackInitFn back);
GLvoid __wglInitGlideDepth(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv,
	       GLint bits, __GLbufFallbackInitFn back);
GLvoid __wglGlideInitDrawable(__WGLdrawablePrivate *wglPriv, 
					__GLcontextModes *modes);
LRESULT	CALLBACK __wglSstMonitorWindowChanges(int code, WPARAM wParam, 
							LPARAM lParam);
#endif /* _wglglide_h_ */


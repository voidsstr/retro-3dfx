#ifndef __GL_DRIVER_H__
#define __GL_DRIVER_H__


/* buffer mask values */

#define __GL_ALLOCATE_HW_TEXTURE           0x80000000
#define __GL_ALLOCATE_FASTHW_TEXTURE       0x40000000

struct __glDeviceStruct {
    unsigned bufferFlags; /* Tells WGL where to allocate buffers */
    unsigned textureFlags; /* Tells texmgr where to allocate textures */
    const struct __GLtextureFormatRec *(*lookupTextureFormat)(GLenum, GLenum*);
         
    /* Called from DLLMain/wgl  */
    int (*devProcessAttach)(int threadIdx);  // Maps registers, etc
    int (*devProcessDetach)(int threadIdx);  // Unmaps registers, etc
    int (*devThreadAttach)(int threadIdx);   // Registers the thread
    int (*devThreadDetach)(int threadIdx);   // Unregisters the thread
    void (*devSignal)(int signo);             // Cleans up after an 
                                              // abnormal exit, in case 
                                              // noone else has.

    /* functions related to pixel formats */
    /* describe a pixelformat index */
    int (*devGetPixelFormat)(struct tagPIXELFORMATDESCRIPTOR *pfd,
			     int iPixelFormat);
    /* called when depth changes, the device changes, or the driver changes */
    void (*devInvalidatePixelFormatList)();

    /* (global and) per-context texture manager initialization */
    void (*devInitTextureManager)(__GLcontext *gc);

    /* create a hwcx */
    __GLcontext *(*devCreateContext)(__GLimports *imports, __GLcontextModes *modes);

    /* initialize the drawable buffer structures. Called during makecurrent time  */
    void (*devFBInitDrawable)(struct __WGLdrawablePrivateRec *, __GLcontextModes *);
    void (*devDIBInitDrawable)(struct __WGLdrawablePrivateRec *, __GLcontextModes *);

    /* Do we need this if we have GetPixelFormat List? */
    int  (*devGetDisplayMasks)(int *, int *, int *);

    /* other routines that deal with the buffers */
    void (*freeBuffers)(struct __WGLdrawablePrivateRec *);
    void (*updatePalette)(struct __WGLdrawablePrivateRec *);
};

extern struct __glDeviceStruct *__glDevice;

#endif

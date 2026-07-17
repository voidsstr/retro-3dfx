LPDIRECTDRAW            lpDD;           // DirectDraw object
LPDIRECTDRAWSURFACE     lpDDSPrimary;   // DirectDraw primary surface
LPDIRECTDRAWSURFACE     lpDDSBack;      // DirectDraw back surface
 /*
  * create the main DirectDraw object
  */
 ddrval = DirectDrawCreate( NULL, &lpDD, NULL );

// Create the framebuffer.  In the window case, bpp is ignored.

        DDSURFACEDESC ddsd;
        HRESULT LastError;

        memset(&ddsd,0,sizeof(DDSURFACEDESC));
        ddsd.dwSize = sizeof(DDSURFACEDESC);
        ddsd.dwFlags = DDSD_CAPS;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
         
        LastError = d3dappi.lpDD->lpVtbl->CreateSurface(d3dappi.lpDD,
                                        &ddsd, &d3dappi.lpFrontBuffer, NULL);
        if(LastError != DD_OK ) {
            atuError(FXTRUE, "Could not create surface\n");
        }

        
        ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
        ddsd.dwWidth = w;
        ddsd.dwHeight = h;
        ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
        if (bIsHardware)
            ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
        else
            ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
        LastError = d3dappi.lpDD->lpVtbl->CreateSurface(d3dappi.lpDD, &ddsd,
                                             &lpBackBuffer, NULL);


    
// use mipmap.c as starting point to draw pixels

// Show back buffer



int NumFrontRects, NumBufferRects, i;
RECT front[D3DAPP_MAXCLEARRECTS];
RECT buffer[D3DAPP_MAXCLEARRECTS];
/*
 * Set the rectangle to blt from the back to front bufer
 */
if (flags & D3DAPP_SHOWALL) {
    /*
     * Set to entire client window
     */
    NumBufferRects = 1;
    SetRect(&buffer[0], 0, 0, d3dappi.szClient.cx,
            d3dappi.szClient.cy);
    SetRect(&front[0],
            d3dappi.pClientOnPrimary.x, d3dappi.pClientOnPrimary.y,
            d3dappi.szClient.cx + d3dappi.pClientOnPrimary.x,
            d3dappi.szClient.cy + d3dappi.pClientOnPrimary.y);



       /*
        * Blt the list of rectangles from the back to front buffer
        */
       for (i = 0; i < NumBufferRects; i++) {
           LastError =
                   d3dappi.lpFrontBuffer->lpVtbl->Blt(d3dappi.lpFrontBuffer,
                                            &front[i], d3dappi.lpBackBuffer,
                                            &buffer[i], DDBLT_WAIT, NULL);
           if (LastError == DDERR_SURFACELOST) {
               d3dappi.lpFrontBuffer->lpVtbl->Restore(d3dappi.lpFrontBuffer);
               d3dappi.lpBackBuffer->lpVtbl->Restore(d3dappi.lpBackBuffer);
               D3DAppIClearBuffers();
           } else if (LastError != DD_OK) {
               D3DAppISetErrorString("Blt of back buffer to front buffer faile
\n%s", D3DAppErrorToString(LastError));
               return FALSE;
           }
       }
       /*
        * The back buffer's dirty rectangles are now also the client's
        */
       D3DAppICopyRectList(&NumDirtyClientRects, DirtyClient,
                           NumDirtyBackRects, DirtyBack);

#define USE_EVENT_TRANSFER		// use events instead of irq
//#define USE_VMM_CALLBACK
//#define USE_VMM_CALLBACK_IRQ		// must have USE_VMM_CALLBACK also defined

#define MAX_TRANSFERS 20
typedef struct _TRANSFER_DATA
{
  DWORD			 dwFlags;
  DWORD          dwTransferID;
  PMDL	         pMDL;
  DDSURFACEDATA *lpSurfaceData;
  DWORD			 dwStartLine;
  DWORD			 dwEndLine;
  DWORD          dwTransferFlags;
  DWORD          EventHandle;
  BOOL			 bPolarity;
} TRANSFER_DATA,*PTRANSFER_DATA;

typedef struct _KMTVDATA
{             
    DWORD         IRQCallback;
    DWORD         dwIRQSources;
    DWORD         Context;
    BYTE          bPreviousSurface;
    BYTE          bCurrentSurface;
    BYTE          bSkipNextField;
    PDEVTABLE     pDev;
    DWORD         dwTransferID;
    DWORD         dwVBITransferID;
    DWORD         dwVBILines;
    DWORD         dwVideoOffset;
    int 		  bInitialized;
    DWORD		  Num;
    BYTE		  fTransferFull;
    DWORD		  dwCurrentTransfer;
    DWORD         dwQueuedTransfers;
    DWORD		  dwBusMasterTransfers;
    DWORD		  dwActive;
    DWORD		  dwFieldCount;
	BOOL		  bReportForVideo;
	DWORD		  dwFirstFieldBuffer;
	KMVTBUFF *	  pKMVTbuff; 
    TRANSFER_DATA Transfer[MAX_TRANSFERS];
} KMTVDATA, *PKMTVDATA;


#define TRANSFER_READY		 0x0
#define TRANSFER_COMPLETE	 0x1
#define TRANSFER_IN_PROGRESS 0x2
#define TRANSFER_QUEUED		 0x04

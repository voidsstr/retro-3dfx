/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
** File name: dx7todx8.h
**
** Description: Allows the driver to still use the old W2KDDK.
**				In order to move to a common code base for Direct X under both both Win9x 
**				and Win2K Microsoft has renamed quite a number of D3D structures in the DX8 DDK. 
**				To make the transition from Dx7 to Dx8 smoother this file has been created. 
**				This file effectively maps the new structure names to the old names and allows 
**				the driver to build using the DX7 headers from the W2KDDK.
**
** $Revision: 2$
** $Date: 10/25/00 4:41:37 AM$
**
** $Log:
** $ 
*/

#ifndef __DX7TODX8_H__
#define __DX7TODX8_H__


#define PFND3DPARSEUNKNOWNCOMMAND					PFND3DNTPARSEUNKNOWNCOMMAND	
#define D3DHAL_D3DEXTENDEDCAPS						D3DNTHAL_D3DEXTENDEDCAPS 


#define D3DHAL_TSS_MAXSTAGES          				D3DNTHAL_TSS_MAXSTAGES

typedef struct _D3DNTHAL_DP2RENDERSTATE       		*LPD3DHAL_DP2RENDERSTATE;
typedef struct _D3DNTHAL_DP2TEXTURESTAGESTATE 		*LPD3DHAL_DP2TEXTURESTAGESTATE;
typedef struct _D3DNTHAL_DP2WINFO             		*LPD3DHAL_DP2WINFO;
typedef struct _D3DNTHAL_DP2TRIANGLESTRIP     		*LPD3DHAL_DP2TRIANGLESTRIP;
typedef struct _D3DNTHAL_DP2STARTVERTEX       		*LPD3DHAL_DP2STARTVERTEX;
typedef struct _D3DNTHAL_DP2TRIANGLEFAN_IMM   		*LPD3DHAL_DP2TRIANGLEFAN_IMM;
typedef struct _D3DNTHAL_DP2TRIANGLEFAN       		*LPD3DHAL_DP2TRIANGLEFAN;
typedef struct _D3DNTHAL_DP2POINTS            		*LPD3DHAL_DP2POINTS;
typedef struct _D3DNTHAL_DP2LINELIST          		*LPD3DHAL_DP2LINELIST;
typedef struct _D3DNTHAL_DP2INDEXEDLINELIST   		*LPD3DHAL_DP2INDEXEDLINELIST;
typedef struct _D3DNTHAL_DP2INDEXEDLINESTRIP  		*LPD3DHAL_DP2INDEXEDLINESTRIP;
typedef struct _D3DNTHAL_DP2INDEXEDTRIANGLELIST   	*LPD3DHAL_DP2INDEXEDTRIANGLELIST;
typedef struct _D3DNTHAL_DP2INDEXEDTRIANGLELIST2  	*LPD3DHAL_DP2INDEXEDTRIANGLELIST2;


#endif //__DX7TODX8_H__





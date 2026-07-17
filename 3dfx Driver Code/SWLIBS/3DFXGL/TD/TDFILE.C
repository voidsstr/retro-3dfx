/*
 *  tdFile.c                                           08/26/94  0.00  NDR
 *                                                     09/07/94  1.00  NDR
 *                                                     09/08/94  2.00  NDR
 *                                                     11/15/94  3.00  NDR
 *                                                     01/03/95  3.10  NDR
 *                                                     09/01/95  4.00  NDR
 *
 *    Three-D Object Library - Library of 3-D object
 *    manipulation routines.
 *
 *    File Module
 *
 *    author: Nathan Drew Robins
 *    email: narobins@es.com
 *
 *
 *    Copyright 1996 by Evans & Sutherland Computer Corporation.  
 *    ALL RIGHTS RESERVED. 
 *    
 *    Permission to use, copy, modify, and distribute this software 
 *    for any purpose and without fee is hereby granted, provided 
 *    that the above copyright notice appear in all copies and that 
 *    both the copyright notice and this notice appear in supporting
 *    documentation.  You may not use the name "Evans & Sutherland 
 *    Computer Corporation" or any trademark of Evans & Sutherland 
 *    Computer Corporation in advertising or publicity pertaining 
 *    to the software without specific, written prior permission. 
 *    
 *    THIS SOFTWARE AND ANY ASSOCIATED MATERIALS IS PROVIDED TO YOU 
 *    "AS-IS" AND WITHOUT REPRESENTATION OR WARRANTY OF ANY KIND, 
 *    EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, 
 *    ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 *    PURPOSE OR NONINFRINGEMENT OF ANY PATENT, COPYRIGHT, TRADE 
 *    SECRET OR OTHER RIGHT.  EVANS & SUTHERLAND COMPUTER CORPORATION 
 *    DOES NOT WARRANT THAT USE OF THE SOFTWARE WILL BE UNINTERRUPTED
 *    OR ERROR-FREE, OR THAT DEFECTS IN THE SOFTWARE WILL BE CORRECTED.
 *    
 *    IN NO EVENT SHALL EVANS & SUTHERLAND COMPUTER CORPORATION BE 
 *    LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT, SPECIAL, INCIDENTAL, 
 *    INDIRECT OR CONSEQUENTIAL  DAMAGES OF ANY KIND, OR ANY DAMAGES 
 *    WHATSOEVER, INCLUDING WITHOUT LIMITATION, LOSS OF PROFIT, LOSS 
 *    OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF THIRD PARTIES, 
 *    WHETHER OR NOT EVANS & SUTHERLAND COMPUTER CORPORATION  HAS 
 *    BEEN ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED 
 *    AND ON ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION 
 *    WITH THE POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *    
 *    US Government Users Restricted Rights 
 *    Use, duplication, or disclosure by the Government is subject 
 *    to restricted rights set forth in FAR 52.227.19(c)(2) or 
 *    subparagraph (c)(1)(ii) of the Rights in Technical Data and 
 *    Computer Software clause at DFARS 252.227-7013 and/or in 
 *    similar or successor clauses in the FAR or the DOD or NASA FAR 
 *    Supplement. Contractor/manufacturer is Evans & Sutherland Computer 
 *    Corporation, 600 Komas Drive, Salt Lake City, UT 84108. 
 *    
 *    Use of this software outside the United States is subject to 
 *    US export laws and regulations.  You agree to comply with all 
 *    such laws and regulations.
 *    
 *    This statement is made under and shall be construed and 
 *    governed by the laws of the State of Utah, without regard to 
 *    conflict of laws principles.
 * 
 */


/* includes */
#include <stdio.h>
#include <string.h>

#include "tdPrivate.h"
#include "tdOBJ.h"
#include "tdOFF.h"


/*
 *  tdWriteObject()
 *
 *    Writes in object data in any supported format.  It uses the
 *    extension on the pathname to decide what type of file format
 *    to write in the file.  Supported extensions (file formats) are:
 *    .obj  - Wavefront .obj file format + .mtl material files. 
 *    .aoff - DEC Object File Format (OFF) + .geom, etc. files (UNIMPLEMENTED)
 *    .dxf  - Drawing Interchange File format (Autodesk) (UNIMPLEMENTED)
 *    .iob  - Lightwave 3D Object File format (UNIMPLEMENTED)
 *    .tddd - Imagine File format (UNIMPLEMENTED)
 *    
 *    pathname - filename of the file the object is to be written in.  It
 *    must be of the form path + name + extension.  Where path is the name of
 *    the path (full pathname if needed, but will go from current working 
 *    directory) and name is the actual name of the object and extension is
 *    one of the valid extensions above.
 *
 *    RETURNs TD_TRUE all went well, TD_FALSE otherwise.
 */
TDboolean tdWriteObject(TDobject *object, char *pathname)
{
    if(strstr(pathname, ".obj"))
	return(_tdWriteOBJObject(object, pathname));
#if 0
    else if(strstr(pathname, ".aoff"))
	return(_tdWriteOff(pathname));
    else if(strstr(pathname, ".dxf"))
	return(_tdWriteDxf(pathname));
    else if(strstr(pathname, ".iob"))
	return(_tdWriteIob(pathname));
    else if(strstr(pathname, ".tddd"))
	return(_tdWriteTddd(pathname));
#endif
    else
    {
	_tdPrintf(TD_ERROR,
		  "can't write file format specified for object `%s'!",
		  pathname);
	return(TD_FALSE);
    }
}

/*
 *  tdReadObject()
 * 
 *    Reads in object data in any supported format.  It uses the
 *    extension on the pathname to decide what type of file format
 *    the data is in.  Supported extensions (file formats) are:
 *    .obj  - Wavefront .obj file format + .mtl material files. 
 *    .aoff - DEC Object File Format (OFF) + .geom, etc. files (UNIMPLEMENTED)
 *    .dxf  - Drawing Interchange File format (Autodesk) (UNIMPLEMENTED)
 *    .iob  - Lightwave 3D Object File format (UNIMPLEMENTED)
 *    .tddd - Imagine File format (UNIMPLEMENTED)
 *    
 *    pathname - filename of the object to be read in.  It must be of the
 *    form path + name + extension.  Where path is the name of the path
 *    (full pathname if needed, but will go from current working directory)
 *    and name is the actual name of the object and extension is one of the
 *    valid extensions above.
 *
 *    RETURNs a pointer to the object if all went well,
 *    otherwise, returns NULL.
 */
TDobject *tdReadObject(char *pathname)
{
    if(strstr(pathname, ".obj"))
	return(_tdReadOBJObject(pathname));
#if 0
    else if(strstr(pathname, ".aoff"))
	return(_tdReadOff(pathname));
    else if(strstr(pathname, ".dxf"))
	return(_tdReadDxf(pathname));
    else if(strstr(pathname, ".iob"))
	return(_tdReadIob(pathname));
    else if(strstr(pathname, ".tddd"))
	return(_tdReadTddd(pathname));
#endif
    else
    {
	_tdPrintf(TD_ERROR, "can't read file format for object `%s'!",
		  pathname);
	return(NULL);
    }
}

/*
 *  tdReadTeximage()
 *
 *    Reads a texture file in any supported format.  It uses the
 *    extension on the filename to decide what type of file format
 *    the data is in.  Supported extensions (file formats) are:
 *    .rgb  - Iris RGB format
 *    .gif  - Graphics Interchange Format (UNIMPLEMENTED)
 *    .jpeg - JPeg format (UNIMPLEMENTED)
 *    .tif  - TIFF format (UNIMPLEMENTED)
 *
 *    filename - name of the file containing the texture data of the
 *    form path + name + extension.  Where path is the name of the
 *    path (full pathname if needed, but will go from current working
 *    directory) and name is the actual name of the file and extension
 *    is one of the valid extensions above.
 */
TDteximage *tdReadTeximage(char *filename)
{
    if(strstr(filename, ".rgb"))
	return(_tdReadRGBTeximage(filename));
#if 0
    else if(strstr(filename, ".gif"))
	return(_tdReadGIFTeximage(filename));
    else if(strstr(filename, ".jpeg"))
	return(_tdReadJPEGTeximage(filename));
    else if(strstr(filename, ".tif"))
	return(_tdReadTIFTeximage(filename));
#endif
    else
    {
	_tdPrintf(TD_ERROR, "can't read file format for object `%s'!",
		  filename);
	return(NULL);
    }
   
}

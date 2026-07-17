#include <3dfx.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gamegen.h"

G3DPoint          *gVertexBuffer;
GTextureList      *gTextureList=NULL;
GTextureList      *gCurTexture;
GPolygon          *gCurPolygon;
short             gNumTextures=0;
long              gNumVertex=0;
long              gCurVertByteOffset=0;
GNode             *gTopNode=NULL;
GNode             *gCurNode;
short             gLevel=1;

/*******************************************************************************
** FUNCTION: TShort()
**
** DESCRIPTION: Extract short from record image.
**
** ARGUMENTS:
** unsigned char *record_ptr
**
** RETURNS:
** short record_value
**
*/
short TShort( unsigned char *record_ptr )
{
  short sval;

#ifdef DOSPLATFORM
  unsigned char sbuff[ 2 ];
  swab( record_ptr, sbuff, 2 ); /* swap two bytes */
  sval = *( (short *)(sbuff) );
  
#else
  sval = *((short *)(record_ptr));
#endif

  return( sval );
}
/* End of TShort() */


/*******************************************************************************
** FUNCTION: TUShort()
**
** DESCRIPTION: Extract unsigned short from record image.
**
** ARGUMENTS:
** unsigned char *record_ptr
**
** RETURNS:
** unsigned short record_value
**
*/
unsigned short TUShort( unsigned char *record_ptr )
{
  unsigned short usval;

#ifdef DOSPLATFORM
  unsigned char usbuff[ 2 ];
  swab( record_ptr, usbuff, 2 ); /* swap two bytes */
  usval = *( (unsigned short *)(usbuff) );
  
#else
  usval = *((unsigned short *)(record_ptr));
#endif

  return( usval );
}
/* End of TUShort() */


/*******************************************************************************
** FUNCTION: TDouble()
**
** DESCRIPTION: Extract double from data.
**
** ARGUMENTS:
** unsigned char *char_ptr
**
** RETURNS:
** double
**
*/
double TDouble( unsigned char *char_ptr )
{
  double dval;

#ifdef DOSPLATFORM
  unsigned char dbuff[ 8 ];
  int i;
  for ( i=0; i<8; i++ )
        dbuff[ i ] = char_ptr[ 7 - i ]; /* reverse byte order*/
  dval = *( (double *)(dbuff) );
  
#else
  dval = *((double *)(char_ptr));
#endif

  return( dval );
}
/* End of TDouble() */


/*******************************************************************************
** FUNCTION: TFloat()
**
** DESCRIPTION: Extract float from data.
**
** ARGUMENTS:
** unsigned char *char_ptr
**
** RETURNS:
** float
**
*/
float TFloat( unsigned char *char_ptr )
{
  float fval;

#ifdef DOSPLATFORM
  unsigned char fbuff[ 4 ];
  int i;
  for ( i=0; i<4; i++ )
        fbuff[ i ] = char_ptr[ 3 - i ]; /* reverse byte order */
  fval = *( (float *)(fbuff) );
  
#else
  fval = *((float *)(char_ptr));
#endif

  return( fval );
}
/* End of TFloat() */


/*******************************************************************************
** FUNCTION: TInt()
**
** DESCRIPTION: Extract int from data.
**
** ARGUMENTS:
** unsigned char *char_ptr
**
** RETURNS:
** int
**
*/
int TInt( unsigned char *char_ptr )
{
  int ival;

#ifdef DOSPLATFORM
  unsigned char ibuff[ 4 ];
  int i;
  for ( i=0; i<4; i++ )
        ibuff[i] = char_ptr[3-i];       
  ival = *( (int *)(ibuff) );
  
#else
  ival = *((int *)(char_ptr));
#endif

  return( ival );
}
/* End of TInt() */



/*******************************************************************************
** FUNCTION: TUInt()
**
** DESCRIPTION: Extract unsigned int from data.
**
** ARGUMENTS:
** unsigned char *char_ptr
**
** RETURNS:
** int
**
*/
unsigned int TUInt( unsigned char *char_ptr )
{
  int uival;

#ifdef DOSPLATFORM
  unsigned char uibuff[ 4 ];
  int i;
  for ( i=0; i<4; i++ )
    uibuff[ i ] = char_ptr[ 3 - i ];   /* swap byte order*/
  uival = *( (int *)(uibuff) );
  
#else
  uival = *((unsigned int *)(char_ptr));
#endif

  return( uival );
}
/* End of TUInt() */

GG_STATUS gg_read_db(char *name)
{
   FILE              *gamegen_file;
   GG_STATUS         result;
   gg_record_info    record_info;
   
   if ((gamegen_file=fopen(name,"rb"))==NULL)
   {
      printf("gg_read_db: Error in opening file %s\n",name);
      return OPEN_ERROR;
   }
   else
   {
      while(1)
      {
         if (result=gg_read_opcode(gamegen_file,&record_info)!=SUCCESS)
         {
            printf("File is done being read\n");   //Note add check for what kind of failure occurred
            //exit(1);
			break;
         }
         switch(record_info.opcode)
         {
   			case OP_SHARED:
               if (gg_read_shared(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
   			case OP_LOD:
               if (gg_read_lod(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
   			case OP_POP:
               if (gg_read_pop(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
   			case OP_VERTEX_LIST:
               if (gg_read_vert_list(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
   			case OP_LONG_ID:
               if (gg_read_unknown(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
   			case OP_NORMAL:
               if (gg_read_coord(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
   			case OP_COLOR:
               if (gg_read_unknown(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
   			case OP_POLYGON:
               if (gg_read_polygon(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
   			case OP_COORD:
               if (gg_read_coord(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
   			case OP_LIGHT_SOURCE:
               if (gg_read_unknown(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
   			case OP_PUSH:
               if (gg_read_push(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
   			case OP_MATERIAL:
               if (gg_read_unknown(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
   			case OP_HEADER:
               if (gg_read_header(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
            case OP_OBJECT:
               if (gg_read_object(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
   			case OP_VERTEX_TEXTURE:
               if (gg_read_coord(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
   			case OP_GROUP:
               if (gg_read_group(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
   			case OP_NORMAL_TEXTURE:
               if (gg_read_norm_tex(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
   			case OP_LIGHT_PALETTE:
               if (gg_read_unknown(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
   			case OP_TEXTURE:
               if (gg_read_texture(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
   			default:
               if (gg_read_unknown(gamegen_file,&record_info)!=SUCCESS)
                  return (READ_ERROR);
               break;
         }
      }
      fclose(gamegen_file);
	  gg_organize_textures();
   }
   return SUCCESS;
}

GG_STATUS gg_read_opcode(FILE *theFile,gg_record_info *info)
{
   char     buffer[8];
   int      count;
   
   count=fread(buffer,1,RECORD_HEADER_SIZE,theFile);
   if (count==0)
      return FAILURE;
   else if (count<RECORD_HEADER_SIZE)
      return READ_ERROR;
   else
   {
      info->opcode=TShort(buffer);
      info->length=(TShort(buffer+2))-4;
   }
   return (SUCCESS);
}

GG_STATUS gg_read_push(FILE *theFile,gg_record_info *info)
{
   char     buffer[60];
   int      count;
   
   count=fread(buffer,1,info->length,theFile);
   if ((count==0)&&(info->length!=0))
      return FAILURE;
   else if (count<info->length)
      return READ_ERROR;
   else
   {
      gLevel++;
   }
   return (SUCCESS);
}

GG_STATUS gg_read_pop(FILE *theFile,gg_record_info *info)
{
   char     buffer[60];
   int      count;
   
   count=fread(buffer,1,info->length,theFile);
   if ((count==0)&&(info->length!=0))
      return FAILURE;
   else if (count<info->length)
      return READ_ERROR;
   else
   {
      gLevel--;
   }
   return (SUCCESS);
}

GG_STATUS gg_read_header(FILE *theFile,gg_record_info *info)
{
   char     buffer[260];
   int      count;
   
   count=fread(buffer,1,info->length,theFile);
   if (count==0)
      return FAILURE;
   else if (count<info->length)
      return READ_ERROR;
   else
   {
      gg_add_node(NULL,TYPE_HEADER);
   }
   return (SUCCESS);
}

GG_STATUS gg_read_shared(FILE *theFile,gg_record_info *info)
{
   char     buffer[8];
   int      count;
   long		memSize,memNeeded;
   
   count=fread(buffer,1,info->length,theFile);
   if (count==0)
      return FAILURE;
   else if (count<info->length)
      return READ_ERROR;
   else
   {
      memSize=TInt(buffer);
	  memNeeded=((memSize/32)+1)*sizeof(G3DPoint);
	  gVertexBuffer=malloc(memNeeded);
	  gNumVertex=0;
	  gCurVertByteOffset=8;
	  if (gVertexBuffer==NULL)
		  return(MEM_ALLOC);
   }
   return (SUCCESS);
}

GG_STATUS gg_read_coord(FILE *theFile,gg_record_info *info)
{
   char     buffer[60];
   int      count;
   
   count=fread(buffer,1,info->length,theFile);
   if (count==0)
      return FAILURE;
   else if (count<info->length)
      return READ_ERROR;
   else
   {
      gVertexBuffer[gNumVertex].x=TDouble(buffer+4);
	  gVertexBuffer[gNumVertex].y=TDouble(buffer+12);
	  gVertexBuffer[gNumVertex].z=TDouble(buffer+20);
	  gVertexBuffer[gNumVertex].byteOffset=gCurVertByteOffset;
	  gVertexBuffer[gNumVertex].u=(float)-1;
	  gVertexBuffer[gNumVertex].v=(float)-1;
      gCurVertByteOffset+=info->length + 4;
      gNumVertex++;
   }
   return (SUCCESS);
}

GG_STATUS gg_read_norm_tex(FILE *theFile,gg_record_info *info)
{
   char     buffer[60];
   int      count;
   
   count=fread(buffer,1,info->length,theFile);
   if (count==0)
      return FAILURE;
   else if (count<info->length)
      return READ_ERROR;
   else
   {
      gVertexBuffer[gNumVertex].x=TDouble(buffer+4);
	  gVertexBuffer[gNumVertex].y=TDouble(buffer+12);
	  gVertexBuffer[gNumVertex].z=TDouble(buffer+20);
	  gVertexBuffer[gNumVertex].byteOffset=gCurVertByteOffset;
	  gVertexBuffer[gNumVertex].u=TFloat(buffer+40);
	  gVertexBuffer[gNumVertex].v=TFloat(buffer+44);
      gCurVertByteOffset+=info->length + 4;
      gNumVertex++;
   }
   return (SUCCESS);
}

GG_STATUS gg_read_polygon(FILE *theFile,gg_record_info *info)
{
   char			buffer[64];
   int			count;
   GPolygon		*temp;
   
   count=fread(buffer,1,info->length,theFile);
   if (count==0)
      return FAILURE;
   else if (count<info->length)
      return READ_ERROR;
   else
   {
      temp=(GPolygon *)malloc(sizeof(GPolygon));
      if (temp==NULL)
         return (MEM_ALLOC);
      gCurPolygon=temp;
	  gCurPolygon->numVerts=0;
	  gg_add_node(temp,TYPE_POLYGON);
   }
   return (SUCCESS);
}

GG_STATUS gg_read_vert_list(FILE *theFile,gg_record_info *info)
{
   char     buffer[60];
   int      count;
   short	numVert,index;
   long		offset,i;
   G3DPoint	*thePoint;
   
   count=fread(buffer,1,info->length,theFile);
   if (count==0)
      return FAILURE;
   else if (count<info->length)
      return READ_ERROR;
   else
   {
      numVert=(info->length>>2);
	  if (numVert>3)
		  printf("Shit num vert > 3 \n");
	  for (index=0;index<numVert;index++)
      {
	     offset=TInt(buffer+(index*4));
		 //i=first possible vertex in list that it can be.  
		 //Actually overcompensating by dividing by 64
		 for ((i=offset>>6),(thePoint=NULL);i<gNumVertex;i++)
		 {
			if(gVertexBuffer[i].byteOffset==offset)
			{
				thePoint=&gVertexBuffer[i];
				break;
			}
		 }
		 if (thePoint==NULL)
		 {
			 printf("gg_read_vert_list:  Error finding vertex\n");
			 return (FAILURE);
		 }
		 gg_add_vert_to_polygon(i);
      }
   }
   return (SUCCESS);
}

GG_STATUS gg_read_object(FILE *theFile,gg_record_info *info)
{
   static int	theCount=0;
   int			count;
   char			buffer[25];

   count=fread(buffer,1,info->length,theFile);
   if (count==0)
      return FAILURE;
   else if (count<info->length)
      return READ_ERROR;
   else
   {
      theCount++;
	  gg_add_node(NULL,TYPE_OBJECT);
	  printf("theObject: %d level: %d\n",theCount,gLevel);
   }
   return (SUCCESS);
}

GG_STATUS gg_read_group(FILE *theFile,gg_record_info *info)
{
   static int	theGroup=0;
   int			count;
   char			buffer[30];

   count=fread(buffer,1,info->length,theFile);
   if (count==0)
      return FAILURE;
   else if (count<info->length)
      return READ_ERROR;
   else
   {
      theGroup++;
	  gg_add_node(NULL,TYPE_GROUP);
	  printf("theGroup: %d level: %d\n",theGroup,gLevel);
   }
   return (SUCCESS);
}

GG_STATUS gg_read_lod(FILE *theFile,gg_record_info *info)
{
   static int	theLOD=0;
   int			count;
   char			buffer[70];
   
   count=fread(buffer,1,info->length,theFile);
   if (count==0)
      return FAILURE;
   else if (count<info->length)
      return READ_ERROR;
   else
   {
      theLOD++;
	  gg_add_node(NULL,TYPE_LOD);
	  printf("theLOD: %d level: %d\n",theLOD,gLevel);
   }
   return (SUCCESS);
}

GG_STATUS gg_read_texture(FILE *theFile,gg_record_info *info)
{
   int			count;
   char			buffer[220];
   GTextureList *temp;

   count=fread(buffer,1,info->length,theFile);
   if (count==0)
      return FAILURE;
   else if (count<info->length)
      return READ_ERROR;
   else
   {
      temp=(GTextureList *)malloc(sizeof(GTextureList));
	  if (temp==NULL)
	  {
         return MEM_ALLOC;
	  }
	  if (gTextureList==NULL)
      {
		  gTextureList=temp;
		  gTextureList->left=NULL;
		  gTextureList->right=NULL;
	  }
	  else
      {
         gCurTexture->right=temp;
         temp->left=gCurTexture;
	  }
      gCurTexture=temp;
	  temp->right=NULL;
	  gNumTextures++;
	  strcpy(gCurTexture->texture.name,buffer);
   }
   return (SUCCESS);
}

GG_STATUS gg_read_unknown(FILE *theFile,gg_record_info *info)
{
   static char       *buffer=NULL;
   static long       bufferSize;
   long              count;
   
   if (buffer==NULL)
   {
      buffer=malloc(info->length);
      if (buffer!=NULL)
         bufferSize=info->length;
      else
         return (MEM_ALLOC);
   }
   if (info->length>bufferSize)
   {
      free(buffer);
	  buffer=malloc(info->length);
      if (buffer!=NULL)
         bufferSize=info->length;
      else
         return (MEM_ALLOC);
   }
   fread(buffer,1,info->length,theFile);
   if (count==0)
      return FAILURE;
   else if (count<RECORD_HEADER_SIZE)
      return READ_ERROR;
   else
   {
      // Don't do anything because opcode is not recognized
   }
   return (SUCCESS);
}

GG_STATUS gg_add_node(void *info, short kind)
{
   static short		lastLevel=0;
   GG_STATUS		response;
   short			tempVal;

   if (gTopNode==NULL)
      response=gg_add_top_node(info,kind);
   else if (gLevel>lastLevel)
      response=gg_add_node_down(info,kind);
   else
   {
      tempVal=lastLevel-gLevel;
	  gg_trace_back_levels(tempVal);
      response=gg_add_node_right(info,kind);
   }
   gCurNode->info=info;
   gCurNode->type=kind;

   lastLevel=gLevel;

   return (response);
}

GG_STATUS gg_add_top_node(void *info, short kind)
{
   GNode		*temp;

   temp=malloc(sizeof(GNode));
   if (temp==NULL)
      return (MEM_ALLOC);
   
   temp->up=NULL;
   temp->down=NULL;
   temp->right=NULL;
   temp->left=NULL;
   gTopNode=gCurNode=temp;
   
   return SUCCESS;
}

GG_STATUS gg_add_node_down(void *info, short kind)
{
   GNode		*temp;

   temp=malloc(sizeof(GNode));
   if (temp==NULL)
      return (MEM_ALLOC);
   
   temp->up=gCurNode;
   temp->down=NULL;
   temp->right=NULL;
   temp->left=NULL;
   gCurNode->down=temp;
   gCurNode=temp;
   
   return SUCCESS;
}

GG_STATUS gg_add_node_right(void *info, short kind)
{
   GNode		*temp;

   temp=malloc(sizeof(GNode));
   if (temp==NULL)
      return (MEM_ALLOC);
   
   temp->up=NULL;
   temp->down=NULL;
   temp->left=gCurNode;
   temp->right=NULL;
   gCurNode->right=temp;
   gCurNode=temp;

   return SUCCESS;
}

GG_STATUS gg_add_vert_to_polygon(long vertIndex)
{
   short		*numVerts=&gCurPolygon->numVerts;

   if (gCurPolygon->numVerts>3)
      return (FAILURE);
   gCurPolygon->index[(*numVerts)++]=vertIndex;
   return SUCCESS;	
}

GG_STATUS gg_trace_back_levels(short numLevels)
{
	while (numLevels!=0)
	{
		if (gCurNode->up!=NULL)
		{
			gCurNode=gCurNode->up;
			if (numLevels--==0)
				return SUCCESS;
		}
		else if (gCurNode->left!=NULL)
			gCurNode=gCurNode->left;
		else
			return FAILURE;
	}
	return SUCCESS;
}

GG_STATUS gg_organize_textures(void)
{
	GTexture		*temp;
	GTextureList	*tempList;
	short			i;

	temp=(GTexture *)malloc((sizeof(GTexture))*gNumTextures);
	if (temp==NULL)
		return MEM_ALLOC;
	gCurTexture=gTextureList;
	for (i=0;i<gNumTextures;i++)
	{
		temp[i]=gCurTexture->texture;
		tempList=gCurTexture;
		gCurTexture=gCurTexture->right;
		free(tempList);
	}
}
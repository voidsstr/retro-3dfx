#include <3dfx.h>
#include <stdlib.h>
#include <stdio.h>
#include "gamegen.h"

extern GNode             *gTopNode;
extern GNode             *gCurNode;
extern G3DPoint          *gVertexBuffer;
extern long              gNumVertex;
GDIRECTION               gFromWhere=NO_WHERE;
FILE                     *g_vrml_file;
short                    g_vrml_level;
char                     *gTexCoordBuffer=NULL;
long                     gTexCoordSize;
long                     gTexCoordUsed;

GG_STATUS gg_convert_db(void)
{
   g_vrml_file=fopen("myVrml.wrl","wb");
   if (g_vrml_file==NULL)
      return (OPEN_ERROR);
   fprintf(g_vrml_file,"#VRML V1.0 ascii\r\n\r\n");
   AddCoordinates();
   AddTexCoordinates();
   gCurNode=gTopNode;
   while (1)
   {
	   if (CheckIfDone())
		   break;
	   switch(gCurNode->type)
	   {
		  case TYPE_HEADER:
             GoNextNode();
			 break;
		  case TYPE_GROUP:
			 HandleGroupNode();
			 break;
		  case TYPE_OBJECT:
			 HandleObjectNode();
			 break;
		  case TYPE_LOD:
             HandleLODNode();
			 break;
		  case TYPE_POLYGON:
			 HandlePolygonNode();
			 break;
		  default:
			 break;
	   }
   }
   fclose(g_vrml_file);
   return SUCCESS;
}

void HandleGroupNode(void)
{
	if(FirstTimeNode())
	{
		AddSeparator();
		GoNextNode();
		if ((gFromWhere==LEFT)||(gFromWhere==RIGHT))
			CloseSeparator();
		return;
	}
	else if (NodeDone())
		CloseSeparator();
	GoNextNode();
}

void HandleLODNode(void)
{
	FxBool			done=FXFALSE;
	static GNode	*temp;

	if(FirstTimeNode())
	{
		AddSeparator();
		GoNextNode();
		if ((gFromWhere==LEFT)||(gFromWhere==RIGHT))
			CloseSeparator();
		return;
	}
	else if (NodeDone())
	{
		CloseSeparator();
		if (done==FXTRUE)
			return;
	}
	GoNextNode();
}

void HandleObjectNode(void)
{
	if(FirstTimeNode())
	{
		AddSeparator();
		GoNextNode();
		if ((gFromWhere==LEFT)||(gFromWhere==RIGHT))
			CloseSeparator();
		return;
	}
	else if (NodeDone())
		CloseSeparator();
	GoNextNode();
}

void HandlePolygonNode(void)
{
	if (FirstTimeNode())
	{
		if (gFromWhere==UP)
		{
			AddIndexedFaceSet();
		}
		AddFaceInfo();
	}
	GoNextNode();
	if (NodeDone())
	{
		CloseIndexedFaceSet();
	}
}

void AddSeparator(void)
{
	AddFormatSpaces();
	fprintf(g_vrml_file,"Separator\r\n");
	AddFormatSpaces();
	fprintf(g_vrml_file,"{\r\n");
	g_vrml_level++;
}

void CloseSeparator(void)
{
	g_vrml_level--;
	AddFormatSpaces();
	fprintf(g_vrml_file,"}\r\n");
}

void AddIndexedFaceSet(void)
{
	AddFormatSpaces();
	fprintf(g_vrml_file,"IndexedFaceSet\r\n");
	AddFormatSpaces();
	fprintf(g_vrml_file,"{\r\n");
	g_vrml_level++;
	AddFormatSpaces();
	fprintf(g_vrml_file,"coordIndex\r\n");
	AddFormatSpaces();
	fprintf(g_vrml_file,"[\r\n");
	g_vrml_level++;
}

void AddFaceInfo(void)
{
	GPolygon		*poly=(GPolygon *)gCurNode->info;

	AddFormatSpaces();
	fprintf(g_vrml_file,"%5d, %5d, %5d,  -1,\r\n",poly->index[0],
                                                  poly->index[1],
                                                  poly->index[2]);
}

void AddCoordinates(void)
{
	long		i;

	fprintf(g_vrml_file,"Coordinate3\r\n");
	fprintf(g_vrml_file,"{\r\n");
	g_vrml_level++;
	AddFormatSpaces();
	fprintf(g_vrml_file,"point\r\n");
	AddFormatSpaces();
	fprintf(g_vrml_file,"[\r\n");
	g_vrml_level++;
	for (i=0;i<gNumVertex;i++)
	{
		AddFormatSpaces();
		fprintf(g_vrml_file,"%lf %lf %lf,\r\n",gVertexBuffer[i].x,
                                               gVertexBuffer[i].y,
			                                   gVertexBuffer[i].z);
	}
	g_vrml_level--;
	AddFormatSpaces();
	fprintf(g_vrml_file,"]\r\n");
	g_vrml_level--;
	AddFormatSpaces();
	fprintf(g_vrml_file,"}\r\n");
}

void AddTexCoordinates(void)
{
	long	i;

	fprintf(g_vrml_file,"TextureCoordinate2\r\n");
	fprintf(g_vrml_file,"{\r\n");
	g_vrml_level++;
	AddFormatSpaces();
	fprintf(g_vrml_file,"point\r\n");
	AddFormatSpaces();
	fprintf(g_vrml_file,"[\r\n");
	g_vrml_level++;
	for (i=0;i<gNumVertex;i++)
	{
		if (gVertexBuffer[i].u!=-1)
		{
			AddFormatSpaces();
			fprintf(g_vrml_file,"%f %f,\r\n",gVertexBuffer[i].u,
                                             gVertexBuffer[i].v);
		}
	}
	g_vrml_level--;
	AddFormatSpaces();
	fprintf(g_vrml_file,"]\r\n");
	g_vrml_level--;
	AddFormatSpaces();
	fprintf(g_vrml_file,"}\r\n");
}

void CloseIndexedFaceSet(void)
{
	g_vrml_level--;
	AddFormatSpaces();
	fprintf(g_vrml_file,"]\r\n");
	if (gTexCoordBuffer!=NULL)
	{
		g_vrml_level++;
		AddFormatSpaces();
		fprintf(g_vrml_file,"textureCoordIndex\r\n");
		AddFormatSpaces();
		fprintf(g_vrml_file,"[\r\n");
		g_vrml_level++;

	}
	g_vrml_level--;
	AddFormatSpaces();
	fprintf(g_vrml_file,"}\r\n");
}

void AddFormatSpaces(void)
{
	short	i;

	for (i=0;i<g_vrml_level;i++)
	{
		fprintf(g_vrml_file,"   ");
	}
}

FxBool CheckIfDone(void)
{
	if (gCurNode!=gTopNode)
		return FXFALSE;
	else
	{
		if (gFromWhere!=NO_WHERE)
			return FXTRUE;
		else
			return FXFALSE;
	}
}

GDIRECTION GoNextNode(void)
{
	switch(gFromWhere)
	{
		case UP:
			if (gCurNode->down!=NULL)
			{
				gCurNode=gCurNode->down;
				return (gFromWhere=UP);
			}
			else if (gCurNode->right!=NULL)
			{
				gCurNode=gCurNode->right;
				return (gFromWhere=LEFT);
			}
			else 
			{
				gCurNode=gCurNode->up;
				return (gFromWhere=DOWN);
			}
			break;
		case LEFT:
			if (gCurNode->down!=NULL)
			{
				gCurNode=gCurNode->down;
				return (gFromWhere=UP);
			}
			else if (gCurNode->right!=NULL)
			{
				gCurNode=gCurNode->right;
				return (gFromWhere=LEFT);
			}
			else 
			{
				gCurNode=gCurNode->left;
				return (gFromWhere=RIGHT);
			}
			break;
		case DOWN:
			if (gCurNode->right!=NULL)
			{
				gCurNode=gCurNode->right;
				return (gFromWhere=LEFT);
			}
		case RIGHT:
			if (gCurNode->up!=NULL)
			{
				gCurNode=gCurNode->up;
				return (gFromWhere=DOWN);
			}
			else if (gCurNode->left!=NULL)
			{
				gCurNode=gCurNode->left;
				return (gFromWhere=RIGHT);
			}
			else
				return (gFromWhere=NO_WHERE);
			break;
		default:
			if (gCurNode->down!=NULL)
			{
				gCurNode=gCurNode->down;
				return (gFromWhere=UP);
			}
			else if (gCurNode->right!=NULL)
			{
				gCurNode=gCurNode->right;
				return (gFromWhere=LEFT);
			}
			else
				printf("Should be end of tree\n");
			break;
	}
	return (NO_WHERE);
}

FxBool FirstTimeNode(void)
{
	if ((gFromWhere==LEFT)||(gFromWhere==UP))
		return FXTRUE;
	else
		return FXFALSE;
}

FxBool NodeDone(void)
{
	if (gFromWhere==DOWN)
		return FXTRUE;
	else 
		return FXFALSE;
}

GG_STATUS OrganizePolys(void)
{
	GNode		*curNode,*temp;
	
	curNode=gTopNode;

}


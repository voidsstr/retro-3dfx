/* THIS C FILE HAS BEEN HACKED BY AN EXPERIMENTAL PERL SCRIPT
// maketrap.pl
// on 15 Jun 98
//
// Trap.c: implementation of the Trap functions
//
// Hector Yee
// yhy1@cornell.edu
// yee@3dfx.com
// 6/8/98
//
// This class captures glide calls
//
*/

#include <glide.h>
#include <stdio.h>
#include <conio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "Trap.h"
#include "hash.h"

FxBool ScreenEcho;			/* Echo calls to screen or not */
FxBool fastcompare;			/* fast compare or exact compare */
FxBool nodraw;				/* skip drawing functions */
FILE *outfile;				// Our Binary Log File
GrVertexLayoutInfo gc;		/* Internal vertex layout representation */
HashTable *states;			/* keeps track of which state is being set or get */
HashTable *textures;		/* keeps track of which texture is being used */
HashTable *lfbs;			/* keeps track of which lfb is being used */
void ** play_states;		/* keeps track of playback states */
List ** trap_states;		/* keeps track of trapped states */
char dir_base[MAX_PATH];	/* directory base for playback/trapping */
FxU32 state_size;			/* to avoid extra grGets, use the playback grGet(STATE_SIZE) */
FxU32 numstates;			/* total number of states (play or trap)*/
FxU32 state_table_size;		/* size of state table */
FxU32 vtxa[15],vtxb[15],vtxc[15]; /* pre allocate vertices for faster playback */
FxBool frontlocked,backlocked,auxlocked; /* buffer lock status */
FxI32 sizeofres;			/* used for grqueryresolutions */
GrLfbInfo_t	lfbf,lfba,lfbb;				 /* frame buffer locks */
GuTexPalette mytexpalette;				 /* preallocate for speed */
GuNccTable   myncctable;				 /* preallocate for speed */
char MESSAGE[5000];						 /* message buffer */

/* glide 3 to glide 2 conversion */

#define LOD(x)     x=8-x;
#define ASPECT(x)  x=3-x;

/* user defined portion */
void Init()
// Inits common to Playback and Trap
{
	/* Makes new hashtables */
	states	= MakeHashTable();
	textures= MakeHashTable();
	lfbs	= MakeHashTable();

	/* Init variables */
	play_states=NULL;
	state_size=0;
	numstates=0;
	sizeofres=0;


	/* Disable all the vertex stuff */
	my_grVertexLayout( GR_PARAM_XY,0,GR_PARAM_DISABLE);
	my_grVertexLayout( GR_PARAM_Z,0,GR_PARAM_DISABLE);
	my_grVertexLayout( GR_PARAM_W,0,GR_PARAM_DISABLE);
	my_grVertexLayout( GR_PARAM_Q,0,GR_PARAM_DISABLE);
	my_grVertexLayout( GR_PARAM_FOG_EXT,0,GR_PARAM_DISABLE);
	my_grVertexLayout( GR_PARAM_A,0,GR_PARAM_DISABLE);
	my_grVertexLayout( GR_PARAM_RGB,0,GR_PARAM_DISABLE);
	my_grVertexLayout( GR_PARAM_PARGB,0,GR_PARAM_DISABLE);
	my_grVertexLayout( GR_PARAM_ST0,0,GR_PARAM_DISABLE);
	my_grVertexLayout( GR_PARAM_ST1,0,GR_PARAM_DISABLE);
	my_grVertexLayout( GR_PARAM_ST2,0,GR_PARAM_DISABLE);
	my_grVertexLayout( GR_PARAM_Q0,0,GR_PARAM_DISABLE);
	my_grVertexLayout( GR_PARAM_Q1,0,GR_PARAM_DISABLE);
	my_grVertexLayout( GR_PARAM_Q2,0,GR_PARAM_DISABLE);

}


FILE * BatchInit(char *indir)
{
	FILE *result = NULL;
	int i;
	char infile[MAX_PATH];

	Init();

	strcpy(dir_base,indir);
	/* append \ if necessary */
	if (strlen(dir_base)>1)
	{
		if (dir_base[strlen(dir_base)-1]!='\\') strcat(dir_base,"\\");
	}

	strcpy(infile,dir_base);
	strcat(infile,TOKENF);
	printf("Reading Tokens from: %s\n",infile);
	
	result=fopen(infile,"rb");

        if (result == NULL) {
		return NULL; /* there was an error opening the file */
        }

#if 0
	while (!result)
	{
		printf("Invalid Directory. Directory name: ");
		gets(dir_base);
		/* append \ if necessary */
		if (strlen(dir_base)>1)
		{
			if (dir_base[strlen(dir_base)-1]!='\\') strcat(dir_base,"\\");
		}
		strcpy(infile,dir_base);
		strcat(infile,TOKENF);
		result=fopen(infile,"rb");
	}
#endif

	play_states=malloc(sizeof(void*)*DARRAYSIZE);
	for (i=0; i<DARRAYSIZE; i++)
	{
		play_states[i]=NULL;
	}
	state_table_size=DARRAYSIZE;

	return result;
}

FILE * PlayInit()
{
	FILE *result;
	int i;
	char infile[MAX_PATH];

	Init();

	printf("Enter Trap/Playback Directory: ");
	gets(dir_base);
	/* append \ if necessary */
	if (strlen(dir_base)>1)
	{
		if (dir_base[strlen(dir_base)-1]!='\\') strcat(dir_base,"\\");
	}

	strcpy(infile,dir_base);
	strcat(infile,TOKENF);
	printf("Reading Tokens from: %s\n",infile);
	
	result=fopen(infile,"rb");

	while (!result)
	{
		printf("Invalid Directory. Directory name: ");
		gets(dir_base);
		/* append \ if necessary */
		if (strlen(dir_base)>1)
		{
			if (dir_base[strlen(dir_base)-1]!='\\') strcat(dir_base,"\\");
		}
		strcpy(infile,dir_base);
		strcat(infile,TOKENF);
		result=fopen(infile,"rb");
	}

	play_states=malloc(sizeof(void*)*DARRAYSIZE);
	for (i=0; i<DARRAYSIZE; i++)
	{
		play_states[i]=NULL;
	}
	state_table_size=DARRAYSIZE;

	return result;
}

void Done()
{
	KillHashTable(states);
	KillHashTable(textures);
	KillHashTable(lfbs);
	states=NULL;
	textures=NULL;
	lfbs=NULL;
}

void PlayDone(FILE *infile)
{
	FxU32 i;
	Done();

	for (i=0; i<numstates; i++)
	{
		free(play_states[i]);
	}
	free(play_states);
	
	fclose(infile);
}

void CTrapDone()
{
	FxU32 i;	
	List *next,*now;
	Done();
	
	for (i=0; i<state_table_size; i++)
	{
		now=trap_states[i];
		while (now)
		{
			next=now->next;
			free(now->data);			
			free(now);			
			now=next;
		}
		trap_states[i]=NULL;
	}
	free(trap_states);

	if (outfile) fclose(outfile);
}

/* local functions */

FxBool filecomp(char * filename, void *data, FxU32 length)
/* compares files and returns TRUE if there is a match */
/* if file does not exist, create and return TRUE */
{
	FILE *tmp;
	FxU32 len;	
	void *indat;
	FxBool match;		

	tmp=NULL;
	tmp=fopen(filename,"rb");
	if (tmp==NULL)
	{
		tmp=fopen(filename,"wb");
		fwrite(&length,sizeof(FxU32),1,tmp);
		fwrite(data,length,1,tmp);
		fclose(tmp);
		return FXTRUE;
	} else
	{
		fread(&len,sizeof(FxU32),1,tmp);
		if (len!=length)
		{
			/* different size... must be different files! */
			fclose(tmp);
			return FXFALSE;
		}

		if (fastcompare) return FXTRUE;

		indat=malloc(length);
		fread(indat,length,1,tmp);
		fclose(tmp);		
		
		if (strncmp((char *) indat,(char *) data,length))
		{
			match=FXFALSE;
		} else
		{
			match=FXTRUE;
		}
		free(indat);
		return match;
	}
}

void * load_f(char *filename)
{
	char filen[MAX_PATH];	
	FxU32 size;	 
	FILE *tmp;
	void *data;

	/* add directory base to filename */
	strcpy(filen,dir_base);
	strcat(filen,filename);
	tmp=fopen(filen,"rb");
	
	if (tmp!=NULL)
	/* read in data */
	{
		fread(&size,sizeof(FxU32),1,tmp);
		data=malloc(size);
		fread(data,size,1,tmp);
		fclose(tmp);
		return data;
	}
	else
	/* die */
	{
		return NULL;
	}
}

void load_texture(char *filename, void** data)
{
	*data=load_f(filename);
	if (data==NULL)
	{
		printf("Texture %s not found!\n",filename);		
		exit(1);
	}
}

void *load_lfb(char *filename)
{
	void *data;
	
	data=load_f(filename);
	if (data==NULL)
	{
		printf("LFB %s not found!\n",filename);		
		exit(1);
	}
	return data;
}


void SetScreenEcho(FxBool echo)
{
	ScreenEcho=echo;
}

void SetFastComp(FxBool fast)
{
	fastcompare=fast;
}

void SetNoDraw(FxBool nd)
{
	nodraw=nd;
}


/* From distrip.c */
void my_grVertexLayout(FxU32 param, FxI32 offset, FxU32 mode)
{
  switch (param) {
  case GR_PARAM_XY:
    gc.vertexInfo.offset = offset;
    gc.vertexInfo.mode = mode;
    break;

  case GR_PARAM_Z:

    gc.zInfo.offset = offset;
    gc.zInfo.mode = mode;

    break;

  case GR_PARAM_W:

    gc.wInfo.offset = offset;
    gc.wInfo.mode = mode;

    break;

  case GR_PARAM_FOG_EXT:
    /*
    ** Fog coordinate is an extension in Glide3. It is supported in V2 and VB.
    ** If z-buffering, we use w iterator for fog coordinate.
    ** If w-buffering, we move the w iterator to floating point z and use w iterator for fog.
    */
    gc.fogInfo.offset = offset;
    gc.fogInfo.mode = mode;

    break;

  case GR_PARAM_A:

    gc.aInfo.offset = offset;
    gc.colorType = GR_FLOAT;
    gc.aInfo.mode = mode;

    break;
  case GR_PARAM_RGB:

    gc.rgbInfo.offset = offset;
    gc.colorType = GR_FLOAT;
    gc.rgbInfo.mode = mode;

    break;
  case GR_PARAM_PARGB:

    gc.pargbInfo.offset = offset;
    gc.colorType = GR_U8;
    gc.pargbInfo.mode = mode;

    break;
  case GR_PARAM_ST0:

    gc.st0Info.offset = offset;
    gc.st0Info.mode = mode;

    break;
  case GR_PARAM_ST1:

    gc.st1Info.offset = offset;
    gc.st1Info.mode = mode;

    break;
  case GR_PARAM_Q:

    gc.qInfo.offset = offset;
    gc.qInfo.mode = mode;

    break;
  case GR_PARAM_Q0:
    gc.q0Info.offset = offset;
    gc.q0Info.mode = mode;

    break;
  case GR_PARAM_Q1:

    gc.q1Info.offset = offset;
    gc.q1Info.mode = mode;

    break;
  }
} /* my_grVertexLayout */

void transformvertex(void *invert, GrVertex *outvert)
/* transforms glide 3 vertices to glide2 vertices */
{

//float x, y, z;                /* X, Y, and Z of scrn space -- Z is ignored */
//  float r, g, b;                /* R, G, B, ([0..255.0]) */
  //float ooz;                    /* 65535/Z (used for Z-buffering) */
  //float a;                      /* Alpha [0..255.0] */
  //float oow;                    /* 1/W (used for W-buffering, texturing) */
  //GrTmuVertex  tmuvtx[GLIDE_NUM_TMU];
/*
  #define DOCOPY(OPART,PART) if (gc.PART.mode == GR_PARAM_ENABLE) \
	{ outvert-> OPART = *(float *) ((int *) (gc.PART.offset + (int) invert)); }*/

  if (gc.vertexInfo.mode == GR_PARAM_ENABLE) 
	{
	  outvert-> x= *(float *) ((int *) (gc.vertexInfo.offset + (int) invert));
	  outvert-> y= *(float *) ((int *) (gc.vertexInfo.offset + sizeof(float)+ (int) invert));  
	}
   if (gc.zInfo.mode == GR_PARAM_ENABLE) 
	{
	  outvert-> ooz= *(float *) ((int *) (gc.zInfo.offset + (int) invert));
	  if (outvert->ooz!=0) outvert-> z=1/outvert->ooz;
	}

  if (gc.rgbInfo.mode == GR_PARAM_ENABLE) 
	{
	  outvert-> r= *(float *) ((int *) (gc.rgbInfo.offset + (int) invert));
	  outvert-> g= *(float *) ((int *) (gc.rgbInfo.offset + sizeof(float)+ (int) invert));  
	  outvert-> b= *(float *) ((int *) (gc.rgbInfo.offset + 2*sizeof(float)+ (int) invert));  
	}
  
  if (gc.aInfo.mode == GR_PARAM_ENABLE) 
	{
	  outvert-> a= *(float *) ((int *) (gc.aInfo.offset + (int) invert));	  
	}

   if (gc.qInfo.mode == GR_PARAM_ENABLE) 
   {
	  outvert-> oow= *(float *) ((int *) (gc.qInfo.offset + (int) invert));
   }

   if (gc.st0Info.mode == GR_PARAM_ENABLE) 
   {
	  outvert->tmuvtx[0].sow= *(float *) ((int *) (gc.st0Info.offset + (int) invert));	  
	  outvert->tmuvtx[0].tow= *(float *) ((int *) (gc.st0Info.offset + sizeof(float)+ (int) invert));	  
	  outvert->tmuvtx[0].oow=outvert->oow;
	  /*outvert->tmuvtx[0].sow*=outvert->oow;
	  outvert->tmuvtx[0].tow*=outvert->oow;*/
   }   
}



// this function calculates the vertex size
#define GETSIZE(PART) \
	if (gc.PART.mode == GR_PARAM_ENABLE) size=__max(size,gc.PART.offset+4*SIZE_##PART);
int getvertexsize()
{
  int size;

  size=0;
  // GR_PARAM_XY:
	GETSIZE(vertexInfo);

  // GR_PARAM_Z:
    GETSIZE(zInfo);

  // GR_PARAM_W:
    GETSIZE(wInfo);

  // GR_PARAM_FOG_EXT:
    GETSIZE(fogInfo);

  // GR_PARAM_A:
	GETSIZE(aInfo);
    
  // GR_PARAM_RGB:
    GETSIZE(rgbInfo);
    
  // GR_PARAM_PARGB:
    GETSIZE(pargbInfo);
    
  // GR_PARAM_ST0:
    GETSIZE(st0Info);
    
  // GR_PARAM_ST1:
    GETSIZE(st1Info);
    
  // GR_PARAM_Q:
    GETSIZE(qInfo);
    
  // GR_PARAM_Q0:
    GETSIZE(q0Info);
    
  // GR_PARAM_Q1:
    GETSIZE(q1Info);

	return size;
}

// Macro to output structure
#define OUTPUT(PART) \
	if (gc.PART.mode == GR_PARAM_ENABLE) \
{  fwrite((int *) (gc.PART.offset + (int) p),sizeof(int),SIZE_##PART,outfile); }

// this function outputs the vertex according
// to what stuff has been enabled/disabled
void my_outputvertex(void *p)
{
  // GR_PARAM_XY:
	
	OUTPUT(vertexInfo);

  // GR_PARAM_Z:
    OUTPUT(zInfo);

  // GR_PARAM_W:
    OUTPUT(wInfo);

  // GR_PARAM_FOG_EXT:
    OUTPUT(fogInfo);

  // GR_PARAM_A:
	OUTPUT(aInfo);
    
  // GR_PARAM_RGB:
    OUTPUT(rgbInfo);
    
  // GR_PARAM_PARGB:
    OUTPUT(pargbInfo);
    
  // GR_PARAM_ST0:
    OUTPUT(st0Info);
    
  // GR_PARAM_ST1:
    OUTPUT(st1Info);
    
  // GR_PARAM_Q:
    OUTPUT(qInfo);
    
  // GR_PARAM_Q0:
    OUTPUT(q0Info);
    
  // GR_PARAM_Q1:
    OUTPUT(q1Info);
    
} /* my_outputvertex */

// Macro to output structure
#define INPUT(PART) \
	if (gc.PART.mode == GR_PARAM_ENABLE) \
{  fread((int *) (gc.PART.offset + (int) vtx),sizeof(int),SIZE_##PART,infile); }

void * my_inputvertex(FILE *infile)
{
	void * p;
	p=malloc(sizeof(int)*15);
	my_inputvnalloc(p,infile);
	return p;
}

/* faster, non allocating version */
void my_inputvnalloc(void *vtx, FILE *infile)
{
  // GR_PARAM_XY:
	INPUT(vertexInfo);	

  // GR_PARAM_Z:
    INPUT(zInfo);

  // GR_PARAM_W:
    INPUT(wInfo);

  // GR_PARAM_FOG_EXT:
    INPUT(fogInfo);

  // GR_PARAM_A:
	INPUT(aInfo);
    
  // GR_PARAM_RGB:
    INPUT(rgbInfo);
    
  // GR_PARAM_PARGB:
    INPUT(pargbInfo);
    
  // GR_PARAM_ST0:
    INPUT(st0Info);
    
  // GR_PARAM_ST1:
    INPUT(st1Info);
    
  // GR_PARAM_Q:
    INPUT(qInfo);
    
  // GR_PARAM_Q0:
    INPUT(q0Info);
    
  // GR_PARAM_Q1:
    INPUT(q1Info);
}


/* Playback stuff */
void play_grAADrawTriangle(FILE *infile)
{

	FxBool ab_antialias;
	FxBool bc_antialias;
	FxBool ca_antialias;

	my_inputvnalloc(&vtxa[0],infile);
	my_inputvnalloc(&vtxb[0],infile);
	my_inputvnalloc(&vtxc[0],infile);


	fread(& ab_antialias, sizeof(FxBool),1,infile);
	fread(& bc_antialias, sizeof(FxBool),1,infile);
	fread(& ca_antialias, sizeof(FxBool),1,infile);		

	//if (!nodraw) grAADrawTriangle(&a, &0], &vtxc[0], ab_antialias, bc_antialias, ca_antialias);	

	if (ScreenEcho)
	{
		printf("grAADrawTriangle()\n");
	}

}

void play_grAlphaBlendFunction(FILE *infile)
{

	GrAlphaBlendFnc_t rgb_sf;
	GrAlphaBlendFnc_t rgb_df;
	GrAlphaBlendFnc_t alpha_sf;
	GrAlphaBlendFnc_t alpha_df;

	fread(& rgb_sf, sizeof(GrAlphaBlendFnc_t),1,infile);
	fread(& rgb_df, sizeof(GrAlphaBlendFnc_t),1,infile);
	fread(& alpha_sf, sizeof(GrAlphaBlendFnc_t),1,infile);
	fread(& alpha_df, sizeof(GrAlphaBlendFnc_t),1,infile);
	grAlphaBlendFunction(rgb_sf, rgb_df, alpha_sf, alpha_df);

	if (ScreenEcho)
	{
		printf("grAlphaBlendFunction(%u,%u,%u,%u)\n",rgb_sf, rgb_df, alpha_sf, alpha_df);
	}

}

void play_grAlphaCombine(FILE *infile)
{

	GrCombineFunction_t function;
	GrCombineFactor_t factor;
	GrCombineLocal_t local;
	GrCombineOther_t other;
	FxBool invert;
	fread(& function, sizeof(GrCombineFunction_t),1,infile);
	fread(& factor, sizeof(GrCombineFactor_t),1,infile);
	fread(& local, sizeof(GrCombineLocal_t),1,infile);
	fread(& other, sizeof(GrCombineOther_t),1,infile);
	fread(& invert, sizeof(FxBool),1,infile);
	grAlphaCombine(function, factor, local, other, invert);

	if (ScreenEcho)
	{
		printf("grAlphaCombine(%u,%u,%u,%u,%u)\n",function, factor, local, other, invert);
	}

}

void play_grAlphaControlsITRGBLighting(FILE *infile)
{

	FxBool enable;
	fread(& enable, sizeof(FxBool),1,infile);
	grAlphaControlsITRGBLighting(enable);

	if (ScreenEcho)
	{
		if (enable)
			printf("grAlphaControlsITRGBLighting(TRUE)\n");
		else
			printf("grAlphaControlsITRGBLighting(FALSE)\n");
	}

}

void play_grAlphaTestFunction(FILE *infile)
{

	GrCmpFnc_t function;
	fread(& function, sizeof(GrCmpFnc_t),1,infile);
	grAlphaTestFunction(function);

	if (ScreenEcho)
	{
		printf("grAlphaTestFunction(%u)\n",function);
	}

}

void play_grAlphaTestReferenceValue(FILE *infile)
{

	GrAlpha_t value;
	fread(& value, sizeof(GrAlpha_t),1,infile);
	grAlphaTestReferenceValue(value);

	if (ScreenEcho)
	{
		printf("grAlphaTestReferenceValue(%u)\n",value);
	}

}

void play_grBufferClear(FILE *infile)
{
	GrColor_t color;
	GrAlpha_t alpha;
	FxU32 depth;
	fread(& color, sizeof(GrColor_t),1,infile);
	fread(& alpha, sizeof(GrAlpha_t),1,infile);
	fread(& depth, sizeof(FxU32),1,infile);
	grBufferClear(color, alpha, (FxU16) depth);

	if (ScreenEcho)
	{
		printf("grBufferClear(%x, %i, %i)\n",color,alpha,depth);
	}

}

void play_grBufferSwap(FILE *infile)
{

	int swap_interval;
	fread(& swap_interval, sizeof(int),1,infile);
	grBufferSwap(swap_interval);

	if (ScreenEcho)
	{
		printf("grBufferSwap(%u)\n",swap_interval);
	}

}

void play_grChromakeyMode(FILE *infile)
{

	GrChromakeyMode_t mode;
	fread(& mode, sizeof(GrChromakeyMode_t),1,infile);
	grChromakeyMode(mode);

	if (ScreenEcho)
	{
		printf("grChromakeyMode(%u)\n",mode);
	}

}

void play_grChromakeyValue(FILE *infile)
{

	GrColor_t value;
	fread(& value, sizeof(GrColor_t),1,infile);
	grChromakeyValue(value);

	if (ScreenEcho)
	{
		printf("grChromakeyValue(%X)\n",value);
	}

}

void play_grClipWindow(FILE *infile)
{

	FxU32 minx;
	FxU32 miny;
	FxU32 maxx;
	FxU32 maxy;
	fread(& minx, sizeof(FxU32),1,infile);
	fread(& miny, sizeof(FxU32),1,infile);
	fread(& maxx, sizeof(FxU32),1,infile);
	fread(& maxy, sizeof(FxU32),1,infile);
	grClipWindow(minx, miny, maxx, maxy);

	if (ScreenEcho)
	{
		printf("grClipWindow(%u,%u,%u,%u)\n",minx,miny,maxx,maxy);
	}

}

void play_grColorCombine(FILE *infile)
{

	GrCombineFunction_t function;
	GrCombineFactor_t factor;
	GrCombineLocal_t local;
	GrCombineOther_t other;
	FxBool invert;

	fread(& function, sizeof(GrCombineFunction_t),1,infile);
	fread(& factor, sizeof(GrCombineFactor_t),1,infile);
	fread(& local, sizeof(GrCombineLocal_t),1,infile);
	fread(& other, sizeof(GrCombineOther_t),1,infile);
	fread(& invert, sizeof(FxBool),1,infile);
	grColorCombine(function, factor, local, other, invert);

	if (ScreenEcho)
	{
		printf("grColorCombine(%u,%u,%u,%u,%u)\n",function,factor,local,other,invert);
	}

}

void play_grColorMask(FILE *infile)
{

	FxBool rgb;
	FxBool a;
	fread(& rgb, sizeof(FxBool),1,infile);
	fread(& a, sizeof(FxBool),1,infile);

	grColorMask(rgb, a);

	if (ScreenEcho)
	{
		printf("grColorMask()\n");
	}

}

void play_grConstantColorValue(FILE *infile)
{

	GrColor_t value;
	fread(& value, sizeof(GrColor_t),1,infile);
	grConstantColorValue(value);

	if (ScreenEcho)
	{
		printf("grConstantColorValue(%X)\n",value);
	}

}

void play_grCoordinateSpace(FILE *infile)
{

	
}

void play_grCullMode(FILE *infile)
{

	GrCullMode_t mode;

	fread(& mode, sizeof(GrCullMode_t),1,infile);
	grCullMode(mode);

	if (ScreenEcho)
	{
		printf("grCullMode(%u)\n",mode);
	}

}

void play_grDepthBiasLevel(FILE *infile)
{

	FxI32 level;
	fread(& level, sizeof(FxI32),1,infile);
//	grDepthBiasLevel(level);

	if (ScreenEcho)
	{
		printf("grDepthBiasLevel(%i)\n",level);
	}

}

void play_grDepthBufferFunction(FILE *infile)
{
	GrCmpFnc_t function;

	fread(& function, sizeof(GrCmpFnc_t),1,infile);
	grDepthBufferFunction(function);

	if (ScreenEcho)
	{
		printf("grDepthBufferFunction(%u)\n",function);
	}

}

void play_grDepthBufferMode(FILE *infile)
{
	GrDepthBufferMode_t mode;

	fread(& mode, sizeof(GrDepthBufferMode_t),1,infile);
	grDepthBufferMode(mode);

	if (ScreenEcho)
	{
		printf("grDepthBufferMode(%u)\n",mode);
	}

}

void play_grDepthMask(FILE *infile)
{

	FxBool mask;
	fread(& mask, sizeof(FxBool),1,infile);
	grDepthMask(mask);

	if (ScreenEcho)
	{
		printf("grDepthMask(%u)\n",mask);
	}

}

void play_grDepthRange(FILE *infile)
{

	FxFloat n;
	FxFloat f;
	fread(& n, sizeof(FxFloat),1,infile);
	fread(& f, sizeof(FxFloat),1,infile);
	//grDepthRange(n, f);

	if (ScreenEcho)
	{
		printf("grDepthRange(%f,%f)\n",n,f);
	}

}

void play_grDisable(FILE *infile)
{

}

void play_grDisableAllEffects(FILE *infile)
{

	grDisableAllEffects();

	if (ScreenEcho)
	{
		printf("grDisableAllEffects()\n");
	}

}

void play_grDitherMode(FILE *infile)
{

	GrDitherMode_t mode;
	fread(& mode, sizeof(GrDitherMode_t),1,infile);
	grDitherMode(mode);

	if (ScreenEcho)
	{
		printf("grDitherMode(%u)\n",mode);
	}

}

void play_grDrawLine(FILE *infile)
{
	my_inputvnalloc(&vtxa[0],infile);
	my_inputvnalloc(&vtxb[0],infile);
//	if (!nodraw) grDrawLine(&vtxa[0],&vtxb[0]);

	if (ScreenEcho)
	{
		printf("grDrawLine( (%4.f,% 4.f) - (%4.f,%4.f) )\n",
			*(float *) (gc.vertexInfo.offset + (int) &vtxa[0]),
			*(float *) (gc.vertexInfo.offset + 4+(int) &vtxa[0]),
			*(float *) (gc.vertexInfo.offset + (int) &vtxb[0]),
			*(float *) (gc.vertexInfo.offset + 4+(int) &vtxb[0]));
	}

}

void play_grDrawPoint(FILE *infile)
{
	my_inputvnalloc(&vtxa[0],infile);
//	if (!nodraw) grDrawPoint(&vtxa[0]);

	if (ScreenEcho)
	{
		printf("grDrawPoint( %4.f,%4.f",		
			*(float *) (gc.vertexInfo.offset + (int) &vtxa[0]),
			*(float *) (gc.vertexInfo.offset + 4+(int) &vtxa[0]));
		printf(" )\n");
	}

}

void play_grDrawTriangle(FILE *infile)
{
	GrVertex a,b,c;
	my_inputvnalloc(&vtxa[0],infile);
	my_inputvnalloc(&vtxb[0],infile);
	my_inputvnalloc(&vtxc[0],infile);

	transformvertex(&vtxa[0],&a);
	transformvertex(&vtxb[0],&b);
	transformvertex(&vtxc[0],&c);
	
	if (ScreenEcho)
	{
		printf("grDrawTriangle( ");
		printf(" (%4.f,%4.f) ",*(float *) (gc.vertexInfo.offset + (int) &vtxa[0]),*(float *) (gc.vertexInfo.offset + 4+(int) &vtxa[0]));
		printf(" (%4.f,%4.f) ",*(float *) (gc.vertexInfo.offset + (int) &vtxb[0]),*(float *) (gc.vertexInfo.offset + 4+(int) &vtxb[0]));
		printf(" (%4.f,%4.f) ",*(float *) (gc.vertexInfo.offset + (int) &vtxc[0]),*(float *) (gc.vertexInfo.offset + 4+(int) &vtxc[0]));
		printf(" )\n");
	}

	if (!nodraw) grDrawTriangle(&a,&b,&c);	
}

void play_grDrawVertexArray(FILE *infile)
{

	FxU32 mode,i;
	FxU32 Count;	
	void **pointers;	

	fread(& mode, sizeof(FxU32),1,infile);
	fread(& Count, sizeof(FxU32),1,infile);
	pointers=(void **) malloc(sizeof(void*)*Count);
	for (i=0; i<Count; i++)
	{
		pointers[i]=my_inputvertex(infile);
	}
	//if (!nodraw) grDrawVertexArray(mode, Count, pointers);
	for (i=0; i<Count; i++)
	{
		free(pointers[i]);
	}
	free(pointers);

	if (ScreenEcho)
	{
		printf("grDrawVertexArray()\n");
	}

}

void play_grDrawVertexArrayContiguous(FILE *infile)
{

	FxU32 mode;
	FxU32 Count;
	void *pointers;
	FxU32 stride;

	fread(& mode, sizeof(FxU32),1,infile);
	fread(& Count, sizeof(FxU32),1,infile);	
	fread(& stride, sizeof(FxU32),1,infile);

	pointers=malloc(stride*Count);
	fread(pointers,stride*Count,1,infile);

	//if (!nodraw) grDrawVertexArrayContiguous(mode, Count, pointers, stride);
	free(pointers);

	if (ScreenEcho)
	{
		printf("grDrawVertexArrayContiguous()\n");
	}

}

void play_grEnable(FILE *infile)
{

//	GrEnableMode_t mode;
	//fread(& mode, sizeof(GrEnableMode_t),1,infile);
	//grEnable(mode);

	if (ScreenEcho)
	{
//		printf("grEnable(%u)\n",mode);
	}

}

void play_grErrorSetCallback(FILE *infile)
{
/*
	GrErrorCallbackFnc_t fnc;
	fread(& fnc, sizeof(GrErrorCallbackFnc_t),1,infile);
	grErrorSetCallback(fnc);
*/
	if (ScreenEcho)
	{
		printf("grErrorSetCallback()\n");
	}

}

void play_grFinish(FILE *infile)
{
	//grFinish();

	if (ScreenEcho)
	{
		printf("grFinish()\n");
	}

}

void play_grFlush(FILE *infile)
{
	//grFlush();

	if (ScreenEcho)
	{
		printf("grFlush()\n");
	}

}

void play_grFogColorValue(FILE *infile)
{

	GrColor_t fogcolor;

	fread(& fogcolor, sizeof(GrColor_t),1,infile);
	grFogColorValue(fogcolor);

	if (ScreenEcho)
	{
		printf("grFogColorValue(%X)\n",fogcolor);
	}

}

void play_grFogMode(FILE *infile)
{

	GrFogMode_t mode;
	fread(& mode, sizeof(GrFogMode_t),1,infile);
	grFogMode(mode);

	if (ScreenEcho)
	{
		printf("grFogMode(%u)\n",mode);
	}

}

void play_grFogTable(FILE *infile)
{
	FxU32 tsize;

	GrFog_t *ft;

	fread(&tsize,sizeof(FxU32),1,infile);

	ft=(GrFog_t *) malloc(tsize*sizeof(GrFog_t));
	fread(&ft[0],sizeof(GrFog_t),tsize,infile);

	grFogTable(ft);
	free(ft);
	if (ScreenEcho)
	{
		printf("grFogTable()\tFog table size=%u\n",tsize);
	}


}

void play_grGet(FILE *infile)
{
	FxU32 pname;
	FxU32 plength;	

	fread(& pname, sizeof(FxU32),1,infile);
	fread(& plength, sizeof(FxU32),1,infile);
}

void play_grGetProcAddress(FILE *infile)
{
	int size;
	char *procName;	

	fread(&size,sizeof(int),1,infile);
	procName=(char *) malloc(sizeof(char)*(size+2));
	fread(procName,sizeof(char),size,infile);
	free(procName);
}

void play_grGetString(FILE *infile)
{
	FxU32 pname;	
	fread(& pname, sizeof(FxU32),1,infile);
}

void play_grGlideGetState(FILE *infile)
{
	FxU32 statenum,i;
	void *state;
	
	if (state_size==0)
	{
		printf("Trapping program tried to grGlideGetState without doing a grGet(GR_GLIDE_STATE_SIZE)!\n");
		exit(1);
	}

	state=(void *) malloc(sizeof(char)*state_size);
	memset(state,0,sizeof(char)*state_size);
	grGlideGetState(state);

	fread(&statenum,sizeof(FxU32),1,infile);

	if (statenum>=numstates)
	/* this is a new one! */
	{
		numstates++;
		if (numstates>=state_table_size)
		{
			realloc(play_states,2*sizeof(void*)*state_table_size);
			for (i=state_table_size; i<2*state_table_size;i++)
				play_states[i]=NULL;
			state_table_size=2*state_table_size;
		}
		play_states[statenum]=state;		
	} else
	{ free(state); }

	if (ScreenEcho)
	{
		printf("grGlideGetState()\tState: %u\n",statenum);
	}	
}

void play_grGlideGetVertexLayout(FILE *infile)
{
}

void play_grGlideInit(FILE *infile)
{
	grGlideInit();
	if (ScreenEcho)
	{
		printf("grGlideInit()\n");
	}

}

void play_grGlideSetState(FILE *infile)
{
	FxU32 token;
	
	fread(&token, sizeof(FxU32),1,infile);
	
	if (token>numstates)
	{
		printf("grGlideSetState(): ERROR! Invalid State: %u\n",token);
		getch();
		exit(1);
	} else
	{
		grGlideSetState(play_states[token]);
		if (ScreenEcho)	printf("grGlideSetState(State: %u)\n",token);	}

}

void play_grGlideSetVertexLayout(FILE *infile)
{

	fread(&gc,sizeof(GrVertexLayoutInfo),1,infile);
	
	//grGlideSetVertexLayout(&gc);

	if (ScreenEcho)
	{
		printf("grGlideSetVertexLayout()\n");
	}

}

void play_grGlideShutdown(FILE *infile)
{

	grGlideShutdown();
	if (ScreenEcho)
	{
		printf("grGlideShutdown()\n");
	}

}

void play_grLfbConstantAlpha(FILE *infile)
{

	GrAlpha_t alpha;
	fread(& alpha, sizeof(GrAlpha_t),1,infile);
	grLfbConstantAlpha(alpha);

	if (ScreenEcho)
	{
		printf("grLfbConstantAlpha(%u)\n",alpha);
	}

}

void play_grLfbConstantDepth(FILE *infile)
{

	FxU32 depth;
	fread(& depth, sizeof(FxU32),1,infile);
//	grLfbConstantDepth(depth);

	if (ScreenEcho)
	{
		printf("grLfbConstantDepth(%u)\n",depth);
	}

}

void play_grLfbLock(FILE *infile)
{

	GrLock_t type;
	GrBuffer_t buffer;
	GrLfbWriteMode_t writeMode;
	GrOriginLocation_t origin;
	FxBool pixelPipeline;
	FxBool result,res;

	fread(& type, sizeof(GrLock_t),1,infile);
	fread(& buffer, sizeof(GrBuffer_t),1,infile);
	fread(& writeMode, sizeof(GrLfbWriteMode_t),1,infile);
	fread(& origin, sizeof(GrOriginLocation_t),1,infile);
	fread(& pixelPipeline, sizeof(FxBool),1,infile);
	fread(& result, sizeof(FxBool),1,infile);
	
	if (!result)
	{
		printf("Trap hardware failed on grLfbLock(). Ignoring lock!\n");		
	}
	else
	{
		res=FXFALSE;
		while (!res)
		{
			switch (buffer)
			{
			case GR_BUFFER_FRONTBUFFER:
				res=frontlocked=grLfbLock(type, buffer, writeMode, origin, pixelPipeline, &lfbf);
				break;
			case GR_BUFFER_BACKBUFFER:
				res=backlocked=grLfbLock(type, buffer, writeMode, origin, pixelPipeline, &lfbb);
				break;
			case GR_BUFFER_AUXBUFFER:
				res=auxlocked=grLfbLock(type, buffer, writeMode, origin, pixelPipeline, &lfba);
				break;
			default:
				printf("Tried to grLfbLock()unknown buffer %u !!",buffer);
				exit(1);
				break;
			} /* of switch */
			if (!res) {printf("Locked failed! Retrying...\n"); }
		} /* while */
	}
	

	if (ScreenEcho)
	{
		printf("grLfbLock(%u,%u,%u,%u,%u)\tReturned %u\n",type,buffer,writeMode,origin,pixelPipeline,res);
	}

}

void play_grLfbReadRegion(FILE *infile)
{

	GrBuffer_t src_buffer;
	FxU32 src_x;
	FxU32 src_y;
	FxU32 src_width;
	FxU32 src_height;
	FxU32 dst_stride;
	void *dst_data;

	fread(& src_buffer, sizeof(GrBuffer_t),1,infile);
	fread(& src_x, sizeof(FxU32),1,infile);
	fread(& src_y, sizeof(FxU32),1,infile);
	fread(& src_width, sizeof(FxU32),1,infile);
	fread(& src_height, sizeof(FxU32),1,infile);
	fread(& dst_stride, sizeof(FxU32),1,infile);
	
	dst_data=malloc(src_height*dst_stride);
	grLfbReadRegion(src_buffer, src_x, src_y, src_width, src_height, dst_stride, dst_data);
	free(dst_data);

	if (ScreenEcho)
	{
		printf("grLfbReadRegion()\n");
	}

}

void play_grLfbUnlock(FILE *infile)
{

	GrLock_t type;
	GrBuffer_t buffer;
	
	fread(& type, sizeof(GrLock_t),1,infile);
	fread(& buffer, sizeof(GrBuffer_t),1,infile);

	switch (buffer)
	{
	case GR_BUFFER_FRONTBUFFER:
		if (!frontlocked)
		{
			printf("FATAL ERROR: Tried to unlock Front Buffer without lock!\n");
			exit(1);
		}		
		frontlocked=FXFALSE;
		break;
	case GR_BUFFER_BACKBUFFER:
		if (!backlocked)
		{
			printf("FATAL ERROR: Tried to unlock Back Buffer without lock!\n");
			exit(1);
		}		
		backlocked=FXFALSE;
		break;

	case GR_BUFFER_AUXBUFFER:
		if (!auxlocked)
		{
			printf("FATAL ERROR: Tried to unlock Aux Buffer without lock!\n");
			exit(1);
		}		
		auxlocked=FXFALSE;
		break;
	default:
		printf("FATAL ERROR: Tried to unlock unknown buffer %u!\n",buffer);
		break;
	}

	grLfbUnlock(type, buffer);

	if (ScreenEcho)
	{
		printf("grLfbUnlock(%u,%u)\n",type,buffer);
	}

}

void play_grLfbWriteColorFormat(FILE *infile)
{

	GrColorFormat_t colorFormat;
	fread(& colorFormat, sizeof(GrColorFormat_t),1,infile);
	grLfbWriteColorFormat(colorFormat);

	if (ScreenEcho)
	{
		printf("grLfbWriteColorFormat(%u)\n",colorFormat);
	}

}

void play_grLfbWriteColorSwizzle(FILE *infile)
{

	FxBool swizzleBytes;
	FxBool swapWords;
	fread(& swizzleBytes, sizeof(FxBool),1,infile);
	fread(& swapWords, sizeof(FxBool),1,infile);
	grLfbWriteColorSwizzle(swizzleBytes, swapWords);

	if (ScreenEcho)
	{
		printf("grLfbWriteColorSwizzle(%u,%u)\n",swizzleBytes,swapWords);
	}

}

void play_grLfbWriteRegion(FILE *infile)
{

	GrBuffer_t dst_buffer;
	FxU32 dst_x;
	FxU32 dst_y;
	GrLfbSrcFmt_t src_format;
	FxU32 src_width;
	FxU32 src_height;
	FxBool pixelPipeline;
	FxI32 src_stride;
	FxBool invalid;
	void * src_data;
	char filename[TEXNAMELEN+1];

	fread(& dst_buffer, sizeof(GrBuffer_t),1,infile);
	fread(& dst_x, sizeof(FxU32),1,infile);
	fread(& dst_y, sizeof(FxU32),1,infile);
	fread(& src_format, sizeof(GrLfbSrcFmt_t),1,infile);
	fread(& src_width, sizeof(FxU32),1,infile);
	fread(& src_height, sizeof(FxU32),1,infile);
	fread(& pixelPipeline, sizeof(FxBool),1,infile);
	fread(& src_stride, sizeof(FxI32),1,infile);

	fread(&filename[0],TEXNAMELEN,1,infile);

	invalid=!strncmp(filename,INVALID,strlen(INVALID));

	if (invalid)
	{
		//grLfbWriteRegion(dst_buffer, dst_x, dst_y, src_format, src_width, src_height, pixelPipeline, src_stride, NULL);		
	}
	else
	{
		src_data=load_lfb(filename);
		//grLfbWriteRegion(dst_buffer, dst_x, dst_y, src_format, src_width, src_height, pixelPipeline, src_stride, src_data);		
		free(src_data);
	}

	if (ScreenEcho)
	{
		printf("grLfbWriteRegion()\tFilename: %s\n",filename);		
	}
}

void play_grLoadGammaTable(FILE *infile)
{

	FxU32 nentries;
	FxU32 *red;
	FxU32 *green;
	FxU32 *blue;
	fread(& nentries, sizeof(FxU32),1,infile);

	red=malloc(sizeof(FxU32)*nentries);
	green=malloc(sizeof(FxU32)*nentries);
	blue=malloc(sizeof(FxU32)*nentries);

	fread(red,sizeof(FxU32),nentries,infile);
	fread(green,sizeof(FxU32),nentries,infile);
	fread(blue,sizeof(FxU32),nentries,infile);

	//grLoadGammaTable(nentries, red, green, blue);
	free(red);
	free(green);
	free(blue);

	if (ScreenEcho)
	{
		printf("grLoadGammaTable()\n");
	}

}

void play_grQueryResolutions(FILE *infile)
{


}

void play_grRenderBuffer(FILE *infile)
{

	GrBuffer_t buffer;

	fread(& buffer, sizeof(GrBuffer_t),1,infile);

	grRenderBuffer(buffer);

	if (ScreenEcho)
	{
		printf("grRenderBuffer(%u)\n",buffer);
	}

}

void play_grReset(FILE *infile)
{

	FxU32 what;
	fread(& what, sizeof(FxU32),1,infile);
	//grReset(what);

	if (ScreenEcho)
	{
		printf("grReset(%u)\n",what);
	}

}

void play_grSelectContext(FILE *infile)
{

}

void play_grSplash(FILE *infile)
{

	float x;
	float y;
	float width;
	float height;
	FxU32 frame;
	fread(& x, sizeof(float),1,infile);
	fread(& y, sizeof(float),1,infile);
	fread(& width, sizeof(float),1,infile);
	fread(& height, sizeof(float),1,infile);
	fread(& frame, sizeof(FxU32),1,infile);

	grSplash(x, y, width, height, frame);

	if (ScreenEcho)
	{
		printf("grSplash(%f,%f,%f,%f,%u)\n",x,y,width,height,frame);
	}

}

void play_grSstOrigin(FILE *infile)
{

	GrOriginLocation_t origin;
	fread(& origin, sizeof(GrOriginLocation_t),1,infile);
	grSstOrigin(origin);

	if (ScreenEcho)
	{
		printf("grSstOrigin(%u)\n",origin);
	}

}

void play_grSstSelect(FILE *infile)
{

	int which_sst;
	fread(& which_sst, sizeof(int),1,infile);
	grSstSelect(which_sst);

	if (ScreenEcho)
	{
		printf("grSstSelect(%i)\n",which_sst);
	}

}

void play_grSstWinClose(FILE *infile)
{
	FxU32 context;
	fread(& context, sizeof(FxU32),1,infile);
	
	grSstWinClose();
}

void play_grSstWinOpen(FILE *infile)
{

	FxU32 hWnd;
	GrScreenResolution_t screen_resolution;
	GrScreenRefresh_t refresh_rate;
	GrColorFormat_t color_format;
	GrOriginLocation_t origin_location;
	int nColBuffers;
	int nAuxBuffers;

	fread(& hWnd, sizeof(FxU32),1,infile);
	fread(& screen_resolution, sizeof(GrScreenResolution_t),1,infile);
	fread(& refresh_rate, sizeof(GrScreenRefresh_t),1,infile);
	fread(& color_format, sizeof(GrColorFormat_t),1,infile);
	fread(& origin_location, sizeof(GrOriginLocation_t),1,infile);
	fread(& nColBuffers, sizeof(int),1,infile);
	fread(& nAuxBuffers, sizeof(int),1,infile);
	grSstWinOpen(0, screen_resolution, refresh_rate, color_format, origin_location, nColBuffers, nAuxBuffers);

	if (ScreenEcho)
	{
		printf("grSstWinOpen(%i,%i,%i,%i,%i,%i,%i) -- This computer calls with param 0=0\n",hWnd,screen_resolution,
			refresh_rate,color_format,origin_location,nColBuffers,nAuxBuffers);
	}

}

void play_grTexCalcMemRequired(FILE *infile)
{

	FxU32 result;
	GrLOD_t lodmin;
	GrLOD_t lodmax;
	GrAspectRatio_t aspect;
	GrTextureFormat_t fmt;
	fread(& lodmin, sizeof(GrLOD_t),1,infile);
	fread(& lodmax, sizeof(GrLOD_t),1,infile);
	fread(& aspect, sizeof(GrAspectRatio_t),1,infile);
	fread(& fmt, sizeof(GrTextureFormat_t),1,infile);
	result=grTexCalcMemRequired(lodmin, lodmax, aspect, fmt);

	if (ScreenEcho)
	{
		printf("grTexCalcMemRequired(%i,%i,%i,%i)\tReturned: %i\n",lodmin,lodmax,aspect,fmt,result);
	}

}

void play_grTexClampMode(FILE *infile)
{

	GrChipID_t tmu;
	GrTextureClampMode_t s_clampmode;
	GrTextureClampMode_t t_clampmode;
 
	fread(& tmu, sizeof(GrChipID_t),1,infile);
	fread(& s_clampmode, sizeof(GrTextureClampMode_t),1,infile);
	fread(& t_clampmode, sizeof(GrTextureClampMode_t),1,infile);
	grTexClampMode(tmu, s_clampmode, t_clampmode);

	if (ScreenEcho)
	{
		printf("grTexClampMode(%u, %u, %u)\n",tmu, s_clampmode, t_clampmode);
	}

}

void play_grTexCombine(FILE *infile)
{

	GrChipID_t tmu;
	GrCombineFunction_t rgb_function;
	GrCombineFactor_t rgb_factor;
	GrCombineFunction_t alpha_function;
	GrCombineFactor_t alpha_factor;
	FxBool rgb_invert;
	FxBool alpha_invert;

	fread(& tmu, sizeof(GrChipID_t),1,infile);
	fread(& rgb_function, sizeof(GrCombineFunction_t),1,infile);
	fread(& rgb_factor, sizeof(GrCombineFactor_t),1,infile);
	fread(& alpha_function, sizeof(GrCombineFunction_t),1,infile);
	fread(& alpha_factor, sizeof(GrCombineFactor_t),1,infile);
	fread(& rgb_invert, sizeof(FxBool),1,infile);
	fread(& alpha_invert, sizeof(FxBool),1,infile);
	grTexCombine(tmu, rgb_function, rgb_factor, alpha_function, alpha_factor, rgb_invert, alpha_invert);

	if (ScreenEcho)
	{
		printf("grTexCombine(%u,%u,%u,%u,%u,%u)\n",tmu, rgb_function, rgb_factor, alpha_function, alpha_factor, rgb_invert, alpha_invert);
	}

}

void play_grTexDetailControl(FILE *infile)
{

	GrChipID_t tmu;
	int lod_bias;
	FxU8 detail_scale;
	float detail_max;

	fread(& tmu, sizeof(GrChipID_t),1,infile);
	fread(& lod_bias, sizeof(int),1,infile);
	fread(& detail_scale, sizeof(FxU8),1,infile);
	fread(& detail_max, sizeof(float),1,infile);

	grTexDetailControl(tmu, lod_bias, detail_scale, detail_max);

	if (ScreenEcho)
	{
		printf("grTexDetailControl(%u,%u,%u,%f)\n",tmu,lod_bias,detail_scale,detail_max);
	}

}


void play_grTexDownloadMipMap(FILE *infile)
{

	GrChipID_t tmu;
	FxU32 startAddress;
	FxU32 evenOdd;
	GrTexInfo *info;
	char filename[TEXNAMELEN+1];
	
	fread(& tmu, sizeof(GrChipID_t),1,infile);
	fread(& startAddress, sizeof(FxU32),1,infile);
	fread(& evenOdd, sizeof(FxU32),1,infile);
	info=(GrTexInfo *) malloc(sizeof(GrTexInfo));
	fread(info,sizeof(GrTexInfo),1,infile);	
	fread(&filename[0],TEXNAMELEN,1,infile);
	load_texture(filename,&info->data);	

	
	grTexDownloadMipMap(tmu, startAddress, evenOdd, info);

	if (ScreenEcho)
	{
		printf("grTexDownloadMipMap(%u,%u,%u) -- Loaded %s\n",tmu,startAddress,evenOdd,filename);
	}
	
	free(info->data);
	free(info);
}

void play_grTexDownloadMipMapLevel(FILE *infile)
{

	GrChipID_t tmu;
	FxU32 startAddress;
	GrLOD_t thisLod;
	GrLOD_t largeLod;
	GrAspectRatio_t aspectRatio;
	GrTextureFormat_t format;
	FxU32 evenOdd;
	void *data;
	char filename[TEXNAMELEN+1];

	fread(& tmu, sizeof(GrChipID_t),1,infile);
	fread(& startAddress, sizeof(FxU32),1,infile);
	fread(& thisLod, sizeof(GrLOD_t),1,infile);
	fread(& largeLod, sizeof(GrLOD_t),1,infile);
	fread(& aspectRatio, sizeof(GrAspectRatio_t),1,infile);
	fread(& format, sizeof(GrTextureFormat_t),1,infile);
	fread(& evenOdd, sizeof(FxU32),1,infile);
	fread(&filename[0],TEXNAMELEN,1,infile);

	data=NULL;
	load_texture(filename,&data);
	ASPECT(aspectRatio);
	LOD(thisLod);
	LOD(largeLod);

	if (ScreenEcho)
	{
		printf("grTexDownloadMipMapLevel(TMU %u, Start %u,this %u, large %u,a %u) File %s\n",tmu,startAddress,thisLod,
			largeLod,aspectRatio,filename);
	}

	if (data==NULL)
	{
		printf("Texture did not load!\n");
		getch();
	}
	
	grTexDownloadMipMapLevel(tmu, startAddress, thisLod, largeLod, aspectRatio, format, evenOdd, data);

	free(data);

}

void play_grTexDownloadMipMapLevelPartial(FILE *infile)
{

	GrChipID_t tmu;
	FxU32 startAddress;
	GrLOD_t thisLod;
	GrLOD_t largeLod;
	GrAspectRatio_t aspectRatio;
	GrTextureFormat_t format;
	FxU32 evenOdd;
	void *data;
	int start;
	int end;
	char filename[TEXNAMELEN+1];

	fread(& tmu, sizeof(GrChipID_t),1,infile);
	fread(& startAddress, sizeof(FxU32),1,infile);
	fread(& thisLod, sizeof(GrLOD_t),1,infile);
	fread(& largeLod, sizeof(GrLOD_t),1,infile);
	fread(& aspectRatio, sizeof(GrAspectRatio_t),1,infile);
	fread(& format, sizeof(GrTextureFormat_t),1,infile);
	fread(& evenOdd, sizeof(FxU32),1,infile);	
	fread(& start, sizeof(int),1,infile);
	fread(& end, sizeof(int),1,infile);
	fread(&filename[0],TEXNAMELEN,1,infile);

	load_texture(filename,&data);
	
	grTexDownloadMipMapLevelPartial(tmu, startAddress, thisLod, largeLod, aspectRatio, format, evenOdd, data, start, end);

	if (ScreenEcho)
	{
		printf("grTexDownloadMipMapLevelPartial()\n");
	}

	free(data);

}

void play_grTexDownloadTable(FILE *infile)
{

	GrTexTable_t type;	

	fread(& type, sizeof(GrTexTable_t),1,infile);
	if ((type == GR_TEXTABLE_PALETTE))
	{
		fread(&mytexpalette,sizeof(GuTexPalette),1,infile);
		grTexDownloadTable(0,type, &mytexpalette);
	}
	else 
	{
		fread(&myncctable,sizeof(GuNccTable),1,infile);
		grTexDownloadTable(0,type, &myncctable);
	}

	if (ScreenEcho)
	{
		printf("grTexDownloadTable(%u)\n",type);
	}

}

void play_grTexDownloadTablePartial(FILE *infile)
{

}

void play_grTexFilterMode(FILE *infile)
{

	GrChipID_t tmu;
	GrTextureFilterMode_t minfilter_mode;
	GrTextureFilterMode_t magfilter_mode;
	fread(& tmu, sizeof(GrChipID_t),1,infile);
	fread(& minfilter_mode, sizeof(GrTextureFilterMode_t),1,infile);
	fread(& magfilter_mode, sizeof(GrTextureFilterMode_t),1,infile);

	grTexFilterMode(tmu, minfilter_mode, magfilter_mode);

	if (ScreenEcho)
	{
		printf("grTexFilterMode(%u,%u,%u)\n",tmu, minfilter_mode, magfilter_mode);
	}

}

void play_grTexLodBiasValue(FILE *infile)
{

	GrChipID_t tmu;
	float bias;

	fread(& tmu, sizeof(GrChipID_t),1,infile);
	fread(& bias, sizeof(float),1,infile);
	grTexLodBiasValue(tmu, bias);

	if (ScreenEcho)
	{
		printf("grTexLodBiasValue(%u,%f)\n",tmu, bias);
	}

}

void play_grTexMaxAddress(FILE *infile)
{

	GrChipID_t tmu;
	FxU32 res;
	fread(& tmu, sizeof(GrChipID_t),1,infile);

	res=grTexMaxAddress(tmu);

	if (ScreenEcho)
	{
		printf("grTexMaxAddress(%i)\tReturned:%u\n",tmu,res);
	}

}

void play_grTexMinAddress(FILE *infile)
{
	FxU32 res;

	GrChipID_t tmu;
	fread(& tmu, sizeof(GrChipID_t),1,infile);
	res=grTexMinAddress(tmu);

	if (ScreenEcho)
	{
		printf("grTexMinAddress(%u)\tReturned: %u\n",tmu,res);
	}

}

void play_grTexMipMapMode(FILE *infile)
{

	GrChipID_t tmu;
	GrMipMapMode_t mode;
	FxBool lodBlend;

	fread(& tmu, sizeof(GrChipID_t),1,infile);
	fread(& mode, sizeof(GrMipMapMode_t),1,infile);
	fread(& lodBlend, sizeof(FxBool),1,infile);
	grTexMipMapMode(tmu, mode, lodBlend);

	if (ScreenEcho)
	{
		printf("grTexMipMapMode(%u,%u,%u)\n",tmu, mode, lodBlend);
	}

}

void play_grTexMultibase(FILE *infile)
{

	GrChipID_t tmu;
	FxBool enable;
	fread(& tmu, sizeof(GrChipID_t),1,infile);
	fread(& enable, sizeof(FxBool),1,infile);
	grTexMultibase(tmu, enable);

	if (ScreenEcho)
	{
		printf("grTexMultibase(%u,%u)\n",tmu,enable);
	}

}

void play_grTexMultibaseAddress(FILE *infile)
{

	GrChipID_t tmu;
	GrTexBaseRange_t range;
	FxU32 startAddress;
	FxU32 evenOdd;
	GrTexInfo info;
	fread(& tmu, sizeof(GrChipID_t),1,infile);
	fread(& range, sizeof(GrTexBaseRange_t),1,infile);
	fread(& startAddress, sizeof(FxU32),1,infile);
	fread(& evenOdd, sizeof(FxU32),1,infile);
	fread(&info,sizeof(GrTexInfo),1,infile);
	grTexMultibaseAddress(tmu, range, startAddress, evenOdd, &info);

	if (ScreenEcho)
	{
		printf("grTexMultibaseAddress()\n");
	}

}

void play_grTexNCCTable(FILE *infile)
{

	GrNCCTable_t table;
	fread(& table, sizeof(GrNCCTable_t),1,infile);
//	grTexNCCTable(table);

	if (ScreenEcho)
	{
		printf("grTexNCCTable(%u)\n",table);
	}

}

void play_grTexSource(FILE *infile)
{

	GrChipID_t tmu;
	FxU32 startAddress;
	FxU32 evenOdd;
	GrTexInfo *info;		

	fread(& tmu, sizeof(GrChipID_t),1,infile);
	fread(& startAddress, sizeof(FxU32),1,infile);
	fread(& evenOdd, sizeof(FxU32),1,infile);
	info=(GrTexInfo *) malloc(sizeof(GrTexInfo));
	fread(info,sizeof(GrTexInfo),1,infile);
	info->data=NULL;

	/* glide3 to glide 2 translation */
	LOD(info->smallLod);
	LOD(info->largeLod);
	ASPECT(info->aspectRatio);
	
	grTexSource(tmu, startAddress, evenOdd, info);	
	
	if (ScreenEcho)
	{
		printf("grTexSource(%u,%u,%u)\tInfo: \n",tmu,startAddress,evenOdd);
	}
	free(info);
}

void play_grTexTextureMemRequired(FILE *infile)
{
	FxU32 size;
	FxU32 evenOdd;
	GrTexInfo *info;

	info=(GrTexInfo *) malloc(sizeof(GrTexInfo));
	fread(& evenOdd, sizeof(FxU32),1,infile);
	fread(info,sizeof(GrTexInfo),1,infile);
	
	size=grTexTextureMemRequired(evenOdd, info);
	free(info);

	if (ScreenEcho)
	{
		printf("grTexTextureMemRequired(%u)\tReturned: %u\n",evenOdd,size);
	}

}

void play_grVertexLayout(FILE *infile)
{

	FxU32 param;
	FxI32 offset;
	FxU32 mode;
	fread(& param, sizeof(FxU32),1,infile);
	fread(& offset, sizeof(FxI32),1,infile);
	fread(& mode, sizeof(FxU32),1,infile);
	my_grVertexLayout(param, offset, mode);
	if (param==GR_PARAM_ST1 && mode==GR_PARAM_ENABLE)
	{	
		grHints(GR_HINT_STWHINT, GR_STWHINT_ST_DIFF_TMU1);
	}

	if (param==GR_PARAM_Q0 && mode==GR_PARAM_ENABLE)
	{   
		grHints(GR_HINT_STWHINT, GR_STWHINT_W_DIFF_TMU0);
    }

	if (ScreenEcho)
	{
		printf("grVertexLayout(%i,%i,%i)\n",param,offset,mode);
	}
}

void play_grViewport(FILE *infile)
{

	FxI32 x;
	FxI32 y;
	FxI32 width;
	FxI32 height;

	fread(& x, sizeof(FxI32),1,infile);
	fread(& y, sizeof(FxI32),1,infile);
	fread(& width, sizeof(FxI32),1,infile);
	fread(& height, sizeof(FxI32),1,infile);
	//grViewport(x, y, width, height);

	if (ScreenEcho)
	{
		printf("grViewport()\n");
	}

}

void play_gu3dfGetInfo(FILE *infile)
{
	int size;
	char *filename;
	Gu3dfInfo info;
	
	fread(&size,sizeof(int),1,infile);
	filename=(char *) malloc(sizeof(char)*(size+1));
	fread(filename,sizeof(char),size,infile);
	filename[size]=0;
	gu3dfGetInfo(filename, &info);


	if (ScreenEcho)
	{
		printf("gu3dfGetInfo(%s)\n",filename);
	}
	free(filename);
}

void play_gu3dfLoad(FILE *infile)
{
	int size;
	char *filename;
	
	fread(&size,sizeof(int),1,infile);
	filename=(char *) malloc(sizeof(char)*(size+1));
	fread(filename,sizeof(char),size,infile);
	filename[size]=0;

	/* gu3dfLoad(*filename, *data); */

	if (ScreenEcho)
	{
		printf("NOT CALLED! gu3dfLoad(%s)\n",filename);
	}

	free(filename);

}

void play_guFogGenerateExp(FILE *infile)
{

	GrFog_t fogtable[BIG_NUM];
	float density;

	fread(& density, sizeof(float),1,infile);
	guFogGenerateExp(fogtable, density);
	

	if (ScreenEcho)
	{
		printf("guFogGenerateExp(%f)\n",density);
	}

}

void play_guFogGenerateExp2(FILE *infile)
{

	GrFog_t fogtable[BIG_NUM];
	float density;

	fread(& density, sizeof(float),1,infile);
	guFogGenerateExp2(fogtable, density);

	if (ScreenEcho)
	{
		printf("guFogGenerateExp2(%f)\n",density);
	}

}

void play_guFogGenerateLinear(FILE *infile)
{

	GrFog_t fogtable[BIG_NUM];
	float nearZ;
	float farZ;
	
	fread(& nearZ, sizeof(float),1,infile);
	fread(& farZ, sizeof(float),1,infile);
	guFogGenerateLinear(fogtable, nearZ, farZ);

	if (ScreenEcho)
	{
		printf("guFogGenerateLinear(%f,%f)\n",nearZ,farZ);
	}

}

void play_guFogTableIndexToW(FILE *infile)
{

	int i;
	float result;

	fread(& i, sizeof(int),1,infile);
	result=guFogTableIndexToW(i);

	if (ScreenEcho)
	{
		printf("guFogTableIndexToW(%u)\tReturned:%f\n",i,result);
	}

}

void play_guGammaCorrectionRGB(FILE *infile)
{

	FxFloat red;
	FxFloat green;
	FxFloat blue;

	fread(& red, sizeof(FxFloat),1,infile);
	fread(& green, sizeof(FxFloat),1,infile);
	fread(& blue, sizeof(FxFloat),1,infile);

	//guGammaCorrectionRGB(red, green, blue);

	if (ScreenEcho)
	{
		printf("guGammaCorrectionRGB(%f,%f,%f)\n",red,green,blue);
	}

}

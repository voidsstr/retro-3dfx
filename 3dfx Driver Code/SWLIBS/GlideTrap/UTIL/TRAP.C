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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glide.h>
#ifdef __WIN32__
#include <conio.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include "trap.h"
#include "hash.h"
#include "texmem.h"

FxBool ScreenEcho;            /* Echo calls to screen or not */
FxBool fastcompare;            /* fast compare or exact compare */
FxBool nodraw;                /* skip drawing functions */
FILEBUFF *outfile;                // Our Binary Log File
GrVertexLayoutInfo gc;        /* Internal vertex layout representation */

HashTable *states;          /* keeps track of which state is being set or get */
void ** play_states;        /* keeps track of playback states */
List ** trap_states;        /* keeps track of trapped states */
FxU32 state_table_size;        /* size of state table */
FxU32 numstates;            /* total number of states (play or trap)*/
FxU32 state_size;           /* to avoid extra grGets, use the playback grGet(STATE_SIZE) */

HashTable *vlayouts;        /* keeps track of which vlayout is being set or get */
void ** play_vlayouts;      /* keeps track of playback vlayouts */
List ** trap_vlayouts;      /* keeps track of trapped vlayouts */
FxU32 vlayout_table_size;   /* size of vlayout table */
FxU32 numvlayouts;          /* total number of vlayouts (play or trap)*/
FxU32 vlayout_size;         /* to avoid extra grGets, use the playback grGet(VERTEXLAYOUT_SIZE) */

HashTable *textures;        /* keeps track of which texture is being used */
HashTable *lfbs;            /* keeps track of which lfb is being used */
char dir_base[MAX_PATH_LEN];    /* directory base for playback/trapping */
FxU32 vtxa[15],vtxb[15],vtxc[15]; /* pre allocate vertices for faster playback */
FxU32 frontlocked,backlocked,auxlocked; /* buffer lock status */
FxI32 sizeofres;            /* used for grqueryresolutions */
GrLfbInfo_t    lfbf,lfba,lfbb;                 /* frame buffer locks */
char MESSAGE[256];                         /* message buffer */
GlideState gstate;
GrScreenResolution_t g_screen_resolution;
GrContext_t play_context;

/* extensions functions */
/* NOTE: Had problems with MSVC 5.0 and having an array of GrProc extensions[] */
/* that's why i'm using individual function pointers :-( */

GrProc MYEXT_grChromaRangeModeExt;
GrProc MYEXT_grChromaRangeExt;
GrProc MYEXT_grTexChromaModeExt;
GrProc MYEXT_grTexChromaRangeExt;
GrProc MYEXT_grDrawTextureLineExt;

/* Because Glide internally may call it's own API, we need to take care
 * that we-disable recording whenever we call the real Glide DLL.
 */
FxBool okToRecord=1;

/* user defined portion */
void Init()
// Inits common to Playback and Trap
{
    /* Makes new hashtables */
    states   = MakeHashTable();
    vlayouts = MakeHashTable();
    textures = MakeHashTable();
    lfbs     = MakeHashTable();

    /* Init variables */
    play_states=NULL;
    state_size=0;
    numstates=0;

    play_vlayouts=NULL;
    vlayout_size=0;
    numvlayouts=0;

    sizeofres=0;

    /* Disable all the vertex stuff */
    my_grVertexLayout( GR_PARAM_XY,      0, GR_PARAM_DISABLE);
    my_grVertexLayout( GR_PARAM_Z,       0, GR_PARAM_DISABLE);
    my_grVertexLayout( GR_PARAM_W,       0, GR_PARAM_DISABLE);
    my_grVertexLayout( GR_PARAM_Q,       0, GR_PARAM_DISABLE);
    my_grVertexLayout( GR_PARAM_FOG_EXT, 0, GR_PARAM_DISABLE);
    my_grVertexLayout( GR_PARAM_A,       0, GR_PARAM_DISABLE);
    my_grVertexLayout( GR_PARAM_RGB,     0, GR_PARAM_DISABLE);
    my_grVertexLayout( GR_PARAM_PARGB,   0, GR_PARAM_DISABLE);
    my_grVertexLayout( GR_PARAM_ST0,     0, GR_PARAM_DISABLE);
    my_grVertexLayout( GR_PARAM_ST1,     0, GR_PARAM_DISABLE);
    my_grVertexLayout( GR_PARAM_ST2,     0, GR_PARAM_DISABLE);
    my_grVertexLayout( GR_PARAM_Q0,      0, GR_PARAM_DISABLE);
    my_grVertexLayout( GR_PARAM_Q1,      0, GR_PARAM_DISABLE);
    my_grVertexLayout( GR_PARAM_Q2,      0, GR_PARAM_DISABLE);

    MYEXT_grChromaRangeModeExt = NULL;
    MYEXT_grChromaRangeExt     = NULL;
    MYEXT_grTexChromaModeExt   = NULL;
    MYEXT_grTexChromaRangeExt  = NULL;
    MYEXT_grDrawTextureLineExt = NULL;
}

#ifdef __WIN32__
void CTrapInit(char * trapdir)
{
    char outfilen[MAX_PATH_LEN];    
    int i;

    Init();

    trap_states=malloc(sizeof(List*)*DARRAYSIZE);
    for (i=0; i<DARRAYSIZE; i++)
        trap_states[i]=NULL;
    state_table_size=DARRAYSIZE;

    trap_vlayouts=malloc(sizeof(List*)*DARRAYSIZE);
    for (i=0; i<DARRAYSIZE; i++)
        trap_vlayouts[i]=NULL;
    vlayout_table_size=DARRAYSIZE;

    /* open trap file */
    strcpy(dir_base,trapdir);
    strcpy(outfilen,dir_base);
    strcat(outfilen,TOKENF);    
    outfile=fileBuffOpen(outfilen, "wb");
}
#endif

FILEBUFF * CommandLinePlayInit(int argc, char *argv[],
                           int *framestart, int *frameend)
{
    FILEBUFF *result;
    int i;
    char infile[MAX_PATH_LEN];

    Init();

    strcpy(dir_base, argv[1]);
    if (*dir_base &&
        (dir_base[strlen(dir_base)-1] != '\\') &&
        (dir_base[strlen(dir_base)-1] != '/'))
      strcat(dir_base, "/");

    strcpy(infile, dir_base);
    strcat(infile, TOKENF);
    if (!(result=fileBuffOpen(infile,"rb"))) {
      printf("Invalid Directory. Try again...");
      exit(1);
    }

    *framestart = atoi(argv[2]);
    *frameend   = atoi(argv[3]);

    play_states=malloc(sizeof(void*)*DARRAYSIZE);
    for (i=0; i<DARRAYSIZE; i++)
        play_states[i]=NULL;
    state_table_size=DARRAYSIZE;

    play_vlayouts=malloc(sizeof(void*)*DARRAYSIZE);
    for (i=0; i<DARRAYSIZE; i++)
        play_vlayouts[i]=NULL;
    vlayout_table_size=DARRAYSIZE;

    return result;
}

FILEBUFF * PlayInit()
{
    FILEBUFF *result=NULL;
    int i;
    char infile[MAX_PATH_LEN];

    Init();

    while (!result) {

        printf("Enter Trap/Playback Directory: ");
        gets(dir_base);

        /* append \ if necessary */
        if (*dir_base &&
            (dir_base[strlen(dir_base)-1] != '\\') &&
            (dir_base[strlen(dir_base)-1] != '/'))
          strcat(dir_base, "/");

        strcpy(infile,dir_base);
        strcat(infile,TOKENF);
        if (!(result=fileBuffOpen(infile,"rb")))
          printf("Invalid Directory. Try again...");
    }
    printf("Reading Tokens from: %s\n",infile);

    play_states=malloc(sizeof(void*)*DARRAYSIZE);
    for (i=0; i<DARRAYSIZE; i++)
        play_states[i]=NULL;
    state_table_size=DARRAYSIZE;

    play_vlayouts=malloc(sizeof(void*)*DARRAYSIZE);
    for (i=0; i<DARRAYSIZE; i++)
        play_vlayouts[i]=NULL;
    vlayout_table_size=DARRAYSIZE;

    return result;
}

FILEBUFF * FilterInit(int *startframe, int *endframe)
{
    FILEBUFF *result;
    int i;
    char infile[MAX_PATH_LEN], tmp[128];

    Init();

    // -----------------------------------------------------------
    // get the existing directory

    result = NULL;
    while (!result) {

        printf("Enter directory to GlideTrap recording: ");
        gets(dir_base);

        /* append \ if necessary */
        if (*dir_base &&
            (dir_base[strlen(dir_base)-1] != '\\') &&
            (dir_base[strlen(dir_base)-1] != '/'))
          strcat(dir_base, "/");

        strcpy(infile,dir_base);
        strcat(infile,TOKENF);
        if (!(result=fileBuffOpen(infile,"rb")))
          printf("Invalid Directory. Try again...\n");
    }

    // -----------------------------------------------------------
    // get the start frame

    *startframe = -1;
    while (*startframe < 0) {

      printf("Enter starting frame: ");
      gets(tmp);
      *startframe = atoi(tmp);

      if (*startframe < 0) {
        printf("Invalid start frame. Try again...\n");
      }
    }

    // -----------------------------------------------------------
    // get the end frame

    *endframe = -1;
    while (*endframe < *startframe) {

      printf("Enter ending frame: ");
      gets(tmp);
      *endframe = atoi(tmp);

      if (*endframe < *startframe) {
        printf("Invalid end frame. Try again...\n");
      }
    }

    printf("Reading Tokens from: %s\n",infile);

    play_states=malloc(sizeof(void*)*DARRAYSIZE);
    for (i=0; i<DARRAYSIZE; i++)
        play_states[i]=NULL;
    state_table_size=DARRAYSIZE;

    play_vlayouts=malloc(sizeof(void*)*DARRAYSIZE);
    for (i=0; i<DARRAYSIZE; i++)
        play_vlayouts[i]=NULL;
    vlayout_table_size=DARRAYSIZE;

    return result;
}

void Done()
{
    KillHashTable(states);
    KillHashTable(vlayouts);
    KillHashTable(textures);
    KillHashTable(lfbs);
    states=NULL;
    vlayouts=NULL;
    textures=NULL;
    lfbs=NULL;
}

void PlayDone(FILEBUFF *infile)
{
    FxU32 i;
    Done();

    for (i=0; i<numstates; i++)
        free(play_states[i]);
    free(play_states);
    
    for (i=0; i<numvlayouts; i++)
        free(play_vlayouts[i]);
    free(play_vlayouts);
    
    fileBuffClose(infile);
}

void CTrapDone()
{
    FxU32 i;    
    List *next,*now;
    Done();
    
    for (i=0; i<state_table_size; i++) {
        now=trap_states[i];
        while (now) {
            next=now->next;
            free(now->data);            
            free(now);            
            now=next;
        }
        trap_states[i]=NULL;
    }
    free(trap_states);

    for (i=0; i<vlayout_table_size; i++) {
        now=trap_vlayouts[i];
        while (now) {
            next=now->next;
            free(now->data);            
            free(now);            
            now=next;
        }
        trap_vlayouts[i]=NULL;
    }
    free(trap_vlayouts);

    if (outfile) fileBuffClose(outfile);
}

/* local functions */

char *strLodLog2(FxU32 lodLog2)
{
  switch (lodLog2) {
    case GR_LOD_LOG2_256 : return "256";
    case GR_LOD_LOG2_128 : return "128";
    case GR_LOD_LOG2_64  : return "64";
    case GR_LOD_LOG2_32  : return "32";
    case GR_LOD_LOG2_16  : return "16";
    case GR_LOD_LOG2_8   : return "8";
    case GR_LOD_LOG2_4   : return "4";
    case GR_LOD_LOG2_2   : return "2";
    case GR_LOD_LOG2_1   : return "1";
    default              : return "<unknown lodLog2>";
  }
}

char *strAspectRatioLog2(FxU32 aspectRatioLog2)
{
  switch (aspectRatioLog2) {
    case GR_ASPECT_LOG2_8x1 : return "8Wx1H";
    case GR_ASPECT_LOG2_4x1 : return "4Wx1H";
    case GR_ASPECT_LOG2_2x1 : return "2Wx1H";
    case GR_ASPECT_LOG2_1x1 : return "1Wx1H";
    case GR_ASPECT_LOG2_1x2 : return "1Wx2H";
    case GR_ASPECT_LOG2_1x4 : return "1Wx4H";
    case GR_ASPECT_LOG2_1x8 : return "1Wx8H";
    default                 : return "<unknown aspectRatioLog2>";
  }
}

char *strTextureFormat(FxU32 textureFormat)
{
  switch (textureFormat) {

    case GR_TEXFMT_8BIT               : return "8_bit";
    case GR_TEXFMT_YIQ_422            : return "YIQ_422";
    case GR_TEXFMT_ALPHA_8            : return "A_8";
    case GR_TEXFMT_INTENSITY_8        : return "I_8";
    case GR_TEXFMT_ALPHA_INTENSITY_44 : return "AI_44";
    case GR_TEXFMT_P_8                : return "P_8";
    case GR_TEXFMT_RSVD0              : return "Rsvd0";
    case GR_TEXFMT_RSVD1              : return "Rsvd1";
    case GR_TEXFMT_16BIT              : return "16_bit";
    case GR_TEXFMT_AYIQ_8422          : return "AYIQ_8422";
    case GR_TEXFMT_RGB_565            : return "RGB_565";
    case GR_TEXFMT_ARGB_1555          : return "ARGB_1555";
    case GR_TEXFMT_ARGB_4444          : return "ARGB_4444";
    case GR_TEXFMT_ALPHA_INTENSITY_88 : return "AI_88";
    case GR_TEXFMT_AP_88              : return "AP_88";
    case GR_TEXFMT_RSVD2              : return "Rsvd2";
    default                  : return "<unknown textureFormat>";
  }
}

static int new_files=0,
           filecomp_calls=0,
           attempted_fopens=0,
           full_freads=0;

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
attempted_fopens++;
    if (tmp==NULL)
    {
        tmp=fopen(filename,"wb");
        fwrite(&length,sizeof(FxU32),1,tmp);
        fwrite(data,length,1,tmp);
        fclose(tmp);
        return FXTRUE;
    } else {
        fread(&len,sizeof(FxU32),1,tmp);
        if (len!=length) {
            /* different size... must be different files! */
            fclose(tmp);
            return FXFALSE;
        }

        if (fastcompare) return FXTRUE;

        indat=malloc(length);
        fread(indat,length,1,tmp);
        fclose(tmp);        
full_freads++;
        
        if (strncmp((char *) indat,(char *) data,length)) {
            match=FXFALSE;
        } else {
            match=FXTRUE;
        }
        free(indat);
        return match;
    }
}

__inline void * load_f(char *filename, int bytes_per_element)
{
    char filen[MAX_PATH_LEN];    
    FxU32 size;     
    FILE *tmp;
    void *data;

    /* add directory base to filename */
    strcpy(filen,dir_base);
    strcat(filen,filename);
    tmp=fopen(filen,"rb");
    
    if (tmp!=NULL) {
        // read in data
#ifdef WIN32
        fread(&size,sizeof(FxU32),1,tmp);
        data=malloc(size);
        fread(data,size,1,tmp);
#else
        {
          // read in the length
          int i;
          char msg[128];
          char *p = (char*)&size;
          fread(p+3,sizeof(char),1,tmp);
          fread(p+2,sizeof(char),1,tmp);
          fread(p+1,sizeof(char),1,tmp);
          fread(p+0,sizeof(char),1,tmp);

          // read in the data
          data=malloc(size);
          switch (bytes_per_element) {

            case 1:
              fread(data,size,1,tmp);
              break;

            case 2:
              p = data;
              for ( i=size>>1 ; i>0 ; i-- ) {
                fread(p+1,sizeof(char),1,tmp);
                fread(p+0,sizeof(char),1,tmp);
                p += 2;
              }
              break;

            case 4:
              p = data;
              for ( i=size>>2 ; i>0 ; i-- ) {
                fread(p+3,sizeof(char),1,tmp);
                fread(p+2,sizeof(char),1,tmp);
                fread(p+1,sizeof(char),1,tmp);
                fread(p+0,sizeof(char),1,tmp);
                p += 4;
              }
              break;

            default:
              sprintf(msg, "Whoops.  Unsupported bytes_per_element\n");
#ifdef __WIN32__
              MessageBox(NULL,msg,"load_f",MB_ICONERROR | MB_OK);
#else
              fprintf(stderr, "%s", msg);
#endif
              exit(1);
          }
        }
#endif
        fclose(tmp);
        return data;
    } else {
        // die
        return NULL;
    }
}

int GetBytesPerTexel(GrTextureFormat_t format)
{
  char msg[128];
  switch (format) {

    case GR_TEXFMT_8BIT: // same as GR_TEXFMT_RGB_332
    case GR_TEXFMT_YIQ_422:
    case GR_TEXFMT_ALPHA_8:
    case GR_TEXFMT_INTENSITY_8:
    case GR_TEXFMT_ALPHA_INTENSITY_44:
    case GR_TEXFMT_P_8:
      return 1;

    case GR_TEXFMT_16BIT: // same as GR_TEXFMT_ARGB_8332
    case GR_TEXFMT_AYIQ_8422:
    case GR_TEXFMT_RGB_565:
    case GR_TEXFMT_ARGB_1555:
    case GR_TEXFMT_ARGB_4444:
    case GR_TEXFMT_ALPHA_INTENSITY_88:
    case GR_TEXFMT_AP_88:
      return 2;

    default:
      sprintf(msg, "Whoops.  Unknown texture format\n");
#ifdef __WIN32__
      MessageBox(NULL,msg,"GetBytesPerTexel",MB_ICONERROR | MB_OK);        
#else
      fprintf(stderr, "%s", msg);
#endif
      exit(1);
  }
}

void load_texture(char *filename, void** data, GrTextureFormat_t format)
{
    char msg[128];

    *data=load_f(filename, GetBytesPerTexel(format));

    if (data==NULL) {
        sprintf(msg,"Texture %s not found!\n",filename);
#ifdef __WIN32__
        MessageBox(NULL,msg,"Texture Load",MB_ICONERROR | MB_OK);
#else
        fprintf(stderr, "%s", msg);
#endif
        exit(1);
    }
}

int GetBytesPerPixel(GrLfbSrcFmt_t src_format)
{
  char msg[128];
  switch(src_format) {

    case GR_LFB_SRC_FMT_565:
    case GR_LFB_SRC_FMT_555:
    case GR_LFB_SRC_FMT_1555:
      return 2;

    case GR_LFB_SRC_FMT_888:
    case GR_LFB_SRC_FMT_8888:
    case GR_LFB_SRC_FMT_565_DEPTH:
    case GR_LFB_SRC_FMT_555_DEPTH:
    case GR_LFB_SRC_FMT_1555_DEPTH:
      return 4;

    case GR_LFB_SRC_FMT_ZA16:
    case GR_LFB_SRC_FMT_RLE16:
    default:
        sprintf(MESSAGE,"Unimplemented GR_LFB_SRC_FMT\n");
#ifdef __WIN32__
        MessageBox(NULL,msg,"GetBytesPerPixel", MB_ICONERROR | MB_OK);
#else
        fprintf(stderr, "%s", msg);
#endif
        exit(1);        
  }
}

void *load_lfb(char *filename, GrLfbSrcFmt_t src_format)
{
    void *data;
    char msg[128];

    data=load_f(filename, GetBytesPerPixel(src_format));
    if (data==NULL) {
        sprintf(msg,"LFB %s not found!\n",filename);
#ifdef __WIN32__
        MessageBox(NULL,msg,"LFB Load",MB_ICONERROR | MB_OK);
#else
        fprintf(stderr, "%s", msg);
#endif
        exit(1);
    }
    return data;
}

__inline char * save_f(unsigned long token,
                       void *data,
                       FxU32 length,
                       FxBool isnew,
                       const char* prefix,
                       int bytes_per_element)
/* save works by creating a hash value for the name of the file*/
/* if there is a collision, a compare is made. if matched, filename is returned*/
{
    char base[MAX_PATH_LEN];
    char filename[MAX_PATH_LEN];
    char *ext;
    char ts[MAX_PATH_LEN];
    int i;

    FxBool found;
    FILE *tmp;

    /* make template for filename */

    strcpy(base,dir_base);
    ext=(char *) malloc(TEXNAMELEN);
    strcpy(ext,prefix);
    sprintf(ts,"%07d",(int)token);
    strcat(ts,".");
    strcat(ext,ts);

    if (isnew) {
        /* first file... just overwrite */
        strcat(ext,"000");        
        /* ext is now t000token.000 - this is saved in command token stream */
        strcpy(filename,base);
        strcat(filename,ext);
        tmp=fopen(filename,"wb");
#ifdef WIN32
        fwrite(&length,sizeof(FxU32),1,tmp);
        fwrite(data,length,1,tmp);
#else
        {
          // write out the length
          int i;
          char msg[128];
          char *p = (char*)&length;
          fwrite(p+3,sizeof(char),1,tmp);
          fwrite(p+2,sizeof(char),1,tmp);
          fwrite(p+1,sizeof(char),1,tmp);
          fwrite(p+0,sizeof(char),1,tmp);

          // write out the data
          switch (bytes_per_element) {

            case 1:
              fwrite(data,length,1,tmp);
              break;

            case 2:
              p = data;
              for ( i=length>>1 ; i>0 ; i-- ) {
                fwrite(p+1,sizeof(char),1,tmp);
                fwrite(p+0,sizeof(char),1,tmp);
                p += 2;
              }
              break;

            case 4:
              p = data;
              for ( i=length>>2 ; i>0 ; i-- ) {
                fwrite(p+3,sizeof(char),1,tmp);
                fwrite(p+2,sizeof(char),1,tmp);
                fwrite(p+1,sizeof(char),1,tmp);
                fwrite(p+0,sizeof(char),1,tmp);
                p += 4;
              }
              break;

            default:
              sprintf(msg, "Whoops.  Unsupported bytes_per_element\n");
#ifdef __WIN32__
              MessageBox(NULL,msg,"save_f", MB_ICONERROR | MB_OK);        
#else
              fprintf(stderr, "%s", msg);
#endif
              exit(1);
          }
        }
#endif
        fclose(tmp);        

new_files++;

    } else {
        /* collided tokens... now check if there is a file that suits our purpose */
        /* if not, create one */
        i=0;
        found=FXFALSE;
        strcat(base,ext);
        /* base now contains <trapdir>\t(token). */
        /* ext contains t<token>. */
        while (!found)
        {
            /* iterate over token.000 until a match is found */
            sprintf(ts,"%03u",i);
            strcpy(filename,base);
            strcat(filename,ts);
            found=filecomp(filename,data,length);
filecomp_calls++;
            i++;
            if (i==999) {
#ifdef __WIN32__
                MessageBox(NULL,"Fatal error! 999 Collisions in files!\n\n",
                    "Save Files",MB_ICONERROR | MB_OK);
#else
                fprintf(stderr, "Fatal error! 999 Collisions in files!\n\n");
#endif
                exit(1);
            }
        }
        strcat(ext,ts);        
    }
    return ext;
}

char * save_texture(unsigned long token,
                    void *data,
                    FxU32 length,
                    FxBool isnew,
                    GrTextureFormat_t format)
{
    char msg[128];
    int bytes_per_texel;

    switch (format) {

      case GR_TEXFMT_8BIT: // same as GR_TEXFMT_RGB_332
      case GR_TEXFMT_YIQ_422:
      case GR_TEXFMT_ALPHA_8:
      case GR_TEXFMT_INTENSITY_8:
      case GR_TEXFMT_ALPHA_INTENSITY_44:
      case GR_TEXFMT_P_8:
        bytes_per_texel = 1;
        break;

      case GR_TEXFMT_16BIT: // same as GR_TEXFMT_ARGB_8332
      case GR_TEXFMT_AYIQ_8422:
      case GR_TEXFMT_RGB_565:
      case GR_TEXFMT_ARGB_1555:
      case GR_TEXFMT_ARGB_4444:
      case GR_TEXFMT_ALPHA_INTENSITY_88:
      case GR_TEXFMT_AP_88:
        bytes_per_texel = 2;
        break;

      default:
        sprintf(msg, "Whoops.  Unknown texture format in load_texture()\n");
#ifdef __WIN32__
        MessageBox(NULL,msg,"Texture Save", MB_ICONERROR | MB_OK);        
#else
        fprintf(stderr, "%s", msg);
#endif
        exit(1);
    }

    return save_f(token,data,length,isnew,TEXF, bytes_per_texel);
}

char * save_lfb(unsigned long token,
                void *data,
                FxU32 length,
                FxBool isnew,
                GrLfbSrcFmt_t src_format)
{
    return save_f(token,data,length,isnew,LFBF, GetBytesPerPixel(src_format));
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

FxU32 GetMipMapSize(GrAspectRatio_t aspectRatio, GrLOD_t thisLod,GrTextureFormat_t format)
{
    FxU32 width;

    width = _grMipMapHostWH[0x3-aspectRatio][0x8-thisLod][0];

    if (format>=GR_TEXFMT_16BIT) width*=2;
    
    return width;
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



// this function calculates the vertex size
/*
#define GETSIZE(PART) \
    if (gc.PART.mode == GR_PARAM_ENABLE) size=MAX(size,gc.PART.offset+4*SIZE_##PART);
*/
#define GETSIZE(PART) \
    if (gc.PART.mode == GR_PARAM_ENABLE) size=MAX(size,gc.PART.offset+4*SIZE_##PART);
int getvertexsize()
{
    int size=0;

    GETSIZE(vertexInfo); // GR_PARAM_XY:
    GETSIZE(zInfo);      // GR_PARAM_Z:
    GETSIZE(wInfo);      // GR_PARAM_W:
    GETSIZE(fogInfo);    // GR_PARAM_FOG_EXT:
    GETSIZE(aInfo);      // GR_PARAM_A:
    GETSIZE(rgbInfo);    // GR_PARAM_RGB:
    GETSIZE(pargbInfo);  // GR_PARAM_PARGB:
    GETSIZE(st0Info);    // GR_PARAM_ST0:
    GETSIZE(st1Info);    // GR_PARAM_ST1:
    GETSIZE(qInfo);      // GR_PARAM_Q:
    GETSIZE(q0Info);     // GR_PARAM_Q0:
    GETSIZE(q1Info);     // GR_PARAM_Q1:

    return size;
}

// Macro to output structure
#define OUTPUT(PART)                                      \
    if (gc.PART.mode == GR_PARAM_ENABLE) {                \
      float *data = (float*)((char*)p + gc.PART.offset);  \
      switch(SIZE_##PART) {                               \
                                                          \
        case 1: fileBuffWriteFloat(outfile, data[0]);     \
                break;                                    \
                                                          \
        case 2: fileBuffWriteFloat(outfile, data[0]);     \
                fileBuffWriteFloat(outfile, data[1]);     \
                break;                                    \
                                                          \
        case 3: fileBuffWriteFloat(outfile, data[0]);     \
                fileBuffWriteFloat(outfile, data[1]);     \
                fileBuffWriteFloat(outfile, data[2]);     \
                break;                                    \
                                                          \
        default: printf("Whoops, fell out of switch in OUTPUT macro\n");\
                 break;                                   \
      }                                                   \
    }

// this function outputs the vertex according
// to what stuff has been enabled/disabled
__inline void my_outputvertex(void *p)
{
    OUTPUT(vertexInfo); // GR_PARAM_XY:
    OUTPUT(zInfo);      // GR_PARAM_Z:
    OUTPUT(wInfo);      // GR_PARAM_W:
    OUTPUT(fogInfo);    // GR_PARAM_FOG_EXT:
    OUTPUT(aInfo);      // GR_PARAM_A:
    OUTPUT(rgbInfo);    // GR_PARAM_RGB:
    OUTPUT(pargbInfo);  // GR_PARAM_PARGB:
    OUTPUT(st0Info);    // GR_PARAM_ST0:
    OUTPUT(st1Info);    // GR_PARAM_ST1:
    OUTPUT(qInfo);      // GR_PARAM_Q:
    OUTPUT(q0Info);     // GR_PARAM_Q0:
    OUTPUT(q1Info);     // GR_PARAM_Q1:
    
} /* my_outputvertex */

// Macro to output structure
#define INPUT(PART)                                         \
    if (gc.PART.mode == GR_PARAM_ENABLE) {                  \
      float *data = (float*)((char*)vtx + gc.PART.offset);  \
      switch(SIZE_##PART) {                                 \
                                                            \
        case 1: data[0] = fileBuffReadFloat(infile);        \
                break;                                      \
                                                            \
        case 2: data[0] = fileBuffReadFloat(infile);        \
                data[1] = fileBuffReadFloat(infile);        \
                break;                                      \
                                                            \
        case 3: data[0] = fileBuffReadFloat(infile);        \
                data[1] = fileBuffReadFloat(infile);        \
                data[2] = fileBuffReadFloat(infile);        \
                break;                                      \
                                                            \
        default: printf("Whoops, fell out of switch in INPUT macro\n");\
                 break;                                     \
      }                                                     \
    }

__inline void * my_inputvertex(FILEBUFF *infile)
{
    void * p;
    p=malloc(sizeof(int)*15);
    my_inputvnalloc(p,infile);
    return p;
}

/* faster, non allocating version */
__inline void my_inputvnalloc(void *vtx, FILEBUFF *infile)
{
    INPUT(vertexInfo); // GR_PARAM_XY:
    INPUT(zInfo);      // GR_PARAM_Z:
    INPUT(wInfo);      // GR_PARAM_W:
    INPUT(fogInfo);    // GR_PARAM_FOG_EXT:
    INPUT(aInfo);      // GR_PARAM_A:
    INPUT(rgbInfo);    // GR_PARAM_RGB:
    INPUT(pargbInfo);  // GR_PARAM_PARGB:
    INPUT(st0Info);    // GR_PARAM_ST0:
    INPUT(st1Info);    // GR_PARAM_ST1:
    INPUT(qInfo);      // GR_PARAM_Q:
    INPUT(q0Info);     // GR_PARAM_Q0:
    INPUT(q1Info);     // GR_PARAM_Q1:
}


/* extensions */
void FX_CALL trap_grChromaRangeModeExt(GrChromakeyMode_t mode)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grChromaRangeModeExt);
    fileBuffWriteFxU32(outfile, mode);
    if (ScreenEcho)
        sprintf(MESSAGE,"grChromaRangeModeExt()\n");
  }

  okToRecord = 0;

  /* CALL glidefunction */
  MYEXT_grChromaRangeModeExt (mode);

  okToRecord = 1;
}

void FX_CALL trap_grChromaRangeExt(GrColor_t color, GrColor_t range, GrChromaRangeMode_t match_mode)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grChromaRangeExt);
    fileBuffWriteFxU32(outfile, color);
    fileBuffWriteFxU32(outfile, range);
    fileBuffWriteFxU32(outfile, match_mode);
    if (ScreenEcho)
        sprintf(MESSAGE,"grChromaRangeExt()\n");
  }
  okToRecord = 0;
  MYEXT_grChromaRangeExt (color,range,match_mode);
  okToRecord = 1;
}

void FX_CALL trap_grTexChromaModeExt(GrChipID_t tmu, GrTexChromakeyMode_t mode)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexChromaModeExt);
    fileBuffWriteFxU32(outfile, tmu);
    fileBuffWriteFxU32(outfile, mode);
    if (ScreenEcho)
        sprintf(MESSAGE,"grTexChromaModeExt()\n");
  }
  okToRecord = 0;
  MYEXT_grTexChromaModeExt (tmu,mode);
  okToRecord = 1;
}

void FX_CALL  trap_grTexChromaRangeExt(GrChipID_t tmu, GrColor_t min, GrColor_t max, GrTexChromakeyMode_t mode)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexChromaRangeExt);
    fileBuffWriteFxU32(outfile, tmu);
    fileBuffWriteFxU32(outfile, min);
    fileBuffWriteFxU32(outfile, max);
    fileBuffWriteFxU32(outfile, mode);
    if (ScreenEcho)
        sprintf(MESSAGE,"grTexChromaRangeExt()\n");
  }
  okToRecord = 0;
  MYEXT_grTexChromaRangeExt(tmu,min,max,mode);
  okToRecord = 1;
}

void FX_CALL trap_grDrawTextureLineExt(const void *a, const void *b)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grDrawTextureLineExt);
    my_outputvertex((void *)a);
    my_outputvertex((void *)b);
    if (ScreenEcho)
        sprintf(MESSAGE,"grTexChromaRangeExt()\n");
  }
  okToRecord = 0;
  MYEXT_grDrawTextureLineExt(a,b);
  okToRecord = 1;
}

void play_grChromaRangeModeExt(FILEBUFF *infile)
{
    GrChromakeyMode_t mode;

    mode = fileBuffReadFxU32(infile);
    if (ScreenEcho)
        printf("grChromaRangeModeExt(%d)\n", (int)mode);
    /* CALL glidefunction */
    if (MYEXT_grChromaRangeModeExt) MYEXT_grChromaRangeModeExt (mode);
}

void play_grChromaRangeExt(FILEBUFF *infile)
{
    GrColor_t color;
    GrColor_t range;
    GrChromaRangeMode_t match_mode;

    color      = fileBuffReadFxU32(infile);
    range      = fileBuffReadFxU32(infile);
    match_mode = fileBuffReadFxU32(infile);
    if (ScreenEcho)
        printf("grChromaRangeExt(%x,%x,%d)\n", (int)color, (int)range, (int)match_mode);
    if (MYEXT_grChromaRangeExt) MYEXT_grChromaRangeExt (color,range,match_mode);
}

void play_grTexChromaModeExt(FILEBUFF *infile)
{
    GrChipID_t tmu;
    GrTexChromakeyMode_t mode;

    tmu  = fileBuffReadFxU32(infile);
    mode = fileBuffReadFxU32(infile);
    if (ScreenEcho)
        printf("grTexChromaModeExt(%d,%d)\n", (int)tmu, (int)mode);
    if (MYEXT_grTexChromaModeExt) MYEXT_grTexChromaModeExt (tmu,mode);
}

void play_grTexChromaRangeExt(FILEBUFF *infile)
{
    GrChipID_t tmu;
    GrColor_t min;
    GrColor_t max;
    GrTexChromakeyMode_t mode;

    tmu  = fileBuffReadFxU32(infile);
    min  = fileBuffReadFxU32(infile);
    max  = fileBuffReadFxU32(infile);
    mode = fileBuffReadFxU32(infile);
    if (ScreenEcho)
        printf("grTexChromaRangeExt(%d,%x,%x,%d)\n", (int)tmu, (int)min, (int)max, (int)mode);
    if (MYEXT_grTexChromaRangeExt) MYEXT_grTexChromaRangeExt (tmu,min,max,mode);
}

void play_grDrawTextureLineExt(FILEBUFF *infile)
{
    my_inputvnalloc(&vtxa,infile);
    my_inputvnalloc(&vtxb,infile);
    if (ScreenEcho)
        printf("grTexChromaRangeExt()\n");
    if (!nodraw && MYEXT_grDrawTextureLineExt) MYEXT_grDrawTextureLineExt(&vtxa,&vtxb);
}

/* TRAP CALLS */

void trap_grAADrawTriangle(const void *a,
                           const void *b,
                           const void *c,
                           FxBool ab_antialias,
                           FxBool bc_antialias,
                           FxBool ca_antialias)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grAADrawTriangle);
    my_outputvertex((void*) a);
    my_outputvertex((void*) b);
    my_outputvertex((void*) c);
    fileBuffWriteFxU32(outfile, ab_antialias);
    fileBuffWriteFxU32(outfile, bc_antialias);
    fileBuffWriteFxU32(outfile, ca_antialias);
    if (ScreenEcho)
        sprintf(MESSAGE,"grAADrawTriangle()\n");
  }
}

void trap_grAlphaBlendFunction(GrAlphaBlendFnc_t rgb_sf,
                               GrAlphaBlendFnc_t rgb_df,
                               GrAlphaBlendFnc_t alpha_sf,
                               GrAlphaBlendFnc_t alpha_df)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grAlphaBlendFunction);
    fileBuffWriteFxU32(outfile, rgb_sf);
    fileBuffWriteFxU32(outfile, rgb_df);
    fileBuffWriteFxU32(outfile, alpha_sf);
    fileBuffWriteFxU32(outfile, alpha_df);
    if (ScreenEcho)
        sprintf(MESSAGE,"grAlphaBlendFunction()\n");
  }
}

void trap_grAlphaCombine(GrCombineFunction_t function,
                         GrCombineFactor_t factor,
                         GrCombineLocal_t local,
                         GrCombineOther_t other,
                         FxBool invert)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grAlphaCombine);
    fileBuffWriteFxU32(outfile, function);
    fileBuffWriteFxU32(outfile, factor);
    fileBuffWriteFxU32(outfile, local);
    fileBuffWriteFxU32(outfile, other);
    fileBuffWriteFxU32(outfile, invert);
    if (ScreenEcho)
        sprintf(MESSAGE,"grAlphaCombine()\n");
  }
}

void trap_grAlphaControlsITRGBLighting(FxBool enable)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grAlphaControlsITRGBLighting);
    fileBuffWriteFxU32(outfile, enable);
    if (ScreenEcho)
        sprintf(MESSAGE,"grAlphaControlsITRGBLighting()\n");
  }
}

void trap_grAlphaTestFunction(GrCmpFnc_t function)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grAlphaTestFunction);
    fileBuffWriteFxU32(outfile, function);
    if (ScreenEcho)
        sprintf(MESSAGE,"grAlphaTestFunction()\n");
  }
}

void trap_grAlphaTestReferenceValue(GrAlpha_t value)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grAlphaTestReferenceValue);
    fileBuffWriteFxU8(outfile, value);
    if (ScreenEcho)
        sprintf(MESSAGE,"grAlphaTestReferenceValue()\n");
  }
}

void trap_grBufferClear(GrColor_t color,
                        GrAlpha_t alpha,
                        FxU32 depth)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grBufferClear);
    fileBuffWriteFxU32(outfile, color);
    fileBuffWriteFxU8(outfile, alpha);
    fileBuffWriteFxU32(outfile, depth);
    if (ScreenEcho)
        sprintf(MESSAGE,"grBufferClear()\n");
  }
}

void trap_grBufferSwap(int swap_interval)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grBufferSwap);
    fileBuffWriteFxU32(outfile, swap_interval);
    fileBuffFlush(outfile);
    if (ScreenEcho)
        sprintf(MESSAGE,"grBufferSwap()\n");
  }
}

void trap_grChromakeyMode(GrChromakeyMode_t mode)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grChromakeyMode);
    fileBuffWriteFxU32(outfile, mode);
    if (ScreenEcho)
        sprintf(MESSAGE,"grChromakeyMode()\n");
  }
}

void trap_grChromakeyValue(GrColor_t value)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grChromakeyValue);
    fileBuffWriteFxU32(outfile, value);
    if (ScreenEcho)
        sprintf(MESSAGE,"grChromakeyValue()\n");
  }
}

void trap_grClipWindow(FxU32 minx,
                       FxU32 miny,
                       FxU32 maxx,
                       FxU32 maxy)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grClipWindow);
    fileBuffWriteFxU32(outfile, minx);
    fileBuffWriteFxU32(outfile, miny);
    fileBuffWriteFxU32(outfile, maxx);
    fileBuffWriteFxU32(outfile, maxy);
    if (ScreenEcho)
        sprintf(MESSAGE,"grClipWindow()\n");
  }
}

void trap_grColorCombine(GrCombineFunction_t function,
                         GrCombineFactor_t factor,
                         GrCombineLocal_t local,
                         GrCombineOther_t other,
                         FxBool invert)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grColorCombine);
    fileBuffWriteFxU32(outfile, function);
    fileBuffWriteFxU32(outfile, factor);
    fileBuffWriteFxU32(outfile, local);
    fileBuffWriteFxU32(outfile, other);
    fileBuffWriteFxU32(outfile, invert);
    if (ScreenEcho)
        sprintf(MESSAGE,"grColorCombine()\n");
  }
}

void trap_grColorMask(FxBool rgb,
                      FxBool a)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grColorMask);
    fileBuffWriteFxU32(outfile, rgb);
    fileBuffWriteFxU32(outfile, a);
    if (ScreenEcho)
        sprintf(MESSAGE,"grColorMask()\n");
  }
}

void trap_grConstantColorValue(GrColor_t value)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grConstantColorValue);
    fileBuffWriteFxU32(outfile, value);
    if (ScreenEcho)
        sprintf(MESSAGE,"grConstantColorValue()\n");
  }
}

void trap_grCoordinateSpace(GrCoordinateSpaceMode_t mode)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grCoordinateSpace);
    fileBuffWriteFxU32(outfile, mode);
    if (ScreenEcho)
        sprintf(MESSAGE,"grCoordinateSpace()\n");
  }
}

void trap_grCullMode(GrCullMode_t mode)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grCullMode);
    fileBuffWriteFxU32(outfile, mode);
    if (ScreenEcho)
        sprintf(MESSAGE,"grCullMode()\n");
  }
}

void trap_grDepthBiasLevel(FxI32 level)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grDepthBiasLevel);
    fileBuffWriteFxU32(outfile, level);
    if (ScreenEcho)
        sprintf(MESSAGE,"grDepthBiasLevel()\n");
  }
}

void trap_grDepthBufferFunction(GrCmpFnc_t function)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grDepthBufferFunction);
    fileBuffWriteFxU32(outfile, function);
    if (ScreenEcho)
        sprintf(MESSAGE,"grDepthBufferFunction()\n");
  }
}

void trap_grDepthBufferMode(GrDepthBufferMode_t mode)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grDepthBufferMode);
    fileBuffWriteFxU32(outfile, mode);
    if (ScreenEcho)
        sprintf(MESSAGE,"grDepthBufferMode()\n");
  }
}

void trap_grDepthMask(FxBool mask)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grDepthMask);
    fileBuffWriteFxU32(outfile, mask);
    if (ScreenEcho)
        sprintf(MESSAGE,"grDepthMask()\n");
  }
}

void trap_grDepthRange(FxFloat n,
                       FxFloat f)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grDepthRange);
    fileBuffWriteFloat(outfile, n);
    fileBuffWriteFloat(outfile, f);
    if (ScreenEcho)
        sprintf(MESSAGE,"grDepthRange()\n");
  }
}

void trap_grDisable(GrEnableMode_t mode)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grDisable);
    fileBuffWriteFxU32(outfile, mode);
    if (ScreenEcho)
        sprintf(MESSAGE,"grDisable()\n");
  }
}

void trap_grDisableAllEffects( void)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grDisableAllEffects);
    if (ScreenEcho)
        sprintf(MESSAGE,"grDisableAllEffects()\n");
  }
}

void trap_grDitherMode(GrDitherMode_t mode)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grDitherMode);
    fileBuffWriteFxU32(outfile, mode);
    if (ScreenEcho)
        sprintf(MESSAGE,"grDitherMode()\n");
  }
}

void trap_grDrawLine(const void *v1,
                     const void *v2)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grDrawLine);
    my_outputvertex((void*) v1);
    my_outputvertex((void*) v2);
    if (ScreenEcho)
        sprintf(MESSAGE,"grDrawLine()\n");
  }
}

void trap_grDrawPoint(const void *pt)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grDrawPoint);
    my_outputvertex((void*) pt);
    if (ScreenEcho)
        sprintf(MESSAGE,"grDrawPoint()\n");
  }
}

void trap_grDrawTriangle(const void *a,
                         const void *b,
                         const void *c)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grDrawTriangle);
    my_outputvertex((void*) a);
    my_outputvertex((void*) b);
    my_outputvertex((void*) c);
    if (ScreenEcho)
        sprintf(MESSAGE,"grDrawTriangle()\n");
  }
}

void trap_grDrawVertexArray(FxU32 mode,
                            FxU32 Count,
                            void *pointers)
 { 
    FxU32 i;
    void **p;

  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grDrawVertexArray);
    fileBuffWriteFxU32(outfile, mode);
    fileBuffWriteFxU32(outfile, Count);

    for ( p=pointers, i=0; i<Count; i++)
        my_outputvertex(p[i]);

    if (ScreenEcho)
        sprintf(MESSAGE,"grDrawVertexArray()\n");
  }
}

void trap_grDrawVertexArrayContiguous(FxU32 mode,
                                      FxU32 Count,
                                      void *pointers,
                                      FxU32 stride)
{
  int i, size_in_words;

  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grDrawVertexArrayContiguous);
    fileBuffWriteFxU32(outfile, mode);
    fileBuffWriteFxU32(outfile, Count);
    // note! order swapped
    fileBuffWriteFxU32(outfile, stride);

    size_in_words = (stride * Count) >> 2;
    for ( i=0 ; i<size_in_words ; i++ )
      fileBuffWriteFxU32(outfile, ((FxU32*)pointers)[i]);    

    if (ScreenEcho)
        sprintf(MESSAGE,"grDrawVertexArrayContiguous()\n");
  }
}

void trap_grEnable(GrEnableMode_t mode)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grEnable);
    fileBuffWriteFxU32(outfile, mode);
    if (ScreenEcho)
        sprintf(MESSAGE,"grEnable()\n");
  }
}

void trap_grErrorSetCallback(GrErrorCallbackFnc_t fnc)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grErrorSetCallback);
    // irrelevant
    /*
    fileBuffWriteFxU32(outfile, fnc);
    */
    if (ScreenEcho)
        sprintf(MESSAGE,"grErrorSetCallback()\n");
  }
}

void trap_grFinish( void)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grFinish);
    if (ScreenEcho)
        sprintf(MESSAGE,"grFinish()\n");
  }
}

void trap_grFlush( void)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grFlush);
    fileBuffFlush(outfile);
    if (ScreenEcho)
        sprintf(MESSAGE,"grFlush()\n");
  }
}

void trap_grFogColorValue(GrColor_t fogcolor)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grFogColorValue);
    fileBuffWriteFxU32(outfile, fogcolor);
    if (ScreenEcho)
        sprintf(MESSAGE,"grFogColorValue()\n");
  }
}

void trap_grFogMode(GrFogMode_t mode)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grFogMode);
    fileBuffWriteFxU32(outfile, mode);
    if (ScreenEcho)
        sprintf(MESSAGE,"grFogMode()\n");
  }
}

void trap_grFogTable(const GrFog_t ft[],FxU32 tsize)
{
    
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grFogTable);
    
    fileBuffWriteFxU32(outfile, tsize);
    fileBuffWriteFxU8Array(outfile, (char*)ft, tsize);
    if (ScreenEcho)
        sprintf(MESSAGE,"grFogTable()\n");
  }
}

void trap_grGet(FxU32 pname,
                FxU32 plength,
                FxI32 *params)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grGet);
    
    fileBuffWriteFxU32(outfile, pname);
    fileBuffWriteFxU32(outfile, plength);

    if (ScreenEcho)
        sprintf(MESSAGE,"grGet(%d,%d)\n", (int)pname, (int)plength);
  }
}


GrProc trap_grGetProcAddress(char *procName, GrProc ext)
{
    int size;    

  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grGetProcAddress);
    size=strlen(procName);
    fileBuffWriteFxU32(outfile, size);
    fileBuffWriteFxU8Array(outfile, procName, size);

    if (ScreenEcho)
        sprintf(MESSAGE,"grGetProcAddress()\n");

    if (!strcmp(procName, "grChromaRangeModeExt")) {
        MYEXT_grChromaRangeModeExt=(GrProc) ext;        
        return (GrProc) trap_grChromaRangeModeExt;
    }    
    
    if (!strcmp(procName, "grChromaRangeExt")) {
        MYEXT_grChromaRangeExt=ext;
        return (GrProc) trap_grChromaRangeExt;
    }

    if (!strcmp(procName, "grTexChromaModeExt")) {
        MYEXT_grTexChromaModeExt=ext;
        return (GrProc) trap_grTexChromaModeExt;
    }

    if (!strcmp(procName, "grTexChromaRangeExt")) {
        MYEXT_grTexChromaRangeExt=ext;
        return (GrProc) trap_grTexChromaRangeExt;
    }

    if (!strcmp(procName, "grDrawTextureLineExt")) {
        MYEXT_grDrawTextureLineExt=ext;
        return (GrProc) trap_grDrawTextureLineExt;
    } 

    sprintf(MESSAGE,"ERROR! Unimplemented trap for extension %s\n",procName);
  }
  return NULL;
}

void trap_grGetString(FxU32 pname)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grGetString);
    fileBuffWriteFxU32(outfile, pname);
    if (ScreenEcho)
        sprintf(MESSAGE,"grGetString()\n");
  }
}

void trap_grGlideGetState(void *state,FxU32 length)
{
    unsigned long token,i;
    FxU32 statenum;
    List *now;
    FxBool found;

  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grGlideGetState);

    /* put this state into the states database */
    
    /* use token for fast access */
    token=find(states,state,length);
    /* oops, need to reallocate */
    if (token>=state_table_size) {
        realloc(trap_states,2*sizeof(List *) * state_table_size);
        for (i=state_table_size; i<2*state_table_size; i++)
            trap_states[i]=NULL;
        state_table_size*=2;
    }

    /* search through the list to see if we have an exact match */

    now=trap_states[token];

    found=FXFALSE;

    while (now && !found) {
        if (!strncmp((char *) now->data, (char *) state,length)) {
            found=FXTRUE;
            statenum=now->token;
        }
    }

    if (!found) {        
        /* this is a new one */
        now=(List *) malloc(sizeof(List));
        statenum=numstates;
        now->token=statenum;
        now->data=malloc(length);
        memcpy(now->data,state,length);
        now->next=trap_states[token];
        trap_states[token]=now;
        numstates++;
    }

    fileBuffWriteFxU32(outfile, statenum);

    if (ScreenEcho)
        sprintf(MESSAGE,"grGlideGetState()\tState: %d\n", (int)statenum);
  }
}

void trap_grGlideGetVertexLayout(void *vlayout, FxU32 length)
{
    unsigned long token,i;
    FxU32 vlayoutnum;
    List *now;
    FxBool found;

  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grGlideGetVertexLayout);

    /* put this vlayout into the vlayouts database */
    
    /* use token for fast access */
    token=find(vlayouts,vlayout,length);
    /* oops, need to reallocate */
    if (token>=vlayout_table_size) {
        realloc(trap_vlayouts,2*sizeof(List *) * vlayout_table_size);
        for (i=vlayout_table_size; i<2*vlayout_table_size; i++)
            trap_vlayouts[i]=NULL;
        vlayout_table_size*=2;
    }

    /* search through the list to see if we have an exact match */

    now=trap_vlayouts[token];

    found=FXFALSE;

    while (now && !found) {
        if (!strncmp((char *) now->data, (char *) vlayout,length)) {
            found=FXTRUE;
            vlayoutnum=now->token;
        }
    }

    if (!found) {        
        /* this is a new one */
        now=(List *) malloc(sizeof(List));
        vlayoutnum=numvlayouts;
        now->token=vlayoutnum;
        now->data=malloc(length);
        memcpy(now->data,vlayout,length);
        now->next=trap_vlayouts[token];
        trap_vlayouts[token]=now;
        numvlayouts++;
    }

    fileBuffWriteFxU32(outfile, vlayoutnum);

    if (ScreenEcho)
        sprintf(MESSAGE,"grGlideGetVertexLayout()\tVLayout: %d\n", (int)vlayoutnum);
  }
}

void trap_grGlideInit( void)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grGlideInit);
    if (ScreenEcho)
        sprintf(MESSAGE,"grGlideInit()\n");
  }
}

void trap_grGlideSetState(const void *state, FxU32 length)
{
    unsigned long token;
    FxU32 statenum;
    List *now;
    FxBool found;
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grGlideSetState);
    
    token=find(states,(void *) state,length);
    
    if (token>state_table_size) {
        sprintf(MESSAGE,"Impossible: program is grGlideSetState() unknown state!\n");
        exit(1);
    }
    found=FXFALSE;
    now=trap_states[token];

    while (now && !found) {
        if (!strncmp((char *) now->data,(char *) state, length)) {
            found=FXTRUE;
            statenum=now->token;
        }
        now=now->next;
    }
    if (!found) {
        sprintf(MESSAGE,"Impossible: program is grGlideSetState() unknown state!\n");
        exit(1);
    }

    fileBuffWriteFxU32(outfile, statenum);    

    if (ScreenEcho)
        sprintf(MESSAGE,"grGlideSetState()\tState: %d\n", (int)statenum);
  }
}

void trap_grGlideSetVertexLayout(const void *vlayout, FxU32 length)
{
    unsigned long token;
    FxU32 vlayoutnum;
    List *now;
    FxBool found;
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grGlideSetVertexLayout);
    
    token=find(vlayouts,(void *) vlayout,length);
    
    if (token>vlayout_table_size) {
        sprintf(MESSAGE,"Impossible: program is grGlideSetVertexLayout() unknown vlayout!\n");
        exit(1);
    }
    found=FXFALSE;
    now=trap_vlayouts[token];

    while (now && !found) {
        if (!strncmp((char *) now->data,(char *) vlayout, length)) {
            found=FXTRUE;
            vlayoutnum=now->token;
        }
        now=now->next;
    }
    if (!found) {
        sprintf(MESSAGE,"Impossible: program is grGlideSetVertexLayout() unknown vlayout!\n");
        exit(1);
    }

    fileBuffWriteFxU32(outfile, vlayoutnum);    

    if (ScreenEcho)
        sprintf(MESSAGE,"grGlideSetVertexLayout()\tVLayout: %d\n",(int)vlayoutnum);
  }
}

void trap_grGlideShutdown( void)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grGlideShutdown);
    fileBuffFlush(outfile);
    if (ScreenEcho)
        sprintf(MESSAGE,"grGlideShutdown()\n");
  }
}

void trap_grLfbConstantAlpha(GrAlpha_t alpha)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grLfbConstantAlpha);
    fileBuffWriteFxU8(outfile, alpha);
    if (ScreenEcho)
        sprintf(MESSAGE,"grLfbConstantAlpha()\n");
  }
}

void trap_grLfbConstantDepth(FxU32 depth)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grLfbConstantDepth);
    fileBuffWriteFxU32(outfile, depth);
    if (ScreenEcho)
        sprintf(MESSAGE,"grLfbConstantDepth()\n");
  }
}

void trap_grLfbLock(GrLock_t type,
                    GrBuffer_t buffer,
                    GrLfbWriteMode_t writeMode,
                    GrOriginLocation_t origin,
                    FxBool pixelPipeline,
                    FxBool result)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grLfbLock);
    
    fileBuffWriteFxU32(outfile, type);
    fileBuffWriteFxU32(outfile, buffer);
    fileBuffWriteFxU32(outfile, writeMode);
    fileBuffWriteFxU32(outfile, origin);
    fileBuffWriteFxU32(outfile, pixelPipeline);
    fileBuffWriteFxU32(outfile, result);    
    
    if (ScreenEcho)
        sprintf(MESSAGE,"grLfbLock()\n");
  }
}

void trap_grLfbReadRegion(GrBuffer_t src_buffer,
                          FxU32 src_x,
                          FxU32 src_y,
                          FxU32 src_width,
                          FxU32 src_height,
                          FxU32 dst_stride,
                          void *dst_data)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grLfbReadRegion);
    
    fileBuffWriteFxU32(outfile, src_buffer);
    fileBuffWriteFxU32(outfile, src_x);
    fileBuffWriteFxU32(outfile, src_y);
    fileBuffWriteFxU32(outfile, src_width);
    fileBuffWriteFxU32(outfile, src_height);
    fileBuffWriteFxU32(outfile, dst_stride);
    
    if (ScreenEcho)
        sprintf(MESSAGE,"grLfbReadRegion()\n");
  }
}

void trap_grLfbUnlock(GrLock_t type,
                      GrBuffer_t buffer)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grLfbUnlock);
    
    fileBuffWriteFxU32(outfile, type);
    fileBuffWriteFxU32(outfile, buffer);
    
    if (ScreenEcho)
        sprintf(MESSAGE,"grLfbUnlock()\n");
  }
}

void trap_grLfbWriteColorFormat(GrColorFormat_t colorFormat)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grLfbWriteColorFormat);
    
    fileBuffWriteFxU32(outfile, colorFormat);
    
    if (ScreenEcho)
        sprintf(MESSAGE,"grLfbWriteColorFormat()\n");
  }
}

void trap_grLfbWriteColorSwizzle(FxBool swizzleBytes,
                                 FxBool swapWords)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grLfbWriteColorSwizzle);
    
    fileBuffWriteFxU32(outfile, swizzleBytes);
    fileBuffWriteFxU32(outfile, swapWords);
    
    if (ScreenEcho)
        sprintf(MESSAGE,"grLfbWriteColorSwizzle()\n");
  }
}

void trap_grLfbWriteRegion(GrBuffer_t dst_buffer,
                           FxU32 dst_x,
                           FxU32 dst_y,
                           GrLfbSrcFmt_t src_format,
                           FxU32 src_width,
                           FxU32 src_height,
                           FxBool pixelPipeline,
                           FxI32 src_stride,
                           void *src_data)
{
    char msg[128];
    FxU32 length,token,old;
    char * filename;
    FxBool invalid;

  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grLfbWriteRegion);
    
    fileBuffWriteFxU32(outfile, dst_buffer);
    fileBuffWriteFxU32(outfile, dst_x);
    fileBuffWriteFxU32(outfile, dst_y);
    fileBuffWriteFxU32(outfile, src_format);
    fileBuffWriteFxU32(outfile, src_width);
    fileBuffWriteFxU32(outfile, src_height);
    fileBuffWriteFxU32(outfile, pixelPipeline);
    fileBuffWriteFxU32(outfile, src_stride);    

    invalid=FXFALSE;

    length = (src_stride < 0) ?
             (-src_stride)*src_height :
             src_stride*src_height;

    switch(src_format) {

      case GR_LFB_SRC_FMT_565:
      case GR_LFB_SRC_FMT_555:
      case GR_LFB_SRC_FMT_1555:
      case GR_LFB_SRC_FMT_888:
      case GR_LFB_SRC_FMT_8888:
      case GR_LFB_SRC_FMT_565_DEPTH:
      case GR_LFB_SRC_FMT_555_DEPTH:
      case GR_LFB_SRC_FMT_1555_DEPTH:
        // everything's ok
        break;

      case GR_LFB_SRC_FMT_ZA16:
      case GR_LFB_SRC_FMT_RLE16:
      default:
          sprintf(MESSAGE,"Unimplemented GR_LFB_SRC_FMT in trap_grLfbWriteRegion\n");
#ifdef __WIN32__
          MessageBox(NULL,msg,"Texture Save", MB_ICONERROR | MB_OK);        
#else
          fprintf(stderr, "%s", msg);
#endif
          exit(1);        
    }

    if (invalid) {        
        fileBuffWriteFxU8Array(outfile, INVALID, TEXNAMELEN);
    } else {
        old=lfbs->numitems;
        token=find(lfbs,src_data,length);
    
        filename=save_lfb(token,src_data,length,old!=token, src_format);
        fileBuffWriteFxU8Array(outfile, filename, TEXNAMELEN);
    
        if (ScreenEcho)
            sprintf(MESSAGE,"grLfbWriteRegion()\tSaved as:%s\n",filename);
        free(filename);
    }
  }
}

void trap_grLoadGammaTable(FxU32 nentries,
                           FxU32 *red,
                           FxU32 *green,
                           FxU32 *blue)
{
  FxU32 i;
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grLoadGammaTable);
    fileBuffWriteFxU32(outfile, nentries);
    for ( i=0 ; i<nentries ; i++ ) {
      fileBuffWriteFxU32(outfile, red[i]);
      fileBuffWriteFxU32(outfile, green[i]);
      fileBuffWriteFxU32(outfile, blue[i]);
    }
    if (ScreenEcho)
        sprintf(MESSAGE,"grLoadGammaTable()\n");
  }
}

void trap_grQueryResolutions(const GlideResolution *resTemplate,
                             GlideResolution *output,FxI32 result)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grQueryResolutions);
    fileBuffWriteFxU32(outfile, resTemplate->resolution);
    fileBuffWriteFxU32(outfile, resTemplate->refresh);
    fileBuffWriteFxU32(outfile, resTemplate->numColorBuffers);
    fileBuffWriteFxU32(outfile, resTemplate->numAuxBuffers);
    fileBuffWriteFxU32(outfile, (FxU32)output);
    fileBuffWriteFxU32(outfile, result);
    if (ScreenEcho)
        sprintf(MESSAGE,"grQueryResolutions()\n");
  }
}

void trap_grRenderBuffer(GrBuffer_t buffer)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grRenderBuffer);
    fileBuffWriteFxU32(outfile, buffer);
    if (ScreenEcho)
        sprintf(MESSAGE,"grRenderBuffer()\n");
  }
}

void trap_grReset(FxU32 what)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grReset);
    fileBuffWriteFxU32(outfile, what);
    if (ScreenEcho)
        sprintf(MESSAGE,"grReset()\n");
  }
}

void trap_grSelectContext(GrContext_t context)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grSelectContext);
    fileBuffWriteFxU32(outfile, context);
    if (ScreenEcho)
        sprintf(MESSAGE,"grSelectContext()\n");
  }
}

void trap_grSplash(float x,
                   float y,
                   float width,
                   float height,
                   FxU32 frame)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grSplash);
    fileBuffWriteFloat(outfile, x);
    fileBuffWriteFloat(outfile, y);
    fileBuffWriteFloat(outfile, width);
    fileBuffWriteFloat(outfile, height);
    fileBuffWriteFxU32(outfile, frame);
    if (ScreenEcho)
        sprintf(MESSAGE,"grSplash()\n");
  }
}

void trap_grSstOrigin(GrOriginLocation_t origin)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grSstOrigin);
    fileBuffWriteFxU32(outfile, origin);
    if (ScreenEcho)
        sprintf(MESSAGE,"grSstOrigin()\n");
  }
}

void trap_grSstSelect(int which_sst)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grSstSelect);
    fileBuffWriteFxU32(outfile, which_sst);
    if (ScreenEcho)
        sprintf(MESSAGE,"grSstSelect()\n");
  }
}

void trap_grSstWinClose(GrContext_t context)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grSstWinClose);
    fileBuffWriteFxU32(outfile, context);
    fileBuffFlush(outfile);
    if (ScreenEcho)
        sprintf(MESSAGE,"grSstWinClose()\n");
  }
}

void trap_grSstWinOpen(FxU32 hWnd,
                       GrScreenResolution_t screen_resolution,
                       GrScreenRefresh_t refresh_rate,
                       GrColorFormat_t color_format,
                       GrOriginLocation_t origin_location,
                       int nColBuffers,
                       int nAuxBuffers)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grSstWinOpen);
    hWnd = 0;
    fileBuffWriteFxU32(outfile, hWnd);
    fileBuffWriteFxU32(outfile, screen_resolution);
    fileBuffWriteFxU32(outfile, refresh_rate);
    fileBuffWriteFxU32(outfile, color_format);
    fileBuffWriteFxU32(outfile, origin_location);
    fileBuffWriteFxU32(outfile, nColBuffers);
    fileBuffWriteFxU32(outfile, nAuxBuffers);
    if (ScreenEcho)
        sprintf(MESSAGE,"grSstWinOpen()\n");
  }
}

void trap_grTexCalcMemRequired(GrLOD_t lodmin,
                               GrLOD_t lodmax,
                               GrAspectRatio_t aspect,
                               GrTextureFormat_t fmt)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexCalcMemRequired);
    fileBuffWriteFxU32(outfile, lodmin);
    fileBuffWriteFxU32(outfile, lodmax);
    fileBuffWriteFxU32(outfile, aspect);
    fileBuffWriteFxU32(outfile, fmt);
    if (ScreenEcho)
        sprintf(MESSAGE,"grTexCalcMemRequired()\n");
  }
}

void trap_grTexClampMode(GrChipID_t tmu,
                         GrTextureClampMode_t s_clampmode,
                         GrTextureClampMode_t t_clampmode)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexClampMode);
    fileBuffWriteFxU32(outfile, tmu);
    fileBuffWriteFxU32(outfile, s_clampmode);
    fileBuffWriteFxU32(outfile, t_clampmode);
    if (ScreenEcho)
        sprintf(MESSAGE,"grTexClampMode()\n");
  }
}

void trap_grTexCombine(GrChipID_t tmu,
                       GrCombineFunction_t rgb_function,
                       GrCombineFactor_t rgb_factor,
                       GrCombineFunction_t alpha_function,
                       GrCombineFactor_t alpha_factor,
                       FxBool rgb_invert,
                       FxBool alpha_invert)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexCombine);
    fileBuffWriteFxU32(outfile, tmu);
    fileBuffWriteFxU32(outfile, rgb_function);
    fileBuffWriteFxU32(outfile, rgb_factor);
    fileBuffWriteFxU32(outfile, alpha_function);
    fileBuffWriteFxU32(outfile, alpha_factor);
    fileBuffWriteFxU32(outfile, rgb_invert);
    fileBuffWriteFxU32(outfile, alpha_invert);
    if (ScreenEcho)
        sprintf(MESSAGE,"grTexCombine()\n");
  }
}

void trap_grTexDetailControl(GrChipID_t tmu,
                             int lod_bias,
                             FxU8 detail_scale,
                             float detail_max)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexDetailControl);

    fileBuffWriteFxU32(outfile, tmu);
    fileBuffWriteFxU32(outfile, lod_bias);
    fileBuffWriteFxU8 (outfile, detail_scale);
    fileBuffWriteFloat(outfile, detail_max);
    if (ScreenEcho)
        sprintf(MESSAGE,"grTexDetailControl()\n");
  }
}

void WriteGrTexInfo(GrTexInfo *info)
{
  fileBuffWriteFxU32(outfile, info->smallLodLog2);
  fileBuffWriteFxU32(outfile, info->largeLodLog2);
  fileBuffWriteFxU32(outfile, info->aspectRatioLog2);
  fileBuffWriteFxU32(outfile, info->format);
}

void ReadGrTexInfo(FILEBUFF *infile, GrTexInfo *info)
{
  info->smallLodLog2    = fileBuffReadFxU32(infile);
  info->largeLodLog2    = fileBuffReadFxU32(infile);
  info->aspectRatioLog2 = fileBuffReadFxU32(infile);
  info->format          = fileBuffReadFxU32(infile);
}

void trap_grTexDownloadMipMap(GrChipID_t tmu,
                              FxU32 startAddress,
                              FxU32 evenOdd,
                              GrTexInfo *info, FxU32 size)
{
    unsigned long token,old;
    char *filename;

  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexDownloadMipMap);
    
    fileBuffWriteFxU32(outfile, tmu);
    fileBuffWriteFxU32(outfile, startAddress);
    fileBuffWriteFxU32(outfile, evenOdd);
    WriteGrTexInfo(info);
    old=textures->numitems;
    token=find(textures,info->data,size);    
    
    filename=save_texture(token,info->data,size,old!=token, info->format);
    fileBuffWriteFxU8Array(outfile, filename, TEXNAMELEN);

    if (ScreenEcho)
        sprintf(MESSAGE,"grTexDownloadMipMap() - Saved as:%s\n",filename);
    free(filename);
  }
}

void trap_grTexDownloadMipMapLevel(GrChipID_t tmu,
                                   FxU32 startAddress,
                                   GrLOD_t thisLod,
                                   GrLOD_t largeLod,
                                   GrAspectRatio_t aspectRatio,
                                   GrTextureFormat_t format,
                                   FxU32 evenOdd,
                                   void *data,
                                   FxU32 size)
{
    unsigned long token,old;
    char *filename;

  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexDownloadMipMapLevel);
    
    fileBuffWriteFxU32(outfile, tmu);
    fileBuffWriteFxU32(outfile, startAddress);
    fileBuffWriteFxU32(outfile, thisLod);
    fileBuffWriteFxU32(outfile, largeLod);
    fileBuffWriteFxU32(outfile, aspectRatio);
    fileBuffWriteFxU32(outfile, format);
    fileBuffWriteFxU32(outfile, evenOdd);
    
    old=textures->numitems;
    token=find(textures,data,size);    
    
    filename=save_texture(token,data,size,old!=token, format);
    fileBuffWriteFxU8Array(outfile, filename, TEXNAMELEN);
    
    if (ScreenEcho)
        sprintf(MESSAGE,"grTexDownloadMipMapLevel()\n");
    free(filename);
  }
}

void trap_grTexDownloadMipMapLevelPartial(GrChipID_t tmu,
                                          FxU32 startAddress,
                                          GrLOD_t thisLod,
                                          GrLOD_t largeLod,
                                          GrAspectRatio_t aspectRatio,
                                          GrTextureFormat_t format,
                                          FxU32 evenOdd,
                                          void *data,
                                          int start,
                                          int end,
                                          FxU32 size)
{
    unsigned long token,old;
    char *filename;

  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexDownloadMipMapLevelPartial);
    
    fileBuffWriteFxU32(outfile, tmu);
    fileBuffWriteFxU32(outfile, startAddress);
    fileBuffWriteFxU32(outfile, thisLod);
    fileBuffWriteFxU32(outfile, largeLod);
    fileBuffWriteFxU32(outfile, aspectRatio);
    fileBuffWriteFxU32(outfile, format);
    fileBuffWriteFxU32(outfile, evenOdd);    
    fileBuffWriteFxU32(outfile, start);
    fileBuffWriteFxU32(outfile, end);

    old=textures->numitems;
    token=find(textures,data,size);    
    
    filename=save_texture(token,data,size,old!=token, format);
    fileBuffWriteFxU8Array(outfile, filename, TEXNAMELEN);
    
    if (ScreenEcho)
        sprintf(MESSAGE,"grTexDownloadMipMapLevelPartial()\n");
  }
}

void WriteGuNccTable(GuNccTable *ncctab)
{
  int i, j;

  fileBuffWriteFxU8Array(outfile, ncctab->yRGB, 16);
  for ( i=0 ; i<4 ; i++ ) {
    for ( j=0 ; j<3 ; j++ ) {
      fileBuffWriteFxU16(outfile, ncctab->iRGB[i][j]);
      fileBuffWriteFxU16(outfile, ncctab->qRGB[i][j]);
    }
  }
  for ( i=0 ; i<12 ; i++ )
    fileBuffWriteFxU32(outfile, ncctab->packed_data[i]);
}

void ReadGuNccTable(FILEBUFF *infile, GuNccTable *ncctab)
{
  int i, j;

  fileBuffReadFxU8Array(infile, ncctab->yRGB, 16);
  for ( i=0 ; i<4 ; i++ ) {
    for ( j=0 ; j<3 ; j++ ) {
      ncctab->iRGB[i][j] = fileBuffReadFxU16(infile);
      ncctab->qRGB[i][j] = fileBuffReadFxU16(infile);
    }
  }
  for ( i=0 ; i<12 ; i++ )
    ncctab->packed_data[i] = fileBuffReadFxU32(infile);
}

void trap_grTexDownloadTable(GrTexTable_t type,
                             void *data)
{
  int i;
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexDownloadTable);
    
    fileBuffWriteFxU32(outfile, type);

    if ((type == GR_TEXTABLE_PALETTE) ||
        (type == GR_TEXTABLE_PALETTE_6666_EXT)) {
        // write out the GuTexPalette
        for ( i=0 ; i<256 ; i++ )
          fileBuffWriteFxU32(outfile, ((int*)data)[i]);
    } else {
        WriteGuNccTable(data);
    }

    if (ScreenEcho)
        sprintf(MESSAGE,"grTexDownloadTable(%d)\n", (int)type);
  }
}

void trap_grTexDownloadTablePartial(GrTexTable_t type,
                                    void *data,
                                    int start,
                                    int end)
{
  int i;
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexDownloadTablePartial);
    
    fileBuffWriteFxU32(outfile, type);    
    fileBuffWriteFxU32(outfile, start);
    fileBuffWriteFxU32(outfile, end);

    if ((type == GR_TEXTABLE_PALETTE) ||
        (type == GR_TEXTABLE_PALETTE_6666_EXT)) {
        for ( i=start ; i<=end ; i++ )
          fileBuffWriteFxU32(outfile, ((int*)data)[i]);
    } else {
        WriteGuNccTable(data);
    }

    if (ScreenEcho)
        sprintf(MESSAGE,"grTexDownloadTablePartial()\n");
  }
}

void trap_grTexFilterMode(GrChipID_t tmu,
                          GrTextureFilterMode_t minfilter_mode,
                          GrTextureFilterMode_t magfilter_mode)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexFilterMode);
    fileBuffWriteFxU32(outfile, tmu);
    fileBuffWriteFxU32(outfile, minfilter_mode);
    fileBuffWriteFxU32(outfile, magfilter_mode);
    if (ScreenEcho)
        sprintf(MESSAGE,"grTexFilterMode()\n");
  }
}

void trap_grTexLodBiasValue(GrChipID_t tmu,
                            float bias)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexLodBiasValue);
    fileBuffWriteFxU32(outfile, tmu);
    fileBuffWriteFloat(outfile, bias);
    if (ScreenEcho)
        sprintf(MESSAGE,"grTexLodBiasValue()\n");
  }
}

void trap_grTexMaxAddress(GrChipID_t tmu)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexMaxAddress);
    fileBuffWriteFxU32(outfile, tmu);
    if (ScreenEcho)
        sprintf(MESSAGE,"grTexMaxAddress()\n");
  }
}

void trap_grTexMinAddress(GrChipID_t tmu)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexMinAddress);
    fileBuffWriteFxU32(outfile, tmu);
    if (ScreenEcho)
        sprintf(MESSAGE,"grTexMinAddress()\n");
  }
}

void trap_grTexMipMapMode(GrChipID_t tmu,
                          GrMipMapMode_t mode,
                          FxBool lodBlend)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexMipMapMode);
    fileBuffWriteFxU32(outfile, tmu);
    fileBuffWriteFxU32(outfile, mode);
    fileBuffWriteFxU32(outfile, lodBlend);
    if (ScreenEcho)
        sprintf(MESSAGE,"grTexMipMapMode()\n");
  }
}

void trap_grTexMultibase(GrChipID_t tmu,
                         FxBool enable)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexMultibase);
    fileBuffWriteFxU32(outfile, tmu);
    fileBuffWriteFxU32(outfile, enable);
    if (ScreenEcho)
        sprintf(MESSAGE,"grTexMultibase()\n");
  }
}

void trap_grTexMultibaseAddress(GrChipID_t tmu,
                                GrTexBaseRange_t range,
                                FxU32 startAddress,
                                FxU32 evenOdd,
                                GrTexInfo *info)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexMultibaseAddress);
    
    fileBuffWriteFxU32(outfile, tmu);
    fileBuffWriteFxU32(outfile, range);
    fileBuffWriteFxU32(outfile, startAddress);
    fileBuffWriteFxU32(outfile, evenOdd);
    WriteGrTexInfo(info);
    
    if (ScreenEcho)
        sprintf(MESSAGE,"grTexMultibaseAddress()\n");
  }
}

void trap_grTexNCCTable(GrNCCTable_t table)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexNCCTable);
    fileBuffWriteFxU32(outfile, table);
    if (ScreenEcho)
        sprintf(MESSAGE,"grTexNCCTable()\n");
  }
}

void trap_grTexSource(GrChipID_t tmu,
                      FxU32 startAddress,
                      FxU32 evenOdd,
                      GrTexInfo *info)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexSource);
    
    fileBuffWriteFxU32(outfile, tmu);
    fileBuffWriteFxU32(outfile, startAddress);
    fileBuffWriteFxU32(outfile, evenOdd);
    WriteGrTexInfo(info);

    if (ScreenEcho)
        sprintf(MESSAGE,"grTexSource()\n");        
  }
}

void trap_grTexTextureMemRequired(FxU32 evenOdd,
                                  GrTexInfo *info)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grTexTextureMemRequired);
    
    fileBuffWriteFxU32(outfile, evenOdd);
    WriteGrTexInfo(info);
    
    if (ScreenEcho)
        sprintf(MESSAGE,"grTexTextureMemRequired()\n");
  }
}

void trap_grVertexLayout(FxU32 param,
                         FxI32 offset,
                         FxU32 mode)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grVertexLayout);
    my_grVertexLayout(param, offset, mode);
    fileBuffWriteFxU32(outfile, param);
    fileBuffWriteFxU32(outfile, offset);
    fileBuffWriteFxU32(outfile, mode);
    if (ScreenEcho)
        sprintf(MESSAGE,"grVertexLayout()\n");
  }
}

void trap_grViewport(FxI32 x,
                     FxI32 y,
                     FxI32 width,
                     FxI32 height)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_grViewport);
    fileBuffWriteFxU32(outfile, x);
    fileBuffWriteFxU32(outfile, y);
    fileBuffWriteFxU32(outfile, width);
    fileBuffWriteFxU32(outfile, height);
    if (ScreenEcho)
        sprintf(MESSAGE,"grViewport()\n");
  }
}

void trap_gu3dfGetInfo(const char *filename,
                       Gu3dfInfo *info)
{
    int size;

  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_gu3dfGetInfo);
    size=strlen(filename);
    fileBuffWriteFxU32(outfile, size);
    fileBuffWriteFxU8Array(outfile, (char*)filename, size);
    
    if (ScreenEcho)
        sprintf(MESSAGE,"gu3dfGetInfo()\n");
  }
}

void trap_gu3dfLoad(const char *filename,
                    Gu3dfInfo *data)
{
    int size;
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_gu3dfLoad);

    size=strlen(filename);
    fileBuffWriteFxU32(outfile, size);
    fileBuffWriteFxU8Array(outfile, (char*)filename, size);
    
    if (ScreenEcho)
        sprintf(MESSAGE,"gu3dfLoad()\n");
  }
}

void trap_guFogGenerateExp(GrFog_t *fogtable,
                           float density)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_guFogGenerateExp);
    
    fileBuffWriteFloat(outfile, density);
    
    if (ScreenEcho)
        sprintf(MESSAGE,"guFogGenerateExp()\n");
  }
}

void trap_guFogGenerateExp2(GrFog_t *fogtable,
                            float density)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_guFogGenerateExp2);
    
    fileBuffWriteFloat(outfile, density);
    
    if (ScreenEcho)
        sprintf(MESSAGE,"guFogGenerateExp2()\n");
  }
}

void trap_guFogGenerateLinear(GrFog_t *fogtable,
                              float nearZ,
                              float farZ)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_guFogGenerateLinear);
    
    
    fileBuffWriteFloat(outfile, nearZ);
    fileBuffWriteFloat(outfile, farZ);
    
    if (ScreenEcho)
        sprintf(MESSAGE,"guFogGenerateLinear()\n");
  }
}

void trap_guFogTableIndexToW(int i)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_guFogTableIndexToW);
    fileBuffWriteFxU32(outfile, i);
    if (ScreenEcho)
        sprintf(MESSAGE,"guFogTableIndexToW()\n");
  }
}

void trap_guGammaCorrectionRGB(FxFloat red,
                               FxFloat green,
                               FxFloat blue)
{
  if (okToRecord) {
    fileBuffWriteFxU8(outfile, token_guGammaCorrectionRGB);
    fileBuffWriteFloat(outfile, red);
    fileBuffWriteFloat(outfile, green);
    fileBuffWriteFloat(outfile, blue);
    if (ScreenEcho)
        sprintf(MESSAGE,"guGammaCorrectionRGB()\n");
  }
}


/* Playback stuff */
void play_grAADrawTriangle(FILEBUFF *infile)
{

    FxBool ab_antialias;
    FxBool bc_antialias;
    FxBool ca_antialias;

    my_inputvnalloc(&vtxa[0],infile);
    my_inputvnalloc(&vtxb[0],infile);
    my_inputvnalloc(&vtxc[0],infile);

    ab_antialias = fileBuffReadFxU32(infile);
    bc_antialias = fileBuffReadFxU32(infile);
    ca_antialias = fileBuffReadFxU32(infile);    

    if (!nodraw) grAADrawTriangle(&vtxa[0], &vtxb[0], &vtxc[0], ab_antialias, bc_antialias, ca_antialias);    

    if (ScreenEcho)
        printf("grAADrawTriangle()\n");
}

void play_grAlphaBlendFunction(FILEBUFF *infile)
{

    GrAlphaBlendFnc_t rgb_sf   = fileBuffReadFxU32(infile);
    GrAlphaBlendFnc_t rgb_df   = fileBuffReadFxU32(infile);
    GrAlphaBlendFnc_t alpha_sf = fileBuffReadFxU32(infile);
    GrAlphaBlendFnc_t alpha_df = fileBuffReadFxU32(infile);

    grAlphaBlendFunction(rgb_sf, rgb_df, alpha_sf, alpha_df);

    if (ScreenEcho)
        printf("grAlphaBlendFunction(%d,%d,%d,%d)\n",
            (int)rgb_sf, (int)rgb_df, (int)alpha_sf, (int)alpha_df);
}

void play_grAlphaCombine(FILEBUFF *infile)
{

    GrCombineFunction_t function = fileBuffReadFxU32(infile);
    GrCombineFactor_t   factor   = fileBuffReadFxU32(infile);
    GrCombineLocal_t    local    = fileBuffReadFxU32(infile);
    GrCombineOther_t    other    = fileBuffReadFxU32(infile);
    FxBool              invert   = fileBuffReadFxU32(infile);

    grAlphaCombine(function, factor, local, other, invert);

    if (ScreenEcho)
        printf("grAlphaCombine(%d,%d,%d,%d,%d)\n",
            (int)function, (int)factor, (int)local, (int)other, (int)invert);
}

void play_grAlphaControlsITRGBLighting(FILEBUFF *infile)
{

    FxBool enable = fileBuffReadFxU32(infile);

    grAlphaControlsITRGBLighting(enable);

    if (ScreenEcho)
        printf("grAlphaControlsITRGBLighting(%s)\n", enable?"TRUE":"FALSE");
}

void play_grAlphaTestFunction(FILEBUFF *infile)
{

    GrCmpFnc_t function = fileBuffReadFxU32(infile);

    grAlphaTestFunction(function);

    if (ScreenEcho)
        printf("grAlphaTestFunction(%d)\n", (int)function);
}

void play_grAlphaTestReferenceValue(FILEBUFF *infile)
{

    GrAlpha_t value = fileBuffReadFxU8(infile);

    grAlphaTestReferenceValue(value);

    if (ScreenEcho)
        printf("grAlphaTestReferenceValue(%d)\n", (int)value);
}

void play_grBufferClear(FILEBUFF *infile)
{
    GrColor_t color = fileBuffReadFxU32(infile);
    GrAlpha_t alpha = fileBuffReadFxU8(infile);
    FxU32     depth = fileBuffReadFxU32(infile);

    if (!nodraw) grBufferClear(color, alpha, depth);

    if (ScreenEcho)
        printf("grBufferClear(%d, %d, %d)\n", (int)color, (int)alpha, (int)depth);
}

void play_grBufferSwap(FILEBUFF *infile)
{

    int swap_interval = fileBuffReadFxU32(infile);

    grBufferSwap(swap_interval);

    if (ScreenEcho)
        printf("grBufferSwap(%u)\n",swap_interval);
}

void play_grChromakeyMode(FILEBUFF *infile)
{

    GrChromakeyMode_t mode = fileBuffReadFxU32(infile);

    grChromakeyMode(mode);

    if (ScreenEcho)
        printf("grChromakeyMode(%d)\n", (int)mode);
}

void play_grChromakeyValue(FILEBUFF *infile)
{
    GrColor_t value = fileBuffReadFxU32(infile);

    grChromakeyValue(value);

    if (ScreenEcho)
        printf("grChromakeyValue(%x)\n",(int)value);
}

void play_grClipWindow(FILEBUFF *infile)
{

    FxU32 minx = fileBuffReadFxU32(infile);
    FxU32 miny = fileBuffReadFxU32(infile);
    FxU32 maxx = fileBuffReadFxU32(infile);
    FxU32 maxy = fileBuffReadFxU32(infile);

    grClipWindow(minx, miny, maxx, maxy);

    if (ScreenEcho)
        printf("grClipWindow(%d,%d,%d,%d)\n",(int)minx,(int)miny,(int)maxx,(int)maxy);
}

void play_grColorCombine(FILEBUFF *infile)
{

    GrCombineFunction_t function = fileBuffReadFxU32(infile);
    GrCombineFactor_t   factor   = fileBuffReadFxU32(infile);
    GrCombineLocal_t    local    = fileBuffReadFxU32(infile);
    GrCombineOther_t    other    = fileBuffReadFxU32(infile);
    FxBool              invert   = fileBuffReadFxU32(infile);

    grColorCombine(function, factor, local, other, invert);

    if (ScreenEcho)
        printf("grColorCombine(%d,%d,%d,%d,%d)\n",
            (int)function,(int)factor,(int)local,(int)other,(int)invert);
}

void play_grColorMask(FILEBUFF *infile)
{

    FxBool rgb = fileBuffReadFxU32(infile);
    FxBool a   = fileBuffReadFxU32(infile);

    grColorMask(rgb, a);

    if (ScreenEcho)
        printf("grColorMask()\n");
}

void play_grConstantColorValue(FILEBUFF *infile)
{

    GrColor_t value = fileBuffReadFxU32(infile);

    grConstantColorValue(value);

    if (ScreenEcho)
        printf("grConstantColorValue(%x)\n",(int)value);
}

void play_grCoordinateSpace(FILEBUFF *infile)
{
    GrCoordinateSpaceMode_t mode = fileBuffReadFxU32(infile);

    grCoordinateSpace(mode);

    if (ScreenEcho)
        printf("grCoordinateSpace(%d)\n", (int)mode);
}

void play_grCullMode(FILEBUFF *infile)
{
    GrCullMode_t mode = fileBuffReadFxU32(infile);

    grCullMode(mode);

    if (ScreenEcho)
        printf("grCullMode(%d)\n", (int)mode);
}

void play_grDepthBiasLevel(FILEBUFF *infile)
{
    FxI32 level = fileBuffReadFxU32(infile);

    grDepthBiasLevel(level);

    if (ScreenEcho)
        printf("grDepthBiasLevel(%d)\n", (int)level);
}

void play_grDepthBufferFunction(FILEBUFF *infile)
{
    GrCmpFnc_t function = fileBuffReadFxU32(infile);

    grDepthBufferFunction(function);

    if (ScreenEcho)
        printf("grDepthBufferFunction(%d)\n", (int)function);
}

void play_grDepthBufferMode(FILEBUFF *infile)
{
    GrDepthBufferMode_t mode = fileBuffReadFxU32(infile);

    grDepthBufferMode(mode);

    if (ScreenEcho)
        printf("grDepthBufferMode(%d)\n", (int)mode);
}

void play_grDepthMask(FILEBUFF *infile)
{
    FxBool mask = fileBuffReadFxU32(infile);

    grDepthMask(mask);

    if (ScreenEcho)
        printf("grDepthMask(%u)\n",mask);
}

void play_grDepthRange(FILEBUFF *infile)
{
    FxFloat n = fileBuffReadFloat(infile);
    FxFloat f = fileBuffReadFloat(infile);

    grDepthRange(n, f);

    if (ScreenEcho)
        printf("grDepthRange(%f,%f)\n",n,f);
}

void play_grDisable(FILEBUFF *infile)
{

    GrEnableMode_t mode = fileBuffReadFxU32(infile);

    grDisable(mode);

    if (ScreenEcho)
        printf("grDisable(%d)\n", (int)mode);
}

void play_grDisableAllEffects(FILEBUFF *infile)
{
    grDisableAllEffects();

    if (ScreenEcho)
        printf("grDisableAllEffects()\n");
}

void play_grDitherMode(FILEBUFF *infile)
{
    GrDitherMode_t mode = fileBuffReadFxU32(infile);

    grDitherMode(mode);

    if (ScreenEcho)
        printf("grDitherMode(%d)\n", (int)mode);
}

void play_grDrawLine(FILEBUFF *infile)
{
    my_inputvnalloc(&vtxa[0],infile);
    my_inputvnalloc(&vtxb[0],infile);
    if (!nodraw) grDrawLine(&vtxa[0],&vtxb[0]);

    if (ScreenEcho)
        printf("grDrawLine( (%4f,% 4f) - (%4f,%4f) )\n",
            *(float *) (gc.vertexInfo.offset + (int) &vtxa[0]),
            *(float *) (gc.vertexInfo.offset + 4+(int) &vtxa[0]),
            *(float *) (gc.vertexInfo.offset + (int) &vtxb[0]),
            *(float *) (gc.vertexInfo.offset + 4+(int) &vtxb[0]));
}

void play_grDrawPoint(FILEBUFF *infile)
{
    my_inputvnalloc(&vtxa[0],infile);
    if (!nodraw) grDrawPoint(&vtxa[0]);

    if (ScreenEcho) {
        printf("grDrawPoint( %4f,%4f",        
            *(float *) (gc.vertexInfo.offset + (int) &vtxa[0]),
            *(float *) (gc.vertexInfo.offset + 4+(int) &vtxa[0]));
        printf(" )\n");
    }

}

void play_grDrawTriangle(FILEBUFF *infile)
{
    my_inputvnalloc(&vtxa[0],infile);
    my_inputvnalloc(&vtxb[0],infile);
    my_inputvnalloc(&vtxc[0],infile);
    
    if (ScreenEcho) {
        printf("grDrawTriangle( ");
        printf(" (%4f,%4f) ",*(float *) (gc.vertexInfo.offset + (int) &vtxa[0]),*(float *) (gc.vertexInfo.offset + 4+(int) &vtxa[0]));
        printf(" (%4f,%4f) ",*(float *) (gc.vertexInfo.offset + (int) &vtxb[0]),*(float *) (gc.vertexInfo.offset + 4+(int) &vtxb[0]));
        printf(" (%4f,%4f) ",*(float *) (gc.vertexInfo.offset + (int) &vtxc[0]),*(float *) (gc.vertexInfo.offset + 4+(int) &vtxc[0]));
        printf(" )\n");
    }

    if (!nodraw) grDrawTriangle(&vtxa[0],&vtxb[0],&vtxc[0]);    
}

void play_grDrawVertexArray(FILEBUFF *infile)
{
    FxU32 i;
    FxU32 mode  = fileBuffReadFxU32(infile);
    FxU32 Count = fileBuffReadFxU32(infile);
    void **pointers;    

    pointers=(void **) malloc(sizeof(void*)*Count);
    for (i=0; i<Count; i++)
        pointers[i]=my_inputvertex(infile);

    if (!nodraw) grDrawVertexArray(mode, Count, pointers);

    for (i=0; i<Count; i++)
        free(pointers[i]);
    free(pointers);

    if (ScreenEcho)
        printf("grDrawVertexArray()\n");
}

void play_grDrawVertexArrayContiguous(FILEBUFF *infile)
{
    FxU32 mode   = fileBuffReadFxU32(infile);
    FxU32 Count  = fileBuffReadFxU32(infile);    
    FxU32 stride = fileBuffReadFxU32(infile);
    void *pointers = malloc(stride*Count);
    int   i, size_in_words = (stride * Count) >> 2;

    for ( i=0 ; i<size_in_words ; i++ )
      ((FxU32*)pointers)[i] = fileBuffReadFxU32(infile);    

    if (!nodraw) grDrawVertexArrayContiguous(mode, Count, pointers, stride);
    free(pointers);

    if (ScreenEcho)
        printf("grDrawVertexArrayContiguous()\n");
}

void play_grEnable(FILEBUFF *infile)
{
    GrEnableMode_t mode = fileBuffReadFxU32(infile);
    grEnable(mode);

    if (ScreenEcho)
        printf("grEnable(%d)\n", (int)mode);
}

void play_grErrorSetCallback(FILEBUFF *infile)
{
/*
    GrErrorCallbackFnc_t fnc;
    fileBuffRead(infile, &fnc, sizeof(GrErrorCallbackFnc_t));
    grErrorSetCallback(fnc);
*/
    if (ScreenEcho)
        printf("grErrorSetCallback()\n");
}

void play_grFinish(FILEBUFF *infile)
{
    grFinish();

    if (ScreenEcho)
        printf("grFinish()\n");
}

void play_grFlush(FILEBUFF *infile)
{
    grFlush();

    if (ScreenEcho)
        printf("grFlush()\n");
}

void play_grFogColorValue(FILEBUFF *infile)
{
    GrColor_t fogcolor = fileBuffReadFxU32(infile);

    grFogColorValue(fogcolor);

    if (ScreenEcho)
        printf("grFogColorValue(%x)\n",(int)fogcolor);
}

void play_grFogMode(FILEBUFF *infile)
{
    GrFogMode_t mode = fileBuffReadFxU32(infile);

    grFogMode(mode);

    if (ScreenEcho)
        printf("grFogMode(%d)\n",(int)mode);
}

void play_grFogTable(FILEBUFF *infile)
{
    FxU32 tsize = fileBuffReadFxU32(infile);
    GrFog_t *ft = (GrFog_t *) malloc(tsize*sizeof(GrFog_t));

    fileBuffReadFxU8Array(infile, ft, tsize);

    grFogTable(ft);
    free(ft);
    if (ScreenEcho)
        printf("grFogTable()\tFog table size=%d\n",(int)tsize);
}

void play_grGet(FILEBUFF *infile)
{
    FxU32 pname   = fileBuffReadFxU32(infile);
    FxU32 plength = fileBuffReadFxU32(infile);
    char *params;

    if (pname==GR_GLIDE_STATE_SIZE) {
        grGet(GR_GLIDE_STATE_SIZE,4,&state_size);        

    } else if (pname==GR_GLIDE_VERTEXLAYOUT_SIZE) {
        grGet(GR_GLIDE_VERTEXLAYOUT_SIZE,4,&vlayout_size);        

    } else {
        params=(char *) malloc(sizeof(char)*plength);
        grGet(pname, plength, (long *) params);
        free(params);
    }
    if (ScreenEcho)
        printf("grGet(%d,%d)\n",(int)pname,(int)plength);
}

void play_grGetProcAddress(FILEBUFF *infile)
{
    int size = fileBuffReadFxU32(infile);
    char *procName = malloc(size+2);
    char msg[500];
    GrProc ext;

    fileBuffReadFxU8Array(infile, procName, size);
    procName[size]=0;
    ext=grGetProcAddress(procName);
    
    if (ScreenEcho)
        printf("grGetProcAddress(%s)\n",procName);        

    if (!strcmp(procName, "grChromaRangeModeExt")) {
        MYEXT_grChromaRangeModeExt=ext;
        free(procName);
        return;
    }    

    if (!strcmp(procName, "grChromaRangeExt")) {
        MYEXT_grChromaRangeExt=ext;
        free(procName);
        return;
    }

    if (!strcmp(procName, "grTexChromaModeExt")) {
        MYEXT_grTexChromaModeExt=ext;
        free(procName);
        return;
    }

    if (!strcmp(procName, "grTexChromaRangeExt")) {
        MYEXT_grTexChromaRangeExt=ext;
        free(procName);
        return;
    }

    if (!strcmp(procName, "grDrawTextureLineExt")) {
        MYEXT_grDrawTextureLineExt=ext;
        free(procName);
        return;
    }

    if (ext==NULL) {
            sprintf(msg,"Process requested for non-existent extension %s\n",procName);
#ifdef __WIN3222222__  // disable the message box because it's annoying
            MessageBox(NULL,msg,"ERROR",MB_OK | MB_ICONSTOP);
#else
            fprintf(stderr, "%s", msg);
#endif
            free(procName);
            return;
    }    
    return;
}

void play_grGetString(FILEBUFF *infile)
{

    FxU32 pname = fileBuffReadFxU32(infile);
    char *result = (char*)grGetString(pname);

    if (ScreenEcho)
        printf("grGetString(%d)\tReturned: %s\n",(int)pname,result);
}

void play_grGlideGetState(FILEBUFF *infile)
{
    FxU32 i;
    FxU32 statenum = fileBuffReadFxU32(infile);
    void *state;
    
    if (state_size==0) {
        printf("Trapping program tried to grGlideGetState without doing a grGet(GR_GLIDE_STATE_SIZE)!\n");
        grGet(GR_GLIDE_STATE_SIZE, 4, &state_size);
    }

    state=(void *) malloc(sizeof(char)*state_size);
    memset(state,0,sizeof(char)*state_size);
    grGlideGetState(state);

    if (statenum>=numstates) {
        /* this is a new one! */
        numstates++;
        if (numstates>=state_table_size)
        {
            realloc(play_states,2*sizeof(void*)*state_table_size);
            for (i=state_table_size; i<2*state_table_size;i++)
                play_states[i]=NULL;
            state_table_size=2*state_table_size;
        }
        play_states[statenum]=state;        
    } else {
        free(state);
    }

    if (ScreenEcho)
        printf("grGlideGetState()\tState: %d\n",(int)statenum);
}

void play_grGlideGetVertexLayout(FILEBUFF *infile)
{
    FxU32 i;
    FxU32 vlayoutnum = fileBuffReadFxU32(infile);
    void *vlayout;
    
    if (vlayout_size==0) {
        printf("Trapping program tried to grGlideGetVertexLayout without doing a grGet(GR_GLIDE_VERTEXLAYOUT_SIZE)!\n");
        grGet(GR_GLIDE_VERTEXLAYOUT_SIZE, 4, &vlayout_size);
    }

    vlayout=(void *) malloc(sizeof(char)*vlayout_size);
    memset(vlayout,0,sizeof(char)*vlayout_size);
    grGlideGetVertexLayout(vlayout);

    if (vlayoutnum>=numvlayouts) {
        /* this is a new one! */
        numvlayouts++;
        if (numvlayouts>=vlayout_table_size)
        {
            realloc(play_vlayouts,2*sizeof(void*)*vlayout_table_size);
            for (i=vlayout_table_size; i<2*vlayout_table_size;i++)
                play_vlayouts[i]=NULL;
            vlayout_table_size=2*vlayout_table_size;
        }
        play_vlayouts[vlayoutnum]=vlayout;        
    } else {
        free(vlayout);
    }

    if (ScreenEcho)
        printf("grGlideGetVertexLayout()\tVLayout: %d\n",(int)vlayoutnum);
}

void play_grGlideInit(FILEBUFF *infile)
{
    grGlideInit();
    if (ScreenEcho)
        printf("grGlideInit()\n");
}

void play_grGlideSetState(FILEBUFF *infile)
{
    FxU32 token = fileBuffReadFxU32(infile);
    
    if (token>numstates) {
        printf("grGlideSetState(): ERROR! Invalid State: %d\n",(int)token);
        GETCH();
        exit(1);
    } else {
        grGlideSetState(play_states[token]);
        if (ScreenEcho)
            printf("grGlideSetState(State: %d)\n",(int)token);
    }
}

void play_grGlideSetVertexLayout(FILEBUFF *infile)
{
    FxU32 token = fileBuffReadFxU32(infile);
    
    if (token>numvlayouts) {
        printf("grGlideSetVertexLayout(): ERROR! Invalid VLayout: %d\n",(int)token);
        GETCH();
        exit(1);
    } else {
        grGlideSetVertexLayout(play_vlayouts[token]);
        if (ScreenEcho)
            printf("grGlideSetVertexLayout(VLayout: %d)\n",(int)token);
    }
}

void play_grGlideShutdown(FILEBUFF *infile)
{

    grGlideShutdown();
    if (ScreenEcho)
        printf("grGlideShutdown()\n");
}

void play_grLfbConstantAlpha(FILEBUFF *infile)
{

    GrAlpha_t alpha = fileBuffReadFxU8(infile);
    grLfbConstantAlpha(alpha);

    if (ScreenEcho)
        printf("grLfbConstantAlpha(%u)\n",alpha);
}

void play_grLfbConstantDepth(FILEBUFF *infile)
{

    FxU32 depth = fileBuffReadFxU32(infile);
    grLfbConstantDepth(depth);

    if (ScreenEcho)
        printf("grLfbConstantDepth(%d)\n",(int)depth);
}

void play_grLfbLock(FILEBUFF *infile)
{
    int countDown=10;
    GrLock_t type              = fileBuffReadFxU32(infile);
    GrBuffer_t buffer          = fileBuffReadFxU32(infile);
    GrLfbWriteMode_t writeMode = fileBuffReadFxU32(infile);
    GrOriginLocation_t origin  = fileBuffReadFxU32(infile);
    FxBool pixelPipeline       = fileBuffReadFxU32(infile);
    FxBool result              = fileBuffReadFxU32(infile);
    FxBool res;
    
    if (!result) {
        printf("Trap hardware failed on grLfbLock(). Ignoring lock!\n");        
    } else {
        res=FXFALSE;
        while (!res && countDown)
        {
            switch (buffer)
            {
            case GR_BUFFER_FRONTBUFFER:
                lfbf.size = sizeof(GrLfbInfo_t);
                res=grLfbLock(type, buffer, writeMode, origin, pixelPipeline, &lfbf);
                if (res) frontlocked |= (1<<type);
                else     frontlocked &= ~(1<<type);
                break;
            case GR_BUFFER_BACKBUFFER:
                lfbb.size = sizeof(GrLfbInfo_t);
                res=grLfbLock(type, buffer, writeMode, origin, pixelPipeline, &lfbb);
                if (res) backlocked |= (1<<type);
                else     backlocked &= ~(1<<type);
                break;
            case GR_BUFFER_AUXBUFFER:
                lfba.size = sizeof(GrLfbInfo_t);
                res=grLfbLock(type, buffer, writeMode, origin, pixelPipeline, &lfba);
                if (res) auxlocked |= (1<<type);
                else     auxlocked &= ~(1<<type);
                break;
            default:
                printf("Tried to grLfbLock()unknown buffer %d !!",(int)buffer);
                exit(1);
                break;
            } /* of switch */
            if (!res) {
              if (countDown == 10)
                printf("Locked failed! Retrying ... %d", countDown--);
              else
                printf(" %d", countDown--);
            }
        } /* while */
        if (countDown != 10) {
          if (countDown<=0)
            printf(", giving up\n");
          else
            printf(", got it!\n");
        }
    }
    

    if (ScreenEcho)
        printf("grLfbLock(%d,%d,%d,%d,%d)\tReturned %d\n",
            (int)type,(int)buffer,(int)writeMode,(int)origin,(int)pixelPipeline,(int)res);
}

void play_grLfbReadRegion(FILEBUFF *infile)
{

    GrBuffer_t src_buffer = fileBuffReadFxU32(infile);
    FxU32 src_x           = fileBuffReadFxU32(infile);
    FxU32 src_y           = fileBuffReadFxU32(infile);
    FxU32 src_width       = fileBuffReadFxU32(infile);
    FxU32 src_height      = fileBuffReadFxU32(infile);
    FxU32 dst_stride      = fileBuffReadFxU32(infile);
    void *dst_data        = malloc(src_height*dst_stride);

    grLfbReadRegion(src_buffer, src_x, src_y, src_width, src_height, dst_stride, dst_data);
    free(dst_data);

    if (ScreenEcho)
        printf("grLfbReadRegion()\n");
}

void play_grLfbUnlock(FILEBUFF *infile)
{

    GrLock_t type      = fileBuffReadFxU32(infile);
    GrBuffer_t buffer  = fileBuffReadFxU32(infile);

    switch (buffer) {
    case GR_BUFFER_FRONTBUFFER:
        if (!(frontlocked & (1<<type))) {
            printf("IGNORING grLfbUnlock: Tried to unlock Front Buffer without lock!\n");
            return;
        }        
        frontlocked &= ~(1<<type);
        break;
    case GR_BUFFER_BACKBUFFER:
        if (!(backlocked & (1<<type))) {
            printf("IGNORING grLfbUnlock: Tried to unlock Back Buffer without lock!\n");
            return;
        }        
        backlocked &= ~(1<<type);
        break;

    case GR_BUFFER_AUXBUFFER:
        if (!(auxlocked & (1<<type))) {
            printf("IGNORING grLfbUnlock: Tried to unlock Aux Buffer without lock!\n");
            return;
        }        
        auxlocked &= ~(1<<type);
        break;
    default:
        printf("IGNORING grLfbUnlock: Tried to unlock unknown buffer %d!\n",(int)buffer);
            return;
        break;
    }

    grLfbUnlock(type, buffer);

    if (ScreenEcho)
        printf("grLfbUnlock(%d,%d)\n",(int)type,(int)buffer);
}

void play_grLfbWriteColorFormat(FILEBUFF *infile)
{

    GrColorFormat_t colorFormat = fileBuffReadFxU32(infile);
    grLfbWriteColorFormat(colorFormat);

    if (ScreenEcho)
        printf("grLfbWriteColorFormat(%d)\n",(int)colorFormat);
}

void play_grLfbWriteColorSwizzle(FILEBUFF *infile)
{

    FxBool swizzleBytes = fileBuffReadFxU32(infile);
    FxBool swapWords    = fileBuffReadFxU32(infile);
    grLfbWriteColorSwizzle(swizzleBytes, swapWords);

    if (ScreenEcho)
        printf("grLfbWriteColorSwizzle(%u,%u)\n",swizzleBytes,swapWords);
}

void play_grLfbWriteRegion(FILEBUFF *infile)
{

    GrBuffer_t dst_buffer    = fileBuffReadFxU32(infile);
    FxU32 dst_x              = fileBuffReadFxU32(infile);
    FxU32 dst_y              = fileBuffReadFxU32(infile);
    GrLfbSrcFmt_t src_format = fileBuffReadFxU32(infile);
    FxU32 src_width          = fileBuffReadFxU32(infile);
    FxU32 src_height         = fileBuffReadFxU32(infile);
    FxBool pixelPipeline     = fileBuffReadFxU32(infile);
    FxI32 src_stride         = fileBuffReadFxU32(infile);
    FxBool invalid;
    void * src_data;
    char filename[TEXNAMELEN+1];

    fileBuffReadFxU8Array(infile, filename, TEXNAMELEN);

    invalid=!strncmp(filename,INVALID,strlen(INVALID));

    if (invalid) {
        grLfbWriteRegion(dst_buffer, dst_x, dst_y, src_format, src_width, src_height, pixelPipeline, src_stride, NULL);        
    } else {
        src_data=load_lfb(filename, src_format);
        grLfbWriteRegion(dst_buffer, dst_x, dst_y, src_format, src_width, src_height, pixelPipeline, src_stride, src_data);        
        free(src_data);
    }

    if (ScreenEcho)
        printf("grLfbWriteRegion()\tFilename: %s\n",filename);        
}

void play_grLoadGammaTable(FILEBUFF *infile)
{
    FxU32 i;
    FxU32 nentries = fileBuffReadFxU32(infile);
    FxU32 *red   = malloc(3 * sizeof(FxU32) * nentries);
    FxU32 *green = red + nentries;
    FxU32 *blue  = green + nentries;

    for ( i=0 ; i<nentries ; i++ ) {
      red  [i] = fileBuffReadFxU32(infile);
      green[i] = fileBuffReadFxU32(infile);
      blue [i] = fileBuffReadFxU32(infile);
    }

    grLoadGammaTable(nentries, red, green, blue);
    free(red);

    if (ScreenEcho)
        printf("grLoadGammaTable()\n");
}

void play_grQueryResolutions(FILEBUFF *infile)
{
    GlideResolution resTemplate;
    GlideResolution *output;
    FxI32 result1, result2;
    void *tmp;

    resTemplate.resolution      = fileBuffReadFxU32(infile);
    resTemplate.refresh         = fileBuffReadFxU32(infile);
    resTemplate.numColorBuffers = fileBuffReadFxU32(infile);
    resTemplate.numAuxBuffers   = fileBuffReadFxU32(infile);
    output    = (GlideResolution*)fileBuffReadFxU32(infile);
    result1                     = fileBuffReadFxU32(infile);
    
    if (output!=NULL) {
        if (sizeofres==0) {
            printf("Fatal Error! Unknown GrResolution size on QueryResoltions!\n");
            exit(1);
        }
        tmp=malloc(sizeofres);
        result2=grQueryResolutions(&resTemplate, tmp);
        free(tmp);
    } else {
        sizeofres=result2=grQueryResolutions(&resTemplate, output);
    }
    
    if (ScreenEcho) 
        printf("grQueryResolutions()\tResult: %d\tExpected: %d\n",
            (int)result2, (int)result1);
}

void play_grRenderBuffer(FILEBUFF *infile)
{
    GrBuffer_t buffer = fileBuffReadFxU32(infile);

    grRenderBuffer(buffer);

    if (ScreenEcho)
        printf("grRenderBuffer(%d)\n",(int)buffer);
}

void play_grReset(FILEBUFF *infile)
{
    FxU32 what = fileBuffReadFxU32(infile);
    grReset(what);

    if (ScreenEcho)
        printf("grReset(%d)\n",(int)what);
}

void play_grSelectContext(FILEBUFF *infile)
{
    GrContext_t context = fileBuffReadFxU32(infile);  // consume context
    grSelectContext(play_context);

    if (ScreenEcho)
        printf("grSelectContext(%d)\n",(int)play_context);
}

void play_grSplash(FILEBUFF *infile)
{
    float x      = fileBuffReadFloat(infile);
    float y      = fileBuffReadFloat(infile);
    float width  = fileBuffReadFloat(infile);
    float height = fileBuffReadFloat(infile);
    FxU32 frame  = fileBuffReadFxU32(infile);

    grSplash(x, y, width, height, frame);

    if (ScreenEcho)
        printf("grSplash(%f,%f,%f,%f,%d)\n",x,y,width,height,(int)frame);
}

void play_grSstOrigin(FILEBUFF *infile)
{
    GrOriginLocation_t origin = fileBuffReadFxU32(infile);
    grSstOrigin(origin);

    if (ScreenEcho)
        printf("grSstOrigin(%d)\n",(int)origin);
}

void play_grSstSelect(FILEBUFF *infile)
{
    int which_sst = fileBuffReadFxU32(infile);
    grSstSelect(which_sst);

    if (ScreenEcho)
        printf("grSstSelect(%i)\n",which_sst);
}

void play_grSstWinClose(FILEBUFF *infile)
{
    GrContext_t context = fileBuffReadFxU32(infile);
    
    grSstWinClose(0);

    if (ScreenEcho)
        printf("grSstWinClose(%d)\tThis computer closes 0\n",(int)context);
}

void play_grSstWinOpen(FILEBUFF *infile)
{
    FxU32 hWnd                             = fileBuffReadFxU32(infile);
    GrScreenResolution_t screen_resolution = fileBuffReadFxU32(infile);
    GrScreenRefresh_t refresh_rate         = fileBuffReadFxU32(infile);
    GrColorFormat_t color_format           = fileBuffReadFxU32(infile);
    GrOriginLocation_t origin_location     = fileBuffReadFxU32(infile);
    int nColBuffers                        = fileBuffReadFxU32(infile);
    int nAuxBuffers                        = fileBuffReadFxU32(infile);

    hWnd = 33;
    play_context = grSstWinOpen(hWnd, screen_resolution, refresh_rate, color_format, origin_location, nColBuffers, nAuxBuffers);

    if (ScreenEcho)
        printf("grSstWinOpen(%d,%d,%d,%d,%d,%d,%d) -- This computer calls with param 0=0\n",
            (int)hWnd,(int)screen_resolution, (int)refresh_rate,(int)color_format,
            (int)origin_location,(int)nColBuffers,(int)nAuxBuffers);

    g_screen_resolution = screen_resolution;
}

void play_grTexCalcMemRequired(FILEBUFF *infile)
{
    FxU32 result;
    GrLOD_t lodmin         = fileBuffReadFxU32(infile);
    GrLOD_t lodmax         = fileBuffReadFxU32(infile);
    GrAspectRatio_t aspect = fileBuffReadFxU32(infile);
    GrTextureFormat_t fmt  = fileBuffReadFxU32(infile);

    result=grTexCalcMemRequired(lodmin, lodmax, aspect, fmt);

    if (ScreenEcho)
        printf("grTexCalcMemRequired(%d,%d,%d,%d)\tReturned: %d\n",
            (int)lodmin,(int)lodmax,(int)aspect,(int)fmt,(int)result);
}

void play_grTexClampMode(FILEBUFF *infile)
{
    GrChipID_t tmu                   = fileBuffReadFxU32(infile);
    GrTextureClampMode_t s_clampmode = fileBuffReadFxU32(infile);
    GrTextureClampMode_t t_clampmode = fileBuffReadFxU32(infile);
 
    grTexClampMode(tmu, s_clampmode, t_clampmode);

    if (ScreenEcho)
        printf("grTexClampMode(%d, %d, %d)\n", (int)tmu, (int)s_clampmode, (int)t_clampmode);
}

void play_grTexCombine(FILEBUFF *infile)
{
    GrChipID_t tmu                     = fileBuffReadFxU32(infile);
    GrCombineFunction_t rgb_function   = fileBuffReadFxU32(infile);
    GrCombineFactor_t rgb_factor       = fileBuffReadFxU32(infile);
    GrCombineFunction_t alpha_function = fileBuffReadFxU32(infile);
    GrCombineFactor_t alpha_factor     = fileBuffReadFxU32(infile);
    FxBool rgb_invert                  = fileBuffReadFxU32(infile);
    FxBool alpha_invert                = fileBuffReadFxU32(infile);

    grTexCombine(tmu, rgb_function, rgb_factor, alpha_function, alpha_factor, rgb_invert, alpha_invert);

    if (ScreenEcho)
        printf("grTexCombine(%d,%d,%d,%d,%d,%d,%d)\n",
            (int)tmu, (int)rgb_function, (int)rgb_factor, (int)alpha_function,
            (int)alpha_factor, (int)rgb_invert, (int)alpha_invert);
}

void play_grTexDetailControl(FILEBUFF *infile)
{
    GrChipID_t tmu    = fileBuffReadFxU32(infile);
    int lod_bias      = fileBuffReadFxU32(infile);
    FxU8 detail_scale = fileBuffReadFxU8 (infile);
    float detail_max  = fileBuffReadFloat(infile);

    grTexDetailControl(tmu, lod_bias, detail_scale, detail_max);

    if (ScreenEcho)
        printf("grTexDetailControl(%d,%d,%d,%f)\n",
            (int)tmu,(int)lod_bias,(int)detail_scale,detail_max);
}

void play_grTexDownloadMipMap(FILEBUFF *infile)
{
    GrChipID_t tmu     = fileBuffReadFxU32(infile);
    FxU32 startAddress = fileBuffReadFxU32(infile);
    FxU32 evenOdd      = fileBuffReadFxU32(infile);
    GrTexInfo info;
    char filename[TEXNAMELEN+1];

    ReadGrTexInfo(infile, &info);
    
    fileBuffReadFxU8Array(infile, filename, TEXNAMELEN);
    load_texture(filename, &info.data, info.format);
    
    grTexDownloadMipMap(tmu, startAddress, evenOdd, &info);

    if (ScreenEcho)
        printf("grTexDownloadMipMap(%d,%d,%d,(%s,%s,%s,%s,0x%x,[%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,...])) -- Loaded %s\n",
            (int)tmu,
            (int)startAddress,
            (int)evenOdd,
            strLodLog2(info.smallLodLog2),
            strLodLog2(info.largeLodLog2),
            strAspectRatioLog2(info.aspectRatioLog2),
            strTextureFormat(info.format),
            (int)info.data,
            ((char*)info.data)[0],
            ((char*)info.data)[1],
            ((char*)info.data)[2],
            ((char*)info.data)[3],
            ((char*)info.data)[4],
            ((char*)info.data)[5],
            ((char*)info.data)[6],
            ((char*)info.data)[7],
            ((char*)info.data)[8],
            ((char*)info.data)[9],
            filename);
    
    free(info.data);
}

void play_grTexDownloadMipMapLevel(FILEBUFF *infile)
{
    GrChipID_t tmu              = fileBuffReadFxU32(infile);
    FxU32 startAddress          = fileBuffReadFxU32(infile);
    GrLOD_t thisLod             = fileBuffReadFxU32(infile);
    GrLOD_t largeLod            = fileBuffReadFxU32(infile);
    GrAspectRatio_t aspectRatio = fileBuffReadFxU32(infile);
    GrTextureFormat_t format    = fileBuffReadFxU32(infile);
    FxU32 evenOdd               = fileBuffReadFxU32(infile);
    void *data;
    char filename[TEXNAMELEN+1];

    fileBuffReadFxU8Array(infile, filename, TEXNAMELEN);

    data=NULL;
    load_texture(filename, &data, format);

    if (ScreenEcho)
        printf("grTexDownloadMipMapLevel(TMU %d, Start %d,this %d, large %d) File %s\n",
            (int)tmu,(int)startAddress,(int)thisLod,(int)largeLod,filename);

    if (data==NULL) {
        printf("Texture did not load!\n");
        GETCH();
    }
    
    grTexDownloadMipMapLevel(tmu, startAddress, thisLod, largeLod, aspectRatio, format, evenOdd, data);

    free(data);
}

void play_grTexDownloadMipMapLevelPartial(FILEBUFF *infile)
{
    GrChipID_t tmu              = fileBuffReadFxU32(infile);
    FxU32 startAddress          = fileBuffReadFxU32(infile);
    GrLOD_t thisLod             = fileBuffReadFxU32(infile);
    GrLOD_t largeLod            = fileBuffReadFxU32(infile);
    GrAspectRatio_t aspectRatio = fileBuffReadFxU32(infile);
    GrTextureFormat_t format    = fileBuffReadFxU32(infile);
    FxU32 evenOdd               = fileBuffReadFxU32(infile);
    int start                   = fileBuffReadFxU32(infile);
    int end                     = fileBuffReadFxU32(infile);
    char filename[TEXNAMELEN+1];
    void *data;

    fileBuffReadFxU8Array(infile, filename, TEXNAMELEN);

    load_texture(filename, &data, format);
    
    grTexDownloadMipMapLevelPartial(tmu, startAddress, thisLod, largeLod, aspectRatio, format, evenOdd, data, start, end);

    if (ScreenEcho)
        printf("grTexDownloadMipMapLevelPartial()\n");

    free(data);
}

void play_grTexDownloadTable(FILEBUFF *infile)
{
    int i;
    GrTexTable_t type = fileBuffReadFxU32(infile);

    if ((type == GR_TEXTABLE_PALETTE) ||
        (type == GR_TEXTABLE_PALETTE_6666_EXT)) {
        GuTexPalette tex_pal;
        for ( i=0 ; i<256 ; i++ )
          tex_pal.data[i] = fileBuffReadFxU32(infile);
        grTexDownloadTable(type, &tex_pal);
    } else {
        GuNccTable ncc_tab;
        ReadGuNccTable(infile, &ncc_tab);
        grTexDownloadTable(type, &ncc_tab);
    }

    if (ScreenEcho)
        printf("grTexDownloadTable(%d)\n",(int)type);
}

void play_grTexDownloadTablePartial(FILEBUFF *infile)
{
    int i;
    GrTexTable_t type = fileBuffReadFxU32(infile);
    int start         = fileBuffReadFxU32(infile);
    int end           = fileBuffReadFxU32(infile);

    if ((type == GR_TEXTABLE_PALETTE) ||
        (type == GR_TEXTABLE_PALETTE_6666_EXT)) {
        GuTexPalette tex_pal;
        for ( i=start ; i<=end ; i++ )
          tex_pal.data[i] = fileBuffReadFxU32(infile);
        grTexDownloadTablePartial(type, &tex_pal, start, end);
    } else {
        GuNccTable ncc_tab;
        ReadGuNccTable(infile, &ncc_tab);
        grTexDownloadTablePartial(type, &ncc_tab, start, end);
    }        

    if (ScreenEcho)
        printf("grTexDownloadTablePartial()\n");
}

void play_grTexFilterMode(FILEBUFF *infile)
{
    GrChipID_t tmu                       = fileBuffReadFxU32(infile);
    GrTextureFilterMode_t minfilter_mode = fileBuffReadFxU32(infile);
    GrTextureFilterMode_t magfilter_mode = fileBuffReadFxU32(infile);

    grTexFilterMode(tmu, minfilter_mode, magfilter_mode);

    if (ScreenEcho)
        printf("grTexFilterMode(%d,%d,%d)\n",(int)tmu, (int)minfilter_mode, (int)magfilter_mode);
}

void play_grTexLodBiasValue(FILEBUFF *infile)
{
    GrChipID_t tmu = fileBuffReadFxU32(infile);
    float bias     = fileBuffReadFloat(infile);

    grTexLodBiasValue(tmu, bias);

    if (ScreenEcho)
        printf("grTexLodBiasValue(%d,%f)\n",(int)tmu, bias);
}

void play_grTexMaxAddress(FILEBUFF *infile)
{
    GrChipID_t tmu = fileBuffReadFxU32(infile);
    FxU32 res = grTexMaxAddress(tmu);

    if (ScreenEcho)
        printf("grTexMaxAddress(%d)\tReturned:%d\n",(int)tmu,(int)res);
}

void play_grTexMinAddress(FILEBUFF *infile)
{
    GrChipID_t tmu = fileBuffReadFxU32(infile);
    FxU32 res = grTexMinAddress(tmu);

    if (ScreenEcho)
        printf("grTexMinAddress(%d)\tReturned: %d\n",(int)tmu,(int)res);
}

void play_grTexMipMapMode(FILEBUFF *infile)
{
    GrChipID_t tmu      = fileBuffReadFxU32(infile);
    GrMipMapMode_t mode = fileBuffReadFxU32(infile);
    FxBool lodBlend     = fileBuffReadFxU32(infile);

    grTexMipMapMode(tmu, mode, lodBlend);

    if (ScreenEcho)
        printf("grTexMipMapMode(%d,%d,%d)\n", (int)tmu, (int)mode, (int)lodBlend);
}

void play_grTexMultibase(FILEBUFF *infile)
{
    GrChipID_t tmu = fileBuffReadFxU32(infile);
    FxBool enable  = fileBuffReadFxU32(infile);

    grTexMultibase(tmu, enable);

    if (ScreenEcho)
        printf("grTexMultibase(%d,%d)\n",(int)tmu,(int)enable);
}

void play_grTexMultibaseAddress(FILEBUFF *infile)
{
    GrChipID_t tmu         = fileBuffReadFxU32(infile);
    GrTexBaseRange_t range = fileBuffReadFxU32(infile);
    FxU32 startAddress     = fileBuffReadFxU32(infile);
    FxU32 evenOdd          = fileBuffReadFxU32(infile);
    GrTexInfo info;
    ReadGrTexInfo(infile, &info);

    grTexMultibaseAddress(tmu, range, startAddress, evenOdd, &info);

    if (ScreenEcho)
        printf("grTexMultibaseAddress()\n");
}

void play_grTexNCCTable(FILEBUFF *infile)
{
    GrNCCTable_t table = fileBuffReadFxU32(infile);

    grTexNCCTable(table);

    if (ScreenEcho)
        printf("grTexNCCTable(%d)\n",(int)table);
}

void play_grTexSource(FILEBUFF *infile)
{
    GrChipID_t tmu     = fileBuffReadFxU32(infile);
    FxU32 startAddress = fileBuffReadFxU32(infile);
    FxU32 evenOdd      = fileBuffReadFxU32(infile);
    GrTexInfo info;        
    ReadGrTexInfo(infile, &info);
    info.data=NULL;
    
    grTexSource(tmu, startAddress, evenOdd, &info);    
    
    if (ScreenEcho)
        printf("grTexSource(%d,%d,%d)\n",(int)tmu,(int)startAddress,(int)evenOdd);
}

void play_grTexTextureMemRequired(FILEBUFF *infile)
{
    FxU32 size;
    FxU32 evenOdd = fileBuffReadFxU32(infile);
    GrTexInfo info;
    ReadGrTexInfo(infile, &info);
    
    size=grTexTextureMemRequired(evenOdd, &info);

    if (ScreenEcho)
        printf("grTexTextureMemRequired(%d)\tReturned: %d\n",(int)evenOdd,(int)size);
}

void play_grVertexLayout(FILEBUFF *infile)
{
    FxU32 param  = fileBuffReadFxU32(infile);
    FxI32 offset = fileBuffReadFxU32(infile);
    FxU32 mode   = fileBuffReadFxU32(infile);

    my_grVertexLayout(param, offset, mode);
    grVertexLayout(param, offset, mode);

    if (ScreenEcho)
        printf("grVertexLayout(%d,%d,%d)\n",(int)param,(int)offset,(int)mode);
}

void play_grViewport(FILEBUFF *infile)
{
    FxI32 x      = fileBuffReadFxU32(infile);
    FxI32 y      = fileBuffReadFxU32(infile);
    FxI32 width  = fileBuffReadFxU32(infile);
    FxI32 height = fileBuffReadFxU32(infile);

    grViewport(x, y, width, height);

    if (ScreenEcho)
        printf("grViewport()\n");
}

void play_gu3dfGetInfo(FILEBUFF *infile)
{
    int size = fileBuffReadFxU32(infile);
    char filename[128];
    Gu3dfInfo info;
    
    fileBuffReadFxU8Array(infile, filename, size);
    filename[size]=0;
    gu3dfGetInfo(filename, &info);

    if (ScreenEcho)
        printf("gu3dfGetInfo(%s)\n",filename);
}

void play_gu3dfLoad(FILEBUFF *infile)
{
    int size = fileBuffReadFxU32(infile);
    char filename[128];
    
    fileBuffReadFxU8Array(infile, filename, size);
    filename[size]=0;

    /* gu3dfLoad(*filename, *data); */

    if (ScreenEcho)
        printf("NOT CALLED! gu3dfLoad(%s)\n",filename);
}

void play_guFogGenerateExp(FILEBUFF *infile)
{
    GrFog_t fogtable[BIG_NUM];
    float density = fileBuffReadFloat(infile);

    guFogGenerateExp(fogtable, density);

    if (ScreenEcho)
        printf("guFogGenerateExp(%f)\n",density);
}

void play_guFogGenerateExp2(FILEBUFF *infile)
{
    GrFog_t fogtable[BIG_NUM];
    float density = fileBuffReadFloat(infile);

    guFogGenerateExp2(fogtable, density);

    if (ScreenEcho)
        printf("guFogGenerateExp2(%f)\n",density);
}

void play_guFogGenerateLinear(FILEBUFF *infile)
{
    GrFog_t fogtable[BIG_NUM];
    float nearZ = fileBuffReadFloat(infile);
    float farZ  = fileBuffReadFloat(infile);
    
    guFogGenerateLinear(fogtable, nearZ, farZ);

    if (ScreenEcho)
        printf("guFogGenerateLinear(%f,%f)\n",nearZ,farZ);
}

void play_guFogTableIndexToW(FILEBUFF *infile)
{
    int i = fileBuffReadFxU32(infile);
    float result = guFogTableIndexToW(i);

    if (ScreenEcho)
        printf("guFogTableIndexToW(%u)\tReturned:%f\n",i,result);
}

void play_guGammaCorrectionRGB(FILEBUFF *infile)
{
    FxFloat red   = fileBuffReadFloat(infile);
    FxFloat green = fileBuffReadFloat(infile);
    FxFloat blue  = fileBuffReadFloat(infile);

    guGammaCorrectionRGB(red, green, blue);

    if (ScreenEcho)
        printf("guGammaCorrectionRGB(%f,%f,%f)\n",red,green,blue);
}








void filter_grChromaRangeModeExt(int token, FILEBUFF *infile)
{
    grChromaRangeModeExtState *state = &gstate.grCRME;

    state->set = FXTRUE;
    state->mode = fileBuffReadFxU32(infile);
}

void filter_grChromaRangeExt(int token, FILEBUFF *infile)
{
    grChromaRangeExtState *state = &gstate.grCRE;

    state->set = FXTRUE;
    state->color      = fileBuffReadFxU32(infile);
    state->range      = fileBuffReadFxU32(infile);
    state->match_mode = fileBuffReadFxU32(infile);
}

void filter_grTexChromaModeExt(int token, FILEBUFF *infile)
{
    GrChipID_t tmu = fileBuffReadFxU32(infile);
    grTexChromaModeExtState *state;

    state = gstate.grTCME + tmu;
    state->set = FXTRUE;
    state->mode = fileBuffReadFxU32(infile);
}

void filter_grTexChromaRangeExt(int token, FILEBUFF *infile)
{
    GrChipID_t tmu = fileBuffReadFxU32(infile);
    grTexChromaRangeExtState *state;

    state = gstate.grTCRE + tmu;
    state->set = FXTRUE;
    state->min  = fileBuffReadFxU32(infile);
    state->max  = fileBuffReadFxU32(infile);
    state->mode = fileBuffReadFxU32(infile);
}

void filter_grDrawTextureLineExt(int token, FILEBUFF *infile)
{
    my_inputvnalloc(&vtxa,infile);
    my_inputvnalloc(&vtxb,infile);

    return;  // don't need to save any state
}

void filter_grAADrawTriangle(int token, FILEBUFF *infile)
{
    FxBool ab_antialias;
    FxBool bc_antialias;
    FxBool ca_antialias;

    my_inputvnalloc(&vtxa[0],infile);
    my_inputvnalloc(&vtxb[0],infile);
    my_inputvnalloc(&vtxc[0],infile);

    ab_antialias = fileBuffReadFxU32(infile);
    bc_antialias = fileBuffReadFxU32(infile);
    ca_antialias = fileBuffReadFxU32(infile);

    return;  // don't need to save any state
}

void filter_grAlphaBlendFunction(int token, FILEBUFF *infile)
{
    grAlphaBlendFunctionState *state = &gstate.grABF;

    state->set = FXTRUE;
    state->rgb_sf   = fileBuffReadFxU32(infile);
    state->rgb_df   = fileBuffReadFxU32(infile);
    state->alpha_sf = fileBuffReadFxU32(infile);
    state->alpha_df = fileBuffReadFxU32(infile);
}

void filter_grAlphaCombine(int token, FILEBUFF *infile)
{
    grAlphaCombineState *state = &gstate.grAC;

    state->set = FXTRUE;
    state->function = fileBuffReadFxU32(infile);
    state->factor   = fileBuffReadFxU32(infile);
    state->local    = fileBuffReadFxU32(infile);
    state->other    = fileBuffReadFxU32(infile);
    state->invert   = fileBuffReadFxU32(infile);
}

void filter_grAlphaControlsITRGBLighting(int token, FILEBUFF *infile)
{
    grAlphaControlsITRGBLightingState *state = &gstate.grACIL;

    state->set = FXTRUE;
    state->enable = fileBuffReadFxU32(infile);
}

void filter_grAlphaTestFunction(int token, FILEBUFF *infile)
{
    grAlphaTestFunctionState *state = &gstate.grATF;

    state->set = FXTRUE;
    state->function = fileBuffReadFxU32(infile);
}

void filter_grAlphaTestReferenceValue(int token, FILEBUFF *infile)
{
    grAlphaTestReferenceValueState *state = &gstate.grATRV;

    state->set = FXTRUE;
    state->value = fileBuffReadFxU8(infile);
}

void filter_grBufferClear(int token, FILEBUFF *infile)
{
    grBufferClearState *state = &gstate.grBC;

    state->set = FXTRUE;
    state->color = fileBuffReadFxU32(infile);
    state->alpha = fileBuffReadFxU8 (infile);
    state->depth = fileBuffReadFxU32(infile);
}

void filter_grBufferSwap(int token, FILEBUFF *infile)
{
    grBufferSwapState *state = &gstate.grBS;

    state->set = FXTRUE;
    state->swap_interval = fileBuffReadFxU32(infile);
}

void filter_grChromakeyMode(int token, FILEBUFF *infile)
{
    grChromakeyModeState *state = &gstate.grCKM;

    state->set = FXTRUE;
    state->mode = fileBuffReadFxU32(infile);
}

void filter_grChromakeyValue(int token, FILEBUFF *infile)
{
    grChromakeyValueState *state = &gstate.grCKV;

    state->set = FXTRUE;
    state->value = fileBuffReadFxU32(infile);
}

void filter_grClipWindow(int token, FILEBUFF *infile)
{
    grClipWindowState *state = &gstate.grCW;

    state->set = FXTRUE;
    state->minx = fileBuffReadFxU32(infile);
    state->miny = fileBuffReadFxU32(infile);
    state->maxx = fileBuffReadFxU32(infile);
    state->maxy = fileBuffReadFxU32(infile);
}

void filter_grColorCombine(int token, FILEBUFF *infile)
{
    grColorCombineState *state = &gstate.grCC;

    state->set = FXTRUE;
    state->function = fileBuffReadFxU32(infile);
    state->factor   = fileBuffReadFxU32(infile);
    state->local    = fileBuffReadFxU32(infile);
    state->other    = fileBuffReadFxU32(infile);
    state->invert   = fileBuffReadFxU32(infile);
}

void filter_grColorMask(int token, FILEBUFF *infile)
{
    grColorMaskState *state = &gstate.grColorMask;

    state->set = FXTRUE;
    state->rgb = fileBuffReadFxU32(infile);
    state->a   = fileBuffReadFxU32(infile);
}

void filter_grConstantColorValue(int token, FILEBUFF *infile)
{
    grConstantColorValueState *state = &gstate.grCCV;

    state->set = FXTRUE;
    state->value = fileBuffReadFxU32(infile);
}

void filter_grCoordinateSpace(int token, FILEBUFF *infile)
{
    grCoordinateSpaceState *state = &gstate.grCS;

    state->set = FXTRUE;
    state->mode = fileBuffReadFxU32(infile);
}

void filter_grCullMode(int token, FILEBUFF *infile)
{
    grCullModeState *state = &gstate.grCullMode;

    state->set = FXTRUE;
    state->mode = fileBuffReadFxU32(infile);
}

void filter_grDepthBiasLevel(int token, FILEBUFF *infile)
{
    grDepthBiasLevelState *state = &gstate.grDBL;

    state->set = FXTRUE;
    state->level = fileBuffReadFxU32(infile);
}

void filter_grDepthBufferFunction(int token, FILEBUFF *infile)
{
    grDepthBufferFunctionState *state = &gstate.grDBF;

    state->set = FXTRUE;
    state->function = fileBuffReadFxU32(infile);
}

void filter_grDepthBufferMode(int token, FILEBUFF *infile)
{
    grDepthBufferModeState *state = &gstate.grDBM;

    state->set = FXTRUE;
    state->mode = fileBuffReadFxU32(infile);
}

void filter_grDepthMask(int token, FILEBUFF *infile)
{
    grDepthMaskState *state = &gstate.grDepthMask;

    state->set = FXTRUE;
    state->mask = fileBuffReadFxU32(infile);
}

void filter_grDepthRange(int token, FILEBUFF *infile)
{
    grDepthRangeState *state = &gstate.grDR;

    state->set = FXTRUE;
    state->n = fileBuffReadFloat(infile);
    state->f = fileBuffReadFloat(infile);
}

void filter_grDisable(int token, FILEBUFF *infile)
{
    // same code as grEnable, except enable gets set to FXFALSE
    grEnableDisableState *state = &gstate.grED;
    GrEnableMode_t mode = fileBuffReadFxU32(infile);

    if (mode >= MAX_ENABLE_DISABLE) {
      printf("MAX_ENABLE_DISABLE too small!  Ack... barf...\n"); exit(0); }

    state->set[mode]    = FXTRUE;
    state->enable[mode] = FXFALSE;
}

void filter_grDisableAllEffects(int token, FILEBUFF *infile)
{
    grDisableAllEffectsState *state = &gstate.grDAE;

    state->set = FXTRUE;
}

void filter_grDitherMode(int token, FILEBUFF *infile)
{
    grDitherModeState *state = &gstate.grDitherMode;

    state->set = FXTRUE;
    state->mode = fileBuffReadFxU32(infile);
}

void filter_grDrawLine(int token, FILEBUFF *infile)
{
    my_inputvnalloc(&vtxa[0],infile);
    my_inputvnalloc(&vtxb[0],infile);
    // don't need to save any state
}

void filter_grDrawPoint(int token, FILEBUFF *infile)
{
    my_inputvnalloc(&vtxa[0],infile);
    // don't need to save any state
}

void filter_grDrawTriangle(int token, FILEBUFF *infile)
{
    my_inputvnalloc(&vtxa[0],infile);
    my_inputvnalloc(&vtxb[0],infile);
    my_inputvnalloc(&vtxc[0],infile);
    // don't need to save any state
}

void filter_grDrawVertexArray(int token, FILEBUFF *infile)
{
    FxU32 i;
    FxU32 Count;
    void **pointers;

    (void) fileBuffReadFxU32(infile);  // mode
    Count = fileBuffReadFxU32(infile);

    pointers = malloc(sizeof(void*)*Count);

    for (i=0; i<Count; i++) {
      pointers[i]=my_inputvertex(infile);
      free(pointers[i]);
    }
    free(pointers);
    // don't need to save any state
}

void filter_grDrawVertexArrayContiguous(int token, FILEBUFF *infile)
{
    FxU32 Count, stride;
    void *pointers;

    (void) fileBuffReadFxU32(infile); // mode
    Count  = fileBuffReadFxU32(infile);
    stride = fileBuffReadFxU32(infile);
    pointers = malloc(stride*Count);

    // Since we don't care about the data, we don't need to worry
    // about endianness issues, hence we just read the bytes in
    // and chuck them.
    fileBuffReadFxU8Array(infile, pointers, stride*Count);

    free(pointers);
    // don't need to save any state
}

void filter_grEnable(int token, FILEBUFF *infile)
{
    // same code as grDisable, except enable gets set to FXTRUE
    grEnableDisableState *state = &gstate.grED;
    GrEnableMode_t mode = fileBuffReadFxU32(infile);

    if (mode >= MAX_ENABLE_DISABLE) {
      printf("MAX_ENABLE_DISABLE too small!  Ack... barf...\n"); exit(0); }

    state->set[mode]    = FXTRUE;
    state->enable[mode] = FXTRUE;
}

void filter_grErrorSetCallback(int token, FILEBUFF *infile)
{
/*
    GrErrorCallbackFnc_t fnc;
    fileBuffRead(infile, &fnc, sizeof(GrErrorCallbackFnc_t));
    grErrorSetCallback(fnc);
*/
}

void filter_grFinish(int token, FILEBUFF *infile)
{
    // don't need to save any state
}

void filter_grFlush(int token, FILEBUFF *infile)
{
    // don't need to save any state
}

void filter_grFogColorValue(int token, FILEBUFF *infile)
{
    grFogColorValueState *state = &gstate.grFCV;

    state->set = FXTRUE;
    state->fogcolor = fileBuffReadFxU32(infile);
}

void filter_grFogMode(int token, FILEBUFF *infile)
{
    grFogModeState *state = &gstate.grFM;

    state->set = FXTRUE;
    state->mode = fileBuffReadFxU32(infile);
}

void filter_grFogTable(int token, FILEBUFF *infile)
{
    grFogTableState *state = &gstate.grFT;

    state->set = FXTRUE;
    state->tsize = fileBuffReadFxU32(infile);
    if (state->tsize >= MAX_FOG_TABLE_SIZE) {
      printf("MAX_FOG_TABLE_SIZE too small!  Ack... barf...\n"); exit(0); }
    fileBuffReadFxU8Array(infile, (FxU8*)&state->data, state->tsize);
}

void filter_grGet(int token, FILEBUFF *infile)
{
    FxU32 pname   = fileBuffReadFxU32(infile);
    (void) fileBuffReadFxU32(infile);  // plength

    if (pname==GR_GLIDE_STATE_SIZE) {
        grGet(GR_GLIDE_STATE_SIZE,4,&state_size);        
    } else if (pname==GR_GLIDE_VERTEXLAYOUT_SIZE) {
        grGet(GR_GLIDE_STATE_SIZE,4,&vlayout_size);        
    }
    // don't need to save any state
}

void filter_grGetProcAddress(int token, FILEBUFF *infile)
{
    int size = fileBuffReadFxU32(infile);
    char *procName;

    procName=(char *) malloc(sizeof(char)*(size+2));
    fileBuffReadFxU8Array(infile, procName, size);
    free(procName);
    // don't need to save any state
}

void filter_grGetString(int token, FILEBUFF *infile)
{
    (void) fileBuffReadFxU32(infile);  // pname
    // don't need to save any state
}

void filter_grLfbConstantAlpha(int token, FILEBUFF *infile)
{
    grLfbConstantAlphaState *state = &gstate.grLCA;

    state->set = FXTRUE;
    state->alpha = fileBuffReadFxU8(infile);
}

void filter_grLfbConstantDepth(int token, FILEBUFF *infile)
{
    grLfbConstantDepthState *state = &gstate.grLCD;

    state->set = FXTRUE;
    state->depth = fileBuffReadFxU32(infile);
}

void filter_grLfbLock(int token, FILEBUFF *infile)
{
    (void) fileBuffReadFxU32(infile);  // type
    (void) fileBuffReadFxU32(infile);  // buffer
    (void) fileBuffReadFxU32(infile);  // writeMode
    (void) fileBuffReadFxU32(infile);  // origin
    (void) fileBuffReadFxU32(infile);  // pixelPipeline
    (void) fileBuffReadFxU32(infile);  // result

    // don't need to save any state
}

void filter_grLfbReadRegion(int token, FILEBUFF *infile)
{
    (void) fileBuffReadFxU32(infile);  // src_buffer
    (void) fileBuffReadFxU32(infile);  // src_x
    (void) fileBuffReadFxU32(infile);  // src_y
    (void) fileBuffReadFxU32(infile);  // src_width
    (void) fileBuffReadFxU32(infile);  // src_height
    (void) fileBuffReadFxU32(infile);  // dst_stride

    // don't need to save any state
}

void filter_grLfbUnlock(int token, FILEBUFF *infile)
{
    (void) fileBuffReadFxU32(infile);  // type
    (void) fileBuffReadFxU32(infile);  // buffer

    // don't need to save any state
}

void filter_grLfbWriteColorFormat(int token, FILEBUFF *infile)
{
    grLfbWriteColorFormatState *state = &gstate.grLWCF;

    state->set = FXTRUE;
    state->colorFormat = fileBuffReadFxU32(infile);
}

void filter_grLfbWriteColorSwizzle(int token, FILEBUFF *infile)
{
    grLfbWriteColorSwizzleState *state = &gstate.grLWCS;

    state->set = FXTRUE;
    state->swizzleBytes = fileBuffReadFxU32(infile);
    state->swapWords    = fileBuffReadFxU32(infile);
}

void filter_grLfbWriteRegion(int token, FILEBUFF *infile)
{
    char filename[TEXNAMELEN+1];
    (void) fileBuffReadFxU32(infile);  // dst_buffer
    (void) fileBuffReadFxU32(infile);  // dst_x
    (void) fileBuffReadFxU32(infile);  // dst_y
    (void) fileBuffReadFxU32(infile);  // src_format
    (void) fileBuffReadFxU32(infile);  // src_width
    (void) fileBuffReadFxU32(infile);  // src_height
    (void) fileBuffReadFxU32(infile);  // pixelPipeline
    (void) fileBuffReadFxU32(infile);  // src_stride
    fileBuffReadFxU8Array(infile, filename, TEXNAMELEN);

    // don't need to save any state
}

void filter_grLoadGammaTable(int token, FILEBUFF *infile)
{
    FxU32 i;
    grLoadGammaTableState *state = &gstate.grLGT;

    state->set = FXTRUE;
    state->nentries = fileBuffReadFxU32(infile);
    if (state->nentries > MAX_GAMMA_TABLE_SIZE) {
      printf("MAX_GAMMA_TABLE_SIZE too small!  Ack... barf...\n"); exit(0); }
    for ( i=0 ; i<state->nentries ; i++ ) {
      state->data_r[i] = fileBuffReadFxU32(infile);
      state->data_g[i] = fileBuffReadFxU32(infile);
      state->data_b[i] = fileBuffReadFxU32(infile);
    }
}

void filter_grQueryResolutions(int token, FILEBUFF *infile)
{
    GlideResolution resTemplate;
    GlideResolution *output;
    FxI32 result1;

    resTemplate.resolution      = fileBuffReadFxU32(infile);
    resTemplate.refresh         = fileBuffReadFxU32(infile);
    resTemplate.numColorBuffers = fileBuffReadFxU32(infile);
    resTemplate.numAuxBuffers   = fileBuffReadFxU32(infile);
    output    = (GlideResolution*)fileBuffReadFxU32(infile);
    result1                     = fileBuffReadFxU32(infile);
    // don't need to save any state
}

void filter_grRenderBuffer(int token, FILEBUFF *infile)
{
    grRenderBufferState *state = &gstate.grRB;

    state->set = FXTRUE;
    state->buffer = fileBuffReadFxU32(infile);
}

void filter_grReset(int token, FILEBUFF *infile)
{
    grResetState *state = &gstate.grR;
    FxU32 what = fileBuffReadFxU32(infile);
    if (what >= MAX_RESET) {
      printf("MAX_RESET too small!  Ack... barf...\n"); exit(0); }
    state->set[what] = FXTRUE;
}

void filter_grSelectContext(int token, FILEBUFF *infile)
{
    // just consume the data
    (void)fileBuffReadFxU32(infile);
}

void filter_grSplash(int token, FILEBUFF *infile)
{
    (void) fileBuffReadFloat(infile);  // x
    (void) fileBuffReadFloat(infile);  // y
    (void) fileBuffReadFloat(infile);  // width
    (void) fileBuffReadFloat(infile);  // height
    (void) fileBuffReadFxU32(infile);  // frame

    // don't need to save any state
}

void filter_grSstOrigin(int token, FILEBUFF *infile)
{
    grSstOriginState *state = &gstate.grSO;

    state->set = FXTRUE;
    state->origin = fileBuffReadFxU32(infile);
}

void filter_grTexCalcMemRequired(int token, FILEBUFF *infile)
{
    (void) fileBuffReadFxU32(infile);  // lodmin
    (void) fileBuffReadFxU32(infile);  // lodmax
    (void) fileBuffReadFxU32(infile);  // aspect
    (void) fileBuffReadFxU32(infile);  // fmt
    // don't need to save any state
}

void filter_grTexClampMode(int token, FILEBUFF *infile)
{
    GrChipID_t tmu = fileBuffReadFxU32(infile);
    grTexClampModeState *state;

    state = gstate.grTCM + tmu;
    state->set = FXTRUE;

    state->s_clampmode = fileBuffReadFxU32(infile);
    state->t_clampmode = fileBuffReadFxU32(infile);
}

void filter_grTexCombine(int token, FILEBUFF *infile)
{
    GrChipID_t tmu = fileBuffReadFxU32(infile);
    grTexCombineState *state;

    state = gstate.grTC + tmu;
    state->set = FXTRUE;

    state->rgb_function   = fileBuffReadFxU32(infile);
    state->rgb_factor     = fileBuffReadFxU32(infile);
    state->alpha_function = fileBuffReadFxU32(infile);
    state->alpha_factor   = fileBuffReadFxU32(infile);
    state->rgb_invert     = fileBuffReadFxU32(infile);
    state->alpha_invert   = fileBuffReadFxU32(infile);
}

void filter_grTexDetailControl(int token, FILEBUFF *infile)
{
    GrChipID_t tmu = fileBuffReadFxU32(infile);
    grTexDetailControlState *state;

    state = gstate.grTDC + tmu;
    state->set = FXTRUE;

    state->lod_bias     = fileBuffReadFxU32(infile);
    state->detail_scale = fileBuffReadFxU8 (infile);
    state->detail_max   = fileBuffReadFloat(infile);
}

void filter_grTexDownloadMipMap(int token, FILEBUFF *infile)
{
    grTexDownloadMipMapState *state =
        (grTexDownloadMipMapState*)malloc(sizeof(grTexDownloadMipMapState));
    
    state->token        = token;
    state->tmu          = fileBuffReadFxU32(infile);
    state->startAddress = fileBuffReadFxU32(infile);
    state->evenOdd      = fileBuffReadFxU32(infile);
    ReadGrTexInfo(infile, &state->info);
    fileBuffReadFxU8Array(infile, state->filename, TEXNAMELEN);
    state->filename[TEXNAMELEN] = '\0';
    state->size = grTexTextureMemRequired(state->evenOdd, &state->info);
    AddTextureData(state->tmu, state->startAddress, state->size, state);
}

void filter_grTexDownloadMipMapLevel(int token, FILEBUFF *infile)
{
    grTexDownloadMipMapLevelState *state =
        (grTexDownloadMipMapLevelState*)malloc(sizeof(grTexDownloadMipMapLevelState));

    state->token        = token;
    state->tmu          = fileBuffReadFxU32(infile);
    state->startAddress = fileBuffReadFxU32(infile);
    state->thisLod      = fileBuffReadFxU32(infile);
    state->largeLod     = fileBuffReadFxU32(infile);
    state->aspectRatio  = fileBuffReadFxU32(infile);
    state->format       = fileBuffReadFxU32(infile);
    state->evenOdd      = fileBuffReadFxU32(infile);
    fileBuffReadFxU8Array(infile, state->filename, TEXNAMELEN);
    state->filename[TEXNAMELEN] = '\0';
    state->size = grTexCalcMemRequired(state->thisLod,
                                       state->thisLod,
                                       state->aspectRatio,
                                       state->format);
    AddTextureData(state->tmu, state->startAddress, state->size, state);
}

void filter_grTexDownloadMipMapLevelPartial(int token, FILEBUFF *infile)
{
    grTexDownloadMipMapLevelPartialState *state =
        (grTexDownloadMipMapLevelPartialState*)malloc(sizeof(grTexDownloadMipMapLevelPartialState));

    state->token = token;
    state->tmu          = fileBuffReadFxU32(infile);
    state->startAddress = fileBuffReadFxU32(infile);
    state->thisLod      = fileBuffReadFxU32(infile);
    state->largeLod     = fileBuffReadFxU32(infile);
    state->aspectRatio  = fileBuffReadFxU32(infile);
    state->format       = fileBuffReadFxU32(infile);
    state->evenOdd      = fileBuffReadFxU32(infile);
    state->start        = fileBuffReadFxU32(infile);
    state->end          = fileBuffReadFxU32(infile);
    fileBuffReadFxU8Array(infile, state->filename, TEXNAMELEN);
    state->filename[TEXNAMELEN] = '\0';
    state->size = GetMipMapSize(state->aspectRatio,
                                state->thisLod,
                                state->format);
    state->size *= (state->end - state->start + 1);
    AddTextureData(state->tmu, state->startAddress, state->size, state);
}

void filter_grTexDownloadTable(int token, FILEBUFF *infile)
{
    int i;
    grTexDownloadTableState *state = &gstate.grTDT;

    state->set = FXTRUE;
    state->type = fileBuffReadFxU32(infile);
    if ((state->type == GR_TEXTABLE_PALETTE) ||
        (state->type == GR_TEXTABLE_PALETTE_6666_EXT)) {
        for ( i=0 ; i<256 ; i++ )
          state->texpalette.data[i] = fileBuffReadFxU32(infile);
    } else {
        ReadGuNccTable(infile, &state->ncctable);
    }
}

void filter_grTexDownloadTablePartial(int token, FILEBUFF *infile)
{
    grTexDownloadTableState *state = &gstate.grTDT;
    int i, start, end;

    state->set  = FXTRUE;
    state->type = fileBuffReadFxU32(infile);
    start       = fileBuffReadFxU32(infile);
    end         = fileBuffReadFxU32(infile);

    if ((state->type == GR_TEXTABLE_PALETTE) ||
        (state->type == GR_TEXTABLE_PALETTE_6666_EXT)) {
        for ( i=start ; i<=end ; i++ )
          state->texpalette.data[i] = fileBuffReadFxU32(infile);
    } else {
        ReadGuNccTable(infile, &state->ncctable);
    }        
}

void filter_grTexFilterMode(int token, FILEBUFF *infile)
{
    GrChipID_t tmu = fileBuffReadFxU32(infile);
    grTexFilterModeState *state;

    state = gstate.grTFM + tmu;
    state->set = FXTRUE;

    state->minfilter_mode = fileBuffReadFxU32(infile);
    state->magfilter_mode = fileBuffReadFxU32(infile);
}

void filter_grTexLodBiasValue(int token, FILEBUFF *infile)
{
    GrChipID_t tmu = fileBuffReadFxU32(infile);
    grTexLodBiasValueState *state;

    state = gstate.grTLBV + tmu;
    state->set = FXTRUE;

    state->bias = fileBuffReadFloat(infile);
}

void filter_grTexMaxAddress(int token, FILEBUFF *infile)
{
    (void) fileBuffReadFxU32(infile);  // tmu
    // don't need to save any state
}

void filter_grTexMinAddress(int token, FILEBUFF *infile)
{
    (void) fileBuffReadFxU32(infile);  // tmu
    // don't need to save any state
}

void filter_grTexMipMapMode(int token, FILEBUFF *infile)
{
    GrChipID_t tmu = fileBuffReadFxU32(infile);
    grTexMipMapModeState *state;

    state = gstate.grTMMM + tmu;
    state->set = FXTRUE;

    state->mode     = fileBuffReadFxU32(infile);
    state->lodBlend = fileBuffReadFxU32(infile);
}

void filter_grTexMultibase(int token, FILEBUFF *infile)
{
    GrChipID_t tmu = fileBuffReadFxU32(infile);
    grTexMultibaseState *state;

    state = gstate.grTMB + tmu;
    state->set = FXTRUE;

    state->enable = fileBuffReadFxU32(infile);
}

void filter_grTexMultibaseAddress(int token, FILEBUFF *infile)
{
    GrChipID_t tmu = fileBuffReadFxU32(infile);
    grTexMultibaseAddressState *state;

    state = gstate.grTMBA + tmu;
    state->set = FXTRUE;

    state->range        = fileBuffReadFxU32(infile);
    state->startAddress = fileBuffReadFxU32(infile);
    state->evenOdd      = fileBuffReadFxU32(infile);
    ReadGrTexInfo(infile, &state->info);
}

void filter_grTexNCCTable(int token, FILEBUFF *infile)
{
    grTexNCCTableState *state = &gstate.grTNT;

    state->set = FXTRUE;
    state->table = fileBuffReadFxU32(infile);
}

void filter_grTexSource(int token, FILEBUFF *infile)
{
    GrChipID_t tmu = fileBuffReadFxU32(infile);
    grTexSourceState *state;

    state = gstate.grTS + tmu;
    state->set = FXTRUE;

    state->startAddress = fileBuffReadFxU32(infile);
    state->evenOdd      = fileBuffReadFxU32(infile);
    ReadGrTexInfo(infile, &state->info);
    state->info.data=NULL;
}

void filter_grTexTextureMemRequired(int token, FILEBUFF *infile)
{
    GrTexInfo info;
    (void)fileBuffReadFxU32(infile);  // evenOdd
    ReadGrTexInfo(infile, &info);
    // don't need to save any state
}

void filter_grVertexLayout(int token, FILEBUFF *infile)
{
    FxU32 param = fileBuffReadFxU32(infile);
    grVertexLayoutState *state;

    if (param >= MAX_VERTEX_LAYOUT) {
      printf("MAX_VERTEX_LAYOUT too small!  Ack... barf...\n"); exit(0); }
    state = gstate.grVL + param;
    state->set = FXTRUE;
    state->offset = fileBuffReadFxU32(infile);
    state->mode   = fileBuffReadFxU32(infile);
    my_grVertexLayout(param, state->offset, state->mode);
}

void filter_grViewport(int token, FILEBUFF *infile)
{
    grViewportState *state = &gstate.grVP;

    state->set    = FXTRUE;
    state->x      = fileBuffReadFxU32(infile);
    state->y      = fileBuffReadFxU32(infile);
    state->width  = fileBuffReadFxU32(infile);
    state->height = fileBuffReadFxU32(infile);
}

void filter_gu3dfGetInfo(int token, FILEBUFF *infile)
{
    int size = fileBuffReadFxU32(infile);
    char filename[128];

    fileBuffReadFxU8Array(infile, filename, size);
    // don't need to save any state
}

void filter_gu3dfLoad(int token, FILEBUFF *infile)
{
    int size = fileBuffReadFxU32(infile);
    char filename[128];
    
    fileBuffReadFxU8Array(infile, filename, size);
    // don't need to save any state
}

void filter_guFogGenerateExp(int token, FILEBUFF *infile)
{
    (void) fileBuffReadFloat(infile);  // density
    // don't need to save any state
}

void filter_guFogGenerateExp2(int token, FILEBUFF *infile)
{
    (void) fileBuffReadFloat(infile);  // density
    // don't need to save any state
}

void filter_guFogGenerateLinear(int token, FILEBUFF *infile)
{
    (void) fileBuffReadFloat(infile);  // nearZ
    (void) fileBuffReadFloat(infile);  // farZ
    // don't need to save any state
}

void filter_guFogTableIndexToW(int token, FILEBUFF *infile)
{
    (void) fileBuffReadFxU32(infile);  // i
    // don't need to save any state
}

void filter_guGammaCorrectionRGB(int token, FILEBUFF *infile)
{
    guGammaCorrectionRGBState *state = &gstate.guGCRGB;

    state->set   = FXTRUE;
    state->red   = fileBuffReadFloat(infile);
    state->green = fileBuffReadFloat(infile);
    state->blue  = fileBuffReadFloat(infile);
}

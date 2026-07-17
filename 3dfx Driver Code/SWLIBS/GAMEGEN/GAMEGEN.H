#define DOSPLATFORM                 1

#define RECORD_HEADER_SIZE          4

/********************************************************************/
/* New stuff for rewrite. */

#define OP_SVERTEX                  7
#define OP_VERTEX_MORPH_LIST      504
#define OP_SHARED                 516
#define OP_LOD                    522
#define OP_TMTX                   525 /* Transformation Matrix */
#define OP_POP                    526
#define OP_SUBPOP                 541
#define OP_VERTEX_LIST            548
#define OP_NORMAL                 550
#define OP_BORIENT                555
#define OP_LONG_ID                556
#define OP_BBOX                   558
#define OP_ROTATE_ABOUT_POINT     568
#define OP_TRANSLATE              569
#define OP_COLOR                  574
#define OP_LOCAL_INSTANCE         579
#define OP_SWITCH                 581
#define OP_POLYGON                583
#define OP_COORD                  597
#define OP_LIGHT_SOURCE           604
#define OP_PUSH                   609
#define OP_LOCAL_INSTANCE_LIBRARY 611
#define OP_MATERIAL               613
#define OP_HEADER                 617
#define OP_EYEPOINT               621
#define OP_REPLICATE              631
#define OP_BCENTER                633
#define OP_OBJECT                 638
#define OP_VERTEX_TEXTURE         643
#define OP_GROUP                  644
#define OP_SUBPUSH                665
#define OP_NORMAL_TEXTURE         666
#define OP_LIGHT_PALETTE          670
#define OP_DOF                    671  /* Degree of Freedom. */
#define OP_VECTOR                 683
#define OP_COMMENT                690
#define OP_TEXTURE                691

#define TYPE_HEADER                 1
#define TYPE_GROUP                  2
#define TYPE_OBJECT                 3
#define TYPE_POLYGON                4
#define TYPE_LOD                    5

typedef enum {
  SUCCESS = 0,
  FAILURE,        /* Indicates non-specific or non-determined failure. */
  BAD_CLASS,
  MEM_ALLOC,
  NULL_PTR,
  STRING_OVERFLOW,
  VERTEX_OVERFLOW,
  OPEN_ERROR,
  READ_ERROR
}GG_STATUS;

typedef enum
{
	NO_WHERE = 0,
	UP,
	DOWN,
	LEFT,
	RIGHT
}GDIRECTION;

typedef struct
{
   short    opcode;
   short    length;
}gg_record_info;

typedef struct GNodeStruct
{
	short					type;
	void					*info;
	struct GNodeStruct		*up,*down,*left,*right;
}GNode;


typedef struct
{
   double   x,y,z;
   long     byteOffset;
   float	u,v;
}G3DPoint;

typedef struct
{
   long		index[3];
   short	numVerts;
}GPolygon;

typedef struct
{
	char name[80];
}GTexture;

typedef struct GTexListStruct
{
	struct GTexListStruct     *left,*right;
	GTexture                  texture;
}GTextureList;

short          TShort( unsigned char *record_ptr );
unsigned short TUShort( unsigned char *record_ptr );
double         TDouble( unsigned char *char_ptr );
float          TFloat( unsigned char *char_ptr );
int            TInt( unsigned char *char_ptr );
unsigned int   TUInt( unsigned char *char_ptr );

GG_STATUS      gg_read_db(char *name);
GG_STATUS      gg_convert_db(void);
GG_STATUS      gg_read_opcode(FILE *theFile,gg_record_info *info);
GG_STATUS      gg_read_header(FILE *theFile,gg_record_info *info);
GG_STATUS      gg_read_shared(FILE *theFile,gg_record_info *info);
GG_STATUS      gg_read_coord(FILE *theFile,gg_record_info *info);
GG_STATUS      gg_read_norm_tex(FILE *theFile,gg_record_info *info);
GG_STATUS      gg_read_polygon(FILE *theFile,gg_record_info *info);
GG_STATUS      gg_read_vert_list(FILE *theFile,gg_record_info *info);
GG_STATUS      gg_read_object(FILE *theFile,gg_record_info *info);
GG_STATUS      gg_read_group(FILE *theFile,gg_record_info *info);
GG_STATUS      gg_read_lod(FILE *theFile,gg_record_info *info);
GG_STATUS      gg_read_unknown(FILE *theFile,gg_record_info *info);
GG_STATUS      gg_read_pop(FILE *theFile,gg_record_info *info);
GG_STATUS      gg_read_push(FILE *theFile,gg_record_info *info);
GG_STATUS      gg_read_texture(FILE *theFile,gg_record_info *info);
GG_STATUS      gg_add_node(void *info, short kind);
GG_STATUS      gg_add_top_node(void *info, short kind);
GG_STATUS      gg_add_node_down(void *info, short kind);
GG_STATUS      gg_add_node_right(void *info, short kind);
GG_STATUS      gg_add_vert_to_polygon(long vertIndex);
GG_STATUS      gg_trace_back_levels(short numLevels);
GG_STATUS      gg_organize_textures(void);
void           HandleGroupNode(void);
void           HandleObjectNode(void);
void           HandleLODNode(void);
void           HandlePolygonNode(void);
void           AddSeparator(void);
void           AddCoordinates(void);
void AddTexCoordinates(void);
FxBool         CheckIfDone(void);
GDIRECTION     GoNextNode(void);
void           AddFormatSpaces(void);
void           AddIndexedFaceSet(void);
void           AddFaceInfo(void);
void           CloseIndexedFaceSet(void);
FxBool         FirstTimeNode(void);
FxBool         NodeDone(void);
void           CloseSeparator(void);
GG_STATUS AddToTexCoordBuffer(char *theString);
GG_STATUS FreeTexCoordBuffer();


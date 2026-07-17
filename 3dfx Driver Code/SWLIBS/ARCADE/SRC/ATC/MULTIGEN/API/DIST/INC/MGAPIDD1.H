/*******************************************************************************
 * 
 * $Header: mgapidd1.h, 4, 10/11/00 7:32:15 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:32:15 PM$
 *
 ******************************************************************************/

/*============================================================================*\

   PROPRIETARY RIGHTS NOTICE: All rights reserved.  This software contains 
   proprietary information and trade secrets of MultiGen Inc. of San Jose, 
   California, and embodies substantial creative efforts as well as 
   confidential information, ideas, and expressions.  No part or all of this 
   software may be reproduced in any form, or by any means of electronic, 
   mechanical, or otherwise, without the written permission of MultiGen Inc.

   COPYRIGHT NOTICE: Copyright (C) 1986-1996 MultiGen Inc., San Jose, California.

\*============================================================================*/

/*----------------------------------------------------------------------------*/

#ifndef MGAPIDD1_H_
#define MGAPIDD1_H_

/*----------------------------------------------------------------------------*/

#include "stdio.h"
#include "mgapistd.h"
#include "mgdddecl.h"

/*----------------------------------------------------------------------------*/

typedef struct record_def record_def;		/* record data-dictionary */
typedef struct data_def data_def;			/* data data-dictionary */
typedef struct attr_def attr_def;			/* record attribute definition */
typedef struct parent_def parent_def;		/* parent table definition */
typedef struct child_def child_def;			/* child table definition */
typedef struct alias_def alias_def;			/* alias table definition */

typedef struct parent_def_str parent_def_str; 
typedef struct child_def_str child_def_str;
typedef struct alias_def_str alias_def_str;

typedef struct runtime_rec_def runtime_rec_def;
typedef struct runtime_data_def runtime_data_def;
typedef struct runtime_com_def runtime_com_def;

#define SITEIDLEN 8
#define SITEDLLLEN 32

typedef mgbool (* GetMaxIdFunc) (mgrec* toprec, mgcode code, int *no);
typedef mgbool (* SetMaxIdFunc) (mgrec* toprec, mgcode code, int no);
typedef mgbool (* GetHeaderCodeFunc) (mgrec* toprec, mgcode *code);

typedef struct siteinfo {
	char id[ SITEIDLEN ];	/* ascii site id */
	char dll[ SITEDLLLEN ];	/* ascii DLL file name */
	int siteno;					/* runtime site number */
	int ioffset;				/* unique ifmt level offset */
	int soffset;				/* unique site offset */
	record_def *rtbl;			/* record dd table */
	int rsize;					/* # of elements in rtbl and rcodemap */
	mgcode minrcode;			/* record code begins from... */ 
	mgcode maxrcode;			/* record code ends at... */ 
	int roffset;				/* unique rcode offset */
	mgcode* rcodemap;			/* mapping from enums to mgcodes */
	data_def *dtbl;			/* data dd table */
	int dsize;					/* # of elements in dtbl and dcodemap */
	mgcode mindcode;			/* data code begins from... */ 
	mgcode maxdcode;			/* data code ends at... */ 
	int doffset;				/* unique dcode offset */
	mgcode* dcodemap;			/* mapping from enums mgcodes */
	FILE *msgfd;				/* file pointer */
	char msgfn[80];			/* site error message file name */
	int msgmin;					/* site min error message no. */
	int msgmax;					/* site max error message no. */
	int *msgindex;				/* array of error message #'s */
	int *msgfilepos;			/* array of error message file positions
								   one for each message no. */
	GetMaxIdFunc GetMaxId;
	SetMaxIdFunc SetMaxId;
	GetHeaderCodeFunc GetHeaderCode;

} siteinfo;

typedef enum action_type {
	action_get = 0, 
	action_set, 
	action_print
} action_type;

typedef enum struct_type {
	dc = 0,				/* don't care */
	pointer = 1,		/* a pointer */
	structure = 2		/* a structure */
} struct_type;

typedef enum code_type {
	is_bad = 0,
	is_record,
	is_data
} code_type;

/*----------------------------------------------------------------------------*/

#define LOGICHDR \
	mgcode code;					/* record/field code */ \
	tagtype type;					/* the data type, INT, FLOAT, etc. */ \
	unsigned key: 2;				/* 0-public, 1-super user, 2-internal, 3-extension */ \
	unsigned display: 1;			/* displayable */ \
	unsigned edit: 1;				/* editable */ \
	unsigned search: 1;			/* searchable */ \
	unsigned packed: 1;			/* packed format */ \
	unsigned flt: 1;				/* a basic flt field/record */ \
	unsigned spare1: 1;			/* spares */ \
	char *siteid;					/* site id (extension of) */ \
	short verno;					/* revision number */ \
	mgcode xcode;					/* parent extension code (extension of) */ \
	char *xstring;					/* xcode string (extension of) from user input */ \
	int acnt;						/* if > 1, number of elements in an array */ \
	int len;							/* record/field length in byte except flags in bits */ \
	char *id;						/* official field/record name */ \
	char *label;					/* widget name */ \
	char *msg;						/* message description */ \
	char *comment;					/* line comment in dd */ \
	mgbool (* cnv) (action_type type, mgrec* rec_in, void* ival);		/* conversion function */ \
	mgbool (* pre) (action_type type, mgrec* par_rec, mgcode code, void *val);		/* pre-action function */ \
	mgbool (* act) (action_type type, mgrec* par_rec, mgrec* rec);		/* post-action function */ \
	void *path;						/* dd parent path */ \
	void *spare2;					/* spares */ \
	struct runtime_com_def *rtcom;	/* collection of all runtime info for common block */ \


struct data_def {		/* data data-dictionary */
	LOGICHDR
	float minval;					/* minimum value allowed */
	float maxval;					/* maxium value allowed */
	float defval;					/* default value */
	char *defstr;					/* default text value */
	unsigned def_set :1;			/* defval is set */
	unsigned limit_set :1;			/* maxval and minval are set */
	unsigned enum_set: 1;			/* enum flags (temp...) */
	unsigned spare3: 13;
	short spare4;
	struct runtime_data_def *rtdata;/* collection of all runtime info for data */
};

struct record_def {		/* record data-dictionary */
	LOGICHDR
	short ilev;						/* ifmt level; may be undefined */
	char spare5;
	char spare6;
	unsigned char nattr;			/* total no. of attributes */
	unsigned char nparent;			/* legal parent table size */
	unsigned char nchild;			/* legal child table size */
	unsigned char nalias;			/* union table size */
	struct attr_def* attr;			/* ptr to attribute table */
	struct parent_def_str* parent;/* ptr to legal parent table; null means all legal */
	struct child_def_str* child;	/* ptr to legal child table; null means all legal */
	struct alias_def_str* alias;	/* ptr to union table for recursive searching */
	void *xlist;						/* extension list */
	char *preid;						/* prefix for next id auto assignment */
	int maxid;							/* maxium current id assignment */
	struct runtime_rec_def *rtrec;	/* collection of all runtime info for record */
};

struct attr_def {		/* record attribute definition */
	mgcode code;				/* field record code */
	unsigned bit: 6;			/* bit offset from beginning of the full word */
	struct_type ptr: 10;		/* whether this is a real structure or a pointer */
	unsigned int delta;		/* byte offset from beginning of the attribute block */
};

struct parent_def_str {		/* record parent definition in string */
	char *name;
};

struct child_def_str {		/* record child definition in string */
	char *name;
};

struct alias_def_str {		/* record union definition in string */
	char *name;
};

struct parent_def {		/* record parent definition */
	mgcode code;
};

struct child_def {		/* record child definition */
	mgcode code;
};

struct alias_def {		/* record union definition */
	mgcode code;
};

/*----------------------------------------------------------------------------*/

#define MAXSITENO 10

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

extern siteinfo*		SiteList [ MAXSITENO ];
extern int				SiteCount;

/*============================================================================*/
/* ddInit API */

/* This is the signature and name of the exported mgInitDD procedure. */
typedef void (*PROC_mgInitDD)(siteinfo *);
#define NAME_mgInitDD "mgInitDD"

/* This is the signature and name of the exported mgInitAction procedure. */
typedef void (*PROC_mgInitAction)(siteinfo *);
#define NAME_mgInitAction "mgInitAction"

DDFUNC(mgbool)			ddInit ( const char* dllFileName, int* fillInSiteno );
							/* ddInit loads in the STATICALLY COMPILED schema from the
							given DLL. */


/*============================================================================*/
/* Site API */

DDFUNC(mgbool)			ddSiteInfo(int siteno, mgcode* minrcode, mgcode* maxrcode,
										  mgcode* mindcode, mgcode* maxdcode);
							/* ddSiteInfo retrieves information about the given site. */

DDFUNC(siteinfo)*		ddSiteNo2SiteInfo ( int siteno );	/* return site info block from site id */

DDFUNC(int)				ddSiteCount();
							/* ddSiteCount returns the number of sites.  Valid siteno
							values run from 0..ddSiteCount()-1. */

DDFUNC(record_def)*	ddSiteGetNthRSchema ( int siteno, int nth );
							/* given site number, find and return nth record schema */

DDFUNC(data_def)*		ddSiteGetNthDSchema ( int siteno, int nth );
							/* given site number, find and return nth data schema */

DDFUNC(mgcode)			ddSiteGetNthRcode ( int siteno, int nth );
							/* given site number, find and return nth record code */

DDFUNC(mgcode)			ddSiteGetNthDcode ( int siteno, int nth );
							/* given site number, find and return nth data code */

/*============================================================================*/
/* General API */

DDFUNC(mgcode)			ddNameToCode(const char* name);
							/* Given a name, ddNameToCode tries to find the corresponding
							code.  If it succeeds it returns the code; otherwise it
							returns 0, which is never a valid mgcode. */

DDFUNC(mgcode)			ddLabelToCode(const char* label);

DDFUNC(mgcode)			ddIlevel2Code ( int ilev );		/* find a record code based on i-fmt level */

DDFUNC(mgbool)			ddIsRcode ( mgcode code );
DDFUNC(mgbool)			ddIsDcode ( mgcode code );
							/* ddIsRcode returns TRUE if the given code is a record_def.
							ddIsDcode returns TRUE if the given code is a data_def. */

DDFUNC(record_def)*	ddGetRSchema ( mgcode code );
DDFUNC(data_def)*		ddGetDSchema ( mgcode code );
DDFUNC(data_def)*		ddGetSchema ( mgcode code );		/* always cast back as data_def* */
							/* ddGetRSchema returns the record_def for a given code,
							or 0 if the code is invalid. ddGetDSchema does the same
							but returns a data_def. ddGetSchema returns the data_def
							or record_def for a given code, but casts it as a data_def*. */

DDFUNC(attr_def)*		ddGetFieldSchema ( mgcode rcode, mgcode fcode );
DDFUNC(mgbool)			ddGetFieldOffset ( mgcode rcode, mgcode fcode, int* delta, int* bit, int* ptr );
DDFUNC(mgcode)			ddGetFieldNth ( mgcode rcode, unsigned nth );
							/* ddGetFieldSchema returns the attr_def belonging to
							the given record.  ddGetFieldOffset returns the delta,
							bit, and ptr fiels of an attr_def associated with a given
							record.  ddGetFieldNth returns the nth attr_def associated
							with a given record, where 'nth' runs from 0 to nattr-1.
							ddGetFieldCount returns the number of attr_defs associated
							with a record. */

DDFUNC(mgbool)			ddIsField ( mgcode rcode, mgcode fcode );
							/* return true if fcode is a legal field of rcode */
DDFUNC(mgbool) 
ddIsFieldFollowInLine ( mgcode rcode, mgcode fcode );
/* return true if fcode is a legal field of rcode; follow in-line record during search */

DDFUNC(mgbool) 
ddIsFieldFollowPointer ( mgcode rcode, mgcode fcode );
/* return true if fcode is a legal field of rcode; follow pointer record during search */

DDFUNC(mgbool) 
ddIsFieldFollowAll ( mgcode rcode, mgcode fcode );
/* return true if fcode is a legal field of rcode; follow pointer/structure record during search */

DDFUNC(int) ddBuildPath ( mgcode rcode, mgcode fcode );
/* given a field code and a parent, find and build the code hierarchy chain;
	store the list in the dd and return number of element on the list. */

DDFUNC(mgbool) ddGetNthParent ( mgcode rcode, mgcode fcode, int nth, mgcode *code, mgbool *xflag );
/* find the nth parent of fcode's parent list */

/*============================================================================*/
/* Record/Data Common API */

DDFUNC(tagtype)		ddGetTagType ( mgcode code );	/* either record or data */
DDFUNC(int)				ddGetKey ( mgcode code );	/* either record or data */
DDFUNC(short)			ddGetVersion ( mgcode code );	/* either record or data */
DDFUNC(mgbool)			ddIsDisplay ( mgcode code );	/* either record or data */
DDFUNC(mgbool)			ddIsEdit ( mgcode code );	/* either record or data */
DDFUNC(mgbool)			ddIsSearch ( mgcode code );	/* either record or data */
DDFUNC(char)*			ddGetName ( mgcode code );	/* either record or data */
DDFUNC(char)*			ddGetLabel ( mgcode code );	/* either record or data */
DDFUNC(char)*			ddGetDesc ( mgcode code );	/* either record or data */
							/* These methods access the following fields in the common
							header of data_def and record_def structures: type, key, display,
							edit, search, id (ddGetName), label, msg (ddGetDesc). */

DDFUNC(int)				ddGetLength ( mgcode code );	/* return field length in byte, or negative no. in bit */
							/* ddGetLength returns the length of a record_def or, if
							given the code for a data_def, returns its length.  In the
							latter case if the data_def occupies some number of bits
							(rather than bytes) the number of bits is returned as a
							negative number. */

DDFUNC(int)				ddGetArrayCount ( mgcode code );
							/* ddGetArrayCount returns the number of entries in this
							record or data schema; this will be 1 for non-arrays, or
							>1 for arrays. */

/*============================================================================*/
/* mgcode and Site API */

DDFUNC(mgcode)			ddGetMyCode ( mgcode code );	/* get enum code */
DDFUNC(mgcode)			ddGetExtendCode ( mgcode code );	/* get extension code */
DDFUNC(mgcode)			ddGetUCode ( char* siteid, mgcode code0 );	/* get unique code from mycode + siteid */
DDFUNC(int)				ddGetSiteNo ( mgcode code );	/* parse and return site number */
DDFUNC(char) *			ddGetSiteId ( mgcode code );	/* find site id string from code */
DDFUNC(mgbool)			ddIsFlt ( mgcode code );		/* check if this is a flt code */

DDFUNC(mgbool)			ddIsLegalXcode ( mgcode parent_code, mgcode code );	/* check if code is a legal extension of parent_code */

/* mgcode values are unique.  Each site is allocated a range of SITEOFFSET
values.  The routine ddGetMyCode returns the code value within a site.  These
values may not be unique between sites.  ddGetUCode is the inverse of ddGetMyCode;
given a code value within a site, and a site ID, it returns the unique mgcode.
It returns 'code0' if the given site does not exist.  ddGetSiteNo and ddGetSideId
return the site info associated with an mgcode. ddGetExtendCode is not implemented
yet. */

/*============================================================================*/
/* Data Def API */

DDFUNC(float)			ddGetMinVal ( mgcode dcode );		/* data only */
DDFUNC(float)			ddGetMaxVal ( mgcode dcode );		/* data only */
DDFUNC(float)			ddGetDefVal ( mgcode dcode );		/* data only */
DDFUNC(char)*			ddGetDefString ( mgcode dcode );	/* data only */
DDFUNC(mgbool)			ddIsDefSet ( mgcode dcode );		/* data only */
DDFUNC(mgbool)			ddIsRangeSet ( mgcode dcode );	/* data only */
DDFUNC(mgbool)			ddIsEnumSet ( mgcode dcode );		/* data only */

/*============================================================================*/
/* Record Def API */

DDFUNC(int)				ddGetIlevel ( mgcode rcode );			/* record only */
DDFUNC(unsigned)		ddGetFieldCount ( mgcode rcode );		/* record only */
DDFUNC(attr_def)*		ddGetField ( mgcode rcode );	/* record only */
DDFUNC(unsigned)		ddGetParentCount ( mgcode rcode );	/* record only */
DDFUNC(parent_def)*		ddGetParent ( mgcode rcode );/* record only */
DDFUNC(mgcode)			ddGetParentNth ( mgcode rcode, int nth );	/* return nth attach parent's code */
DDFUNC(unsigned)		ddGetChildCount ( mgcode rcode );		/* record only */
DDFUNC(child_def)*		ddGetChild ( mgcode rcode );	/* record only */
DDFUNC(unsigned)		ddGetAliasCount ( mgcode rcode );		/* record only */
DDFUNC(alias_def)*		ddGetAlias ( mgcode rcode );	/* record only */

DDFUNC(char) *			ddGetPrefix ( mgcode rcode );	/* get record id prefix */
DDFUNC(int)				ddGetMaxId ( mgcode rcode );	/* temp... */
DDFUNC(mgbool)			ddSetMaxId ( mgcode rcode, int no );	/* temp... */

DDFUNC(void)			ddDeRcode ( mgcode code, int *index, tagtype *type, int *len, 
								char **id, int *nattr, int *ilev );
DDFUNC(void)			ddDeDcode ( mgcode code, int *index, tagtype *type, int *len, char **id );
							/* ddDeRcode and ddDeDcode retrieve information about the
							record_def or data_def indicated.  The 'index' is the index
							into the internal table. */

/*============================================================================*/
/* Attr Def API */

DDFUNC(mgbool)			ddIsPointer ( mgcode rcode, mgcode fcode );
DDFUNC(mgbool)			ddIsStructure ( mgcode rcode, mgcode fcode );
DDFUNC(mgbool)			ddIsDC ( mgcode rcode, mgcode fcode );

DDFUNC(mgbool)			ddFieldIsPointer ( attr_def* aptr );
DDFUNC(mgbool)			ddFieldIsStructure ( attr_def* aptr );
DDFUNC(mgbool)			ddFieldIsDC ( attr_def* aptr );

DDFUNC(mgcode)			ddHasExtension ( mgcode code );
							/* check all extension site's dictionary to see if anyone is tagging along;
							should return a list of code... TEMP */
DDFUNC(mgcode)			ddHasBead ( mgcode code );
							/* return true if this record has an ifmt representation */

DDFUNC(mgcode)			ddFindFieldParent ( mgcode fcode );
			/* given a field, find its direct parent record; if this field is being used by multiple
				records, only the first one found in the schema will be returned */

/*============================================================================*/

#ifdef __cplusplus
}
#endif

#endif
/* DON'T ADD STUFF AFTER THIS #endif */

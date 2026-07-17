/*******************************************************************************
 * 
 * $Header: modflt.c, 4, 10/11/00 7:33:11 PM, Brent$
 * $Revision: 4$
 * $Author: Brent$
 * $Date: 10/11/00 7:33:11 PM$
 *
 ******************************************************************************/

#include <stdio.h>		/* fgets */
#include <string.h>		/* strcmp */
#include <stdarg.h>		/* err msg */
#ifndef _WIN32
#include <unistd.h>		/* access */
#endif
#include <stdlib.h>		/* atof */
#include <ctype.h>		/* isalpha */

#include "mgapibase.h"
#include "mgapiinfo.h"
#include "mgapiio.h"
#include "mgapistruc.h"
#include "mgapiattr.h"
#include "mgapiutil.h"
#include "mgapiinfo.h"
#include "mgapimath.h"
#include "mgapidd.h"
#include "fltcode.h"

#ifdef _WIN32
#define DLL_NAME "easdata.dll"
#else
#define DLL_NAME "libeasdata.so"
#endif

/********************************************************************/

#define DELIM " \t\n,|"

typedef enum {			/* function code */
	OP_exit = 0,
	OP_mgOpenDb,
	OP_mgNewDb,
	OP_mgGetRecByName,
	OP_mgGetAtt,
	OP_mgGetNext,
	OP_mgGetPrevious,
	OP_mgGetParent,
	OP_mgGetChild,
	OP_mgGetNestedParent,
	OP_mgGetNestedChild,
	OP_mgGetReference,
	OP_mgCastRec,
	OP_mgPrintRec,
	OP_mgSetAtt,
	OP_mgCloseDb
} opfunc;

/********************************************************************/

static mgrec* Db;
static mgrec* Rec;
static mgrec* Rec1;
static mgrec Recs;

static mgcode Fcode;
static int Ival;
static float Fval;
static double Dval;
static int Walk_type;

/********************************************************************/
/********************************************************************/

static void Open ( char* fname )
{
	char *siteid, *errortxt;

#ifndef _WIN32
	if ( access ( fname, F_OK ) == -1 ) {
		Db = mgNewDb ( fname );
		if ( !Db ) {
			mgGetError ( &siteid, &errortxt );
			printf ( "%s", errortxt );
		}
		else
			Rec = Db;	/* reset */
	} else
#endif
	{
		Db = mgOpenDb ( fname );
		if ( !Db ) {
			mgGetError ( &siteid, &errortxt );
			printf ( "%s", errortxt );
		}
		else
			Rec = Db;	/* reset */
	}
}

static void GetRecByName ( mgrec* db, char* name )
{
	mgrec* rec;
	char *siteid, *errortxt;

	if ( rec = mgGetRecByName ( db, name ) )
		Rec = rec;
	else {
		mgGetError ( &siteid, &errortxt );
		printf ( "%s", errortxt );
	}
}

static void GetRec ( mgrec* rec, mgcode fcode )
{
	mgrec* rec1;
	char *siteid, *errortxt;

	if ( rec && ( rec1 = mgGetAtt ( rec, fcode ) ) ) {
		mgPrintField ( rec1 );		/* print field */
		Rec1 = rec1;
	}
	else {
		mgGetError ( &siteid, &errortxt );
		printf ( "%s", errortxt );
	}
}

static void GetRecList ( mgrec* rec, char* cmd )
{
	mgrec* rec1;
	mgrec* rec2;
	int i, cnt;
	mgcode fcode[ 10 ];
	int ival, ival2;
	short sval, sval2, sval3;
	char text[48];
	int cval;
	int flag;
	double dval0, dval1, dval2;

	fcode[0] = fltHdrFormatRev;		/* int */
	fcode[1] = fltHdrUdiv;				/* short */
	fcode[2] = fltHdrDbRev;				/* int */
	fcode[3] = fltHdrLastDate;			/* text */
	fcode[4] = fltHdrNextFacId;		/* short */
	fcode[5] = fltHdrTexWhite;			/* char val */
	fcode[6] = fltHdrFlagVtxNorms;	/* flag */
	fcode[7] = fltHdrNextFacId;		/* short */

	cnt = 8;
	mgGetAttList ( rec, fcode[0], &ival, fcode[1], &sval, fcode[2], &ival2,
	fcode[3], &text, fcode[4], &sval2, fcode[5], &cval, fcode[6], &flag, fcode[7], &sval3, mgNULL );
	printf ( "%d, %d, %d, %s, %d, %c, %d, %d\n", ival,sval,ival2,text,sval2, cval,flag,sval3);

	for ( i = 0; i < cnt; i++ ) {
		if ( rec && ( rec1 = mgGetAtt ( rec, fcode[i] ) ) )
			mgPrintField ( rec1 );		/* print field */
	}
	rec1 = mgGetRecByName ( Db, "p1/V1" );
	if ( !rec1 ) return;
	fcode[0] = fltIcoordX;		/* double */
	fcode[1] = fltIcoordY;		/* double */
	fcode[2] = fltIcoordZ;		/* double */
	cnt = 3;
	mgGetAttList ( rec1, fcode[0], &dval0, fcode[1], &dval1, fcode[2], &dval2, mgNULL );
	printf ( "%lf, %lf, %lf\n", dval0, dval1, dval2 );
	for ( i = 0; i < cnt; i++ ) {
		if ( rec1 && ( rec2 = mgGetAtt ( rec1, fcode[i] ) ) )
			mgPrintField ( rec2 );		/* print field */
	}
}

static void SetRec ( mgrec* rec, tagtype tag, mgcode fcode, char* cmd )
{
	int cval;
	short sval;
	unsigned short usval;
	int ival;
	unsigned int uival;
	float fval;
	double dval;

	switch ( tag ) {
	case mgtype_sval:
		sval = (short) ( atoi ( cmd ) );
		mgSetAttValS ( rec, fcode, sval );
		break;
	case mgtype_usval:
		usval = (unsigned short) ( atoi ( cmd ) );
		mgSetAttValUS ( rec, fcode, usval );
		break;
	case mgtype_ival:
		ival = (int) ( atoi ( cmd ) );
		mgSetAttValI ( rec, fcode, ival );
		break;
	case mgtype_uival:
		uival = (unsigned int) ( atoi ( cmd ) );
		mgSetAttValUI ( rec, fcode, uival );
		break;
	case mgtype_fval:
		fval = (float) ( atof ( cmd ) );
		mgSetAttValF ( rec, fcode, fval );
		break;
	case mgtype_dval:
		dval = (double) ( atof ( cmd ) );
		mgSetAttValD ( rec, fcode, dval );
		break;
	case mgtype_cval:
		cval = (int) (*cmd);
		mgSetAttValC ( rec, fcode, *cmd );
		break;
	case mgtype_ucval:
		cval = *cmd;
		mgSetAttValUC ( rec, fcode, *cmd );
		break;
	case mgtype_text:
		mgSetAttText ( rec, fcode, cmd );
		break;
	case mgtype_flag:
		ival = atoi ( cmd );
		mgSetAttFlag ( rec, fcode, (ival == 0 ? mgFALSE : mgTRUE) );
		break;
	default:
		printf ( "Invalid tag in SetRec()\n" );
		break;
	}
}

static mgbool action ( mgrec* db_rec, mgrec* par_rec, mgrec* rec, void *unused ) 
{	
	char* id;

	if ( !mgGetPrevious ( rec ) )
		printf ( "\n" );
	if ( id = mgGetName ( rec ) ) {
		printf ( "%s ", id );
		mgFree ( id ); /* mgGetName allocs, user must dealloc */
	}
	return ( mgTRUE );
}

static void WalkTree ( mgrec* rec )
{
	mgWalk ( rec, action, mgNULL, mgNULL, MGWALKNORDONLY + MGWALKMASTER + MGWALKVERTEX + MGWALKMASTERALL /*+ MGWALKON*/ );
	printf ( "\n" );
	/* mgWalk ( rec, udlr, mgNULL ); */
}

/********************************************************************/

static opfunc GetCmd ( int arg[] )
{
	static char cmd[ 128 ];
	char *tok;
	int opcode = 999;
	mgrec* rec;

	memset ( cmd, mgNULL, sizeof ( cmd ) );
	printf ("Open [fname] | Get [rec/field name] | Set [rec/field value] | \n");
	printf ("Next|PREv|PARent|Child|SUPer|SUB|REF|CAST|Print|Walk|wRite|CLOSE|Quit: ");
	gets ( cmd );

	if ( !strcmp( cmd, "open" ) || *cmd == 'O' || *cmd == 'o' ) {
		printf ("Name: ");
		gets ( cmd );
		Open ( &cmd[0] );
	}
	else if ( !strcmp( cmd, "get" ) || *cmd == 'G' || *cmd == 'g' ) {
		printf ("Rec/Field Name: ");
		gets ( cmd );
		if ( isalpha ( cmd[0] ) ) {
			GetRecByName ( Db, &cmd[0] );
		}
		else if ( cmd[0] == '0' ) {		/* more than one... */
			GetRecList ( Rec, cmd );
		}
		else {
			Fcode = atoi ( &cmd[0] );
			GetRec ( Rec, Fcode );
		}
	}
	else if ( !strcmp( cmd, "set" ) || *cmd == 'S' || *cmd == 's' ) {
		if ( rec = mgGetAtt ( Rec, Fcode ) ) {
			printf ("Value: ");
			gets ( cmd );
			SetRec ( Rec, rec->tag, Fcode, &cmd[0] );
			GetRec ( Rec, Fcode );		/* print the field out again */
		}
	}
	else if ( !strcmp( cmd, "next" ) || *cmd == 'N' || *cmd == 'n' ) {
		opcode = OP_mgGetNext;
	}
	else if ( !strcmp( cmd, "prev" ) || !strcmp( cmd, "pre" ) ) {
		opcode = OP_mgGetPrevious;
	}
	else if ( !strcmp( cmd, "parent" ) || !strcmp( cmd, "par" ) ) {
		opcode = OP_mgGetParent;
	}
	else if ( !strcmp( cmd, "child" ) || !strcmp( cmd, "c" ) ) {
		opcode = OP_mgGetChild;
	}
	else if ( !strcmp( cmd, "super" ) || !strcmp( cmd, "sup" ) ) {
		opcode = OP_mgGetNestedParent;
	}
	else if ( !strcmp( cmd, "sub" ) ) {
		opcode = OP_mgGetNestedChild;
	}
	else if ( !strcmp( cmd, "ref" ) ) {
		opcode = OP_mgGetReference;
	}
	else if ( !strcmp( cmd, "cast" ) ) {
		printf ("Field_name Rcode: ");
		gets ( cmd );
		tok = strtok ( cmd, DELIM );
		arg[0] = atoi ( tok );
		tok = strtok ( mgNULL, DELIM );
		arg[1] = atoi ( tok );
		opcode = OP_mgCastRec;
	}
	else if ( !strcmp( cmd, "print" ) || *cmd == 'P' || *cmd == 'p' ) {
		opcode = OP_mgPrintRec;
	}
	else if ( !strcmp( cmd, "close" ) ) {
		opcode = OP_mgCloseDb;
	}
	else if ( !strcmp( cmd, "quit" ) || *cmd == 'Q' || *cmd == 'q' ) {
		opcode = OP_exit;
	}
	else if ( !strcmp( cmd, "walk" ) || *cmd == 'W' || *cmd == 'w' )
		WalkTree ( Rec );
	else if ( !strcmp( cmd, "write" ) || *cmd == 'R' || *cmd == 'r' ) {
		if ( Db )
			mgWriteDb ( Db );
	}

	return opcode;
}

void main ( int argc, char* argv [] )
{
	opfunc op;
	int arg[ 8 ];
	static mgrec recs;
	int siteno;
	char *siteid, *errortxt;

	/* Walk_type = argc > 1 ? atoi ( argv[2] ) : 0; */

	/* Mgcolorinit (); */
	mgInit ( &argc, argv );
	if ( argc > 1 )
		if ( !ddInit ( DLL_NAME, &siteno ) )
			exit (1);		/* error msg already sent */

	/* mgWriteMaterialFile ( &recs, "hello" ); linker debug */

	while ( op = GetCmd ( arg ) ) {
		switch ( op ) {
		case OP_mgGetNext:
			if ( Rec && ( Rec1 = mgGetNext ( Rec ) ) )
				Rec = Rec1;
			else {
				mgGetError ( &siteid, &errortxt );
				printf ( "%s: %s\n", errortxt, (char*) arg[ 0 ] );
			}
			break;
		case OP_mgGetPrevious:
			if ( Rec && ( Rec1 = mgGetPrevious ( Rec ) ) )
				Rec = Rec1;
			else {
				mgGetError ( &siteid, &errortxt );
				printf ( "%s: %s\n", errortxt, (char*) arg[ 0 ] );
			}
			break;
		case OP_mgGetParent:
			if ( Rec && ( Rec1 = mgGetParent ( Rec ) ) )
				Rec = Rec1;
			else {
				mgGetError ( &siteid, &errortxt );
				printf ( "%s: %s\n", errortxt, (char*) arg[ 0 ] );
			}
			break;
		case OP_mgGetChild:
			if ( !Rec && Db ) Rec = Db;	/* down from topib */
			if ( Rec && ( Rec1 = mgGetChild ( Rec ) ) )
				Rec = Rec1;
			else {
				mgGetError ( &siteid, &errortxt );
				printf ( "%s: %s\n", errortxt, (char*) arg[ 0 ] );
			}
			break;
		case OP_mgGetNestedParent:
			if ( Rec && ( Rec1 = mgGetNestedParent ( Rec ) ) )
				Rec = Rec1;
			else {
				mgGetError ( &siteid, &errortxt );
				printf ( "%s: %s\n", errortxt, (char*) arg[ 0 ] );
			}
			break;
		case OP_mgGetNestedChild:
			if ( Rec && ( Rec1 = mgGetNestedChild ( Rec ) ) )
				Rec = Rec1;
			else {
				mgGetError ( &siteid, &errortxt );
				printf ( "%s: %s\n", errortxt, (char*) arg[ 0 ] );
			}
			break;
		case OP_mgGetReference:
			if ( Rec && ( Rec1 = mgGetReference ( Rec ) ) )
				Rec = Rec1;
			else {
				mgGetError ( &siteid, &errortxt );
				printf ( "%s: %s\n", errortxt, (char*) arg[ 0 ] );
			}
			break;
		case OP_mgCastRec:
			mgCastRec ( Rec, (mgcode) arg[0], (mgcode) arg[1], &Recs );	/* fltVtxCoordX:2123, fltIcoord:384 */
			Rec = &Recs;
			break;
		case OP_mgPrintRec:
			if ( Rec )
				mgPrintRec ( Rec );		/* current record */
			else
				mgPrintRec ( Db );		/* current file */
			break;
		case OP_mgCloseDb: {
			mgMatrix *m = mgGetMatrix ( Rec );
			if ( m ) {		/* test for matrix */
				int i;
				double *d;
				for ( i=0, d = (double *) m; i < 16; i++, d++ ) {
					printf ( "%lf ", *d );
					if ( i % 4 == 3 ) printf ( "\n" );
				}
			}
			if ( Db )
				mgCloseDb ( Db );		/* current file */
		}
		}
	}
}

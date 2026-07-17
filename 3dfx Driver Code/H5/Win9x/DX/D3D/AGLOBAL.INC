FAN_HEADER           = 01040bccbh
FAN_HEADER_CONT      = 01040bc53h

STRIP_HEADER         = 01000bccbh
STRIP_CONTINUE       = 01000bc53h

STRIP_FOGW_HEADER    = 01000fccbh
STRIP_FOGW_CONTINUE  = 01000fc53h

STRIP_GOURAD_HEADER  = 010003ccbh
FAN_GOURAD_HEADER    = 010403ccbh

STRIP_GOURAD_CONT    = 010003c53h

NOP_PACKET     = 0

_FIFO_PTR      = 0
_FIFO_ROOM     = 4
_VERTEX_TYPE   = 8
_VERTEX_SIZE   =12
_CULL_MASK     =16
_T0_OFFSET     =20
_SCALE_S       =24
_SCALE_T       =28
_TEXEL_OFFSET_S=32
_TEXEL_OFFSET_T=36
_SCALEZ        =40
_PIXEL_CENTER  =44
_CULL_HEADER   =48
_SAVED_RET_ADDR=52
_SAVED_COUNT   =56
_RC = 60
ifdef WINNT
_TEMPSPACE = 64
ifndef STBKNI
STBKNI = 0
endif
endif  ; WINNT
if STBKNI
_kni_array		=64
_SCALE_TS11     =64             ;16 bytes 1.0 1.0 SCALE_S SCALE_T
_00PIXEL_CENTER =80             ;16 bytes PIXEL_CENTER PIXEL_CENTER 0.0 0.0 
_TEXEL_OFFSET_TS00 =96          ;16 bytes 0.0 0.0 _TEXEL_OFFSET_S _TEXEL_OFFSET_T
_SCALEZ1        =112            ;8 bytes  _SCALEZ 1.0 
_TEMPSPACE      =128
ifdef DCT_FIX
_SCALEWS        =128            ;ScaleW factors
_TEMPSPACE      =160            ; Temp Scratch Pad
endif ;DCT_FIX
else ;NOT STBKNI
_TEMPSPACE     =64
endif

FVF_SIZE                         = _TEMPSPACE
CMD_START                        = _TEMPSPACE + 4
CMD_CONT                         = _TEMPSPACE + 8
CMD_TRI                          = _TEMPSPACE + 12
SETUP_FLAG                       = _TEMPSPACE + 16
SPECULAR_OFFSET                  = _TEMPSPACE + 20
T1_OFFSET                        = _TEMPSPACE + 24
RHW_OFFSET                       = _TEMPSPACE + 28
Z_OFFSET                         = _TEMPSPACE + 32
COLOR_OFFSET                     = _TEMPSPACE + 36
S1_SCALE                         = _TEMPSPACE + 40
T1_SCALE                         = _TEMPSPACE + 44
S1_TEXEL_OFFSET                  = _TEMPSPACE + 48
T1_TEXEL_OFFSET                  = _TEMPSPACE + 52
SCALE_W                          = _TEMPSPACE + 56
RENDER_STATE                     = _TEMPSPACE + 60
WBUFFER_SCALE_AW                 = _TEMPSPACE + 64
WBUFFER_SCALE_BW                 = _TEMPSPACE + 68
CMD_ALPHA_BLEND_SPECULAR         = _TEMPSPACE + 72
CMD_NO_ALPHA_BLEND_SPECULAR      = _TEMPSPACE + 76
CMD_CONT_ALPHA_BLEND_SPECULAR    = _TEMPSPACE + 80
CMD_CONT_NO_ALPHA_BLEND_SPECULAR = _TEMPSPACE + 84

sx    =0
sy    =4
sz    =8
rhw   =12
color =16
spec  =20
tu    =24
tv    =28

BIT_16         = 0010000h
BIT_22         = 0400000h
BIT_23         = 0800000h

CacheLineSize   equ     32
CacheNop        equ     nop

movq_esi_mm0    equ     db 0Fh, 7Fh, 46h, 00h
movq_esi_mm7    equ     db 0Fh, 7Fh, 7Dh, 00h

; SST sSetupMode bitfield from h3defs.h
BIT_SST_SETUP_RGB		   	   	equ	(0)
BIT_SST_SETUP_A				   	equ	(1)
BIT_SST_SETUP_Z				   	equ	(2)
BIT_SST_SETUP_Wfbi			   	equ	(3)
BIT_SST_SETUP_W0			   	equ	(4)
BIT_SST_SETUP_ST0			   	equ	(5)
BIT_SST_SETUP_W1		   	   	equ	(6)
BIT_SST_SETUP_ST1			   	equ	(7)
BIT_SST_SETUP_FAN			   	equ	(16)
BIT_SST_SETUP_EN_CULLING	   	equ	(17)
BIT_SST_SETUP_CULL_NEGATIVE	   	equ	(18)
BIT_SST_SETUP_DIS_PINGPONG	   	equ	(19)
SST_SETUP_RGB					equ	(1 SHL BIT_SST_SETUP_RGB)
SST_SETUP_A						equ	(1 SHL BIT_SST_SETUP_A)
SST_SETUP_Z						equ	(1 SHL BIT_SST_SETUP_Z)
SST_SETUP_Wfbi					equ	(1 SHL BIT_SST_SETUP_Wfbi)
SST_SETUP_W0					equ	(1 SHL BIT_SST_SETUP_W0)
SST_SETUP_ST0					equ	(1 SHL BIT_SST_SETUP_ST0)
SST_SETUP_W1					equ	(1 SHL BIT_SST_SETUP_W1)
SST_SETUP_ST1					equ	(1 SHL BIT_SST_SETUP_ST1)
SST_SETUP_FAN					equ	(1 SHL BIT_SST_SETUP_FAN)
SST_SETUP_EN_CULLING			equ	(1 SHL BIT_SST_SETUP_EN_CULLING)
SST_SETUP_CULL_NEGATIVE			equ	(1 SHL BIT_SST_SETUP_CULL_NEGATIVE)
SST_SETUP_DIS_PINGPONG			equ	(1 SHL BIT_SST_SETUP_DIS_PINGPONG)

; Override renderstate management bitfield from D3global.h
BIT_STATE_REQUIRES_VERTS_AREA	equ	( 0)
BIT_STATE_REQUIRES_RGB			equ	( 1)
BIT_STATE_REQUIRES_IT_RGB		equ ( 2)
BIT_STATE_REQUIRES_IT_ALPHA		equ ( 3)
BIT_STATE_REQUIRES_OOZ			equ ( 4)
BIT_STATE_REQUIRES_W_FBI		equ ( 5)
BIT_STATE_REQUIRES_W_TMU0		equ ( 6) ; currently broadcast to FBI & all TMUs
BIT_STATE_REQUIRES_ST_TMU0		equ ( 7) ; currently broadcast to FBI & all TMUs
BIT_STATE_REQUIRES_W_TMU1		equ ( 8) ; currently broadcast to FBI & all TMUs
BIT_STATE_REQUIRES_ST_TMU1		equ ( 9) ; currently broadcast to FBI & all TMUs
BIT_STATE_REQUIRES_C0			equ (10)
BIT_STATE_REQUIRES_VERTEXFOG	equ (11)
BIT_STATE_REQUIRES_SPECULAR		equ (12)
BIT_STATE_REQUIRES_HWFOG		equ	(13)
BIT_STATE_REQUIRES_WRAP	   		equ	(14)
BIT_STATE_REQUIRES_PERSPECTIVE	equ	(15)
BIT_STATE_REQUIRES_ZBUFFER		equ	(16)
BIT_STATE_REQUIRES_WBUFFER		equ	(17)
BIT_STATE_REQUIRES_AA			equ	(20)
BIT_STATE_BAD_CPU				equ	(24)
BIT_STATE_NOT_PERSPECTIVE		equ	(25)	; opposite of STATE_REQUIRES_PERSPECTIVE
BIT_STATE_NOT_SOLID_FILL		equ	(26)
BIT_STATE_MULTITEXTURING		equ	(27)
BIT_STATE_VERTEX_FOG_WITH_Z		equ	(28)
BIT_FLAGS_FVF_IDX2				equ	(30)	; DX >= 6
BIT_FLAGS_FVF_EXECUTE			equ	(31)	; DX >= 6
STATE_REQUIRES_VERTS_AREA		equ	(1 SHL BIT_STATE_REQUIRES_VERTS_AREA)
STATE_REQUIRES_RGB				equ	(1 SHL BIT_STATE_REQUIRES_RGB)
STATE_REQUIRES_IT_RGB			equ (1 SHL BIT_STATE_REQUIRES_IT_RGB)
STATE_REQUIRES_IT_ALPHA			equ (1 SHL BIT_STATE_REQUIRES_IT_ALPHA)
STATE_REQUIRES_OOZ				equ (1 SHL BIT_STATE_REQUIRES_OOZ)
STATE_REQUIRES_W_FBI			equ (1 SHL BIT_STATE_REQUIRES_W_FBI)
STATE_REQUIRES_W_TMU0			equ (1 SHL BIT_STATE_REQUIRES_W_TMU0)	; currently broadcast to FBI & all TMUs
STATE_REQUIRES_ST_TMU0			equ (1 SHL BIT_STATE_REQUIRES_ST_TMU0)	; currently broadcast to FBI & all TMUs
STATE_REQUIRES_W_TMU1			equ (1 SHL BIT_STATE_REQUIRES_W_TMU1)	; currently broadcast to FBI & all TMUs
STATE_REQUIRES_ST_TMU1			equ (1 SHL BIT_STATE_REQUIRES_ST_TMU1)	; currently broadcast to FBI & all TMUs
STATE_REQUIRES_C0				equ (1 SHL BIT_STATE_REQUIRES_C0)
STATE_REQUIRES_VERTEXFOG		equ (1 SHL BIT_STATE_REQUIRES_VERTEXFOG)
STATE_REQUIRES_SPECULAR			equ (1 SHL BIT_STATE_REQUIRES_SPECULAR)
STATE_REQUIRES_HWFOG			equ	(1 SHL BIT_STATE_REQUIRES_HWFOG)
STATE_REQUIRES_WRAP	   			equ	(1 SHL BIT_STATE_REQUIRES_WRAP)
STATE_REQUIRES_PERSPECTIVE		equ	(1 SHL BIT_STATE_REQUIRES_PERSPECTIVE)
STATE_REQUIRES_ZBUFFER			equ	(1 SHL BIT_STATE_REQUIRES_ZBUFFER)
STATE_REQUIRES_WBUFFER			equ	(1 SHL BIT_STATE_REQUIRES_WBUFFER)
STATE_REQUIRES_AA				equ	(1 SHL BIT_STATE_REQUIRES_AA)
STATE_BAD_CPU					equ	(1 SHL BIT_STATE_BAD_CPU)
STATE_NOT_PERSPECTIVE			equ	(1 SHL BIT_STATE_NOT_PERSPECTIVE)	; opposite of STATE_REQUIRES_PERSPECTIVE
STATE_NOT_SOLID_FILL			equ	(1 SHL BIT_STATE_NOT_SOLID_FILL)
STATE_MULTITEXTURING			equ	(1 SHL BIT_STATE_MULTITEXTURING)
STATE_VERTEX_FOG_WITH_Z			equ	(1 SHL BIT_STATE_VERTEX_FOG_WITH_Z)
FLAGS_FVF_IDX2					equ	(1 SHL BIT_FLAGS_FVF_IDX2)	; DX >= 6
FLAGS_FVF_EXECUTE				equ	(1 SHL BIT_FLAGS_FVF_EXECUTE)	; DX >= 6

CMD_FOG_MODE  				equ (00010211h)
CMD_ALPHA_SPEC_MODE			equ (00010219h)
CMD_FBZ_MODE				equ (00010221h)
CMD_NAPALM_COMBINE_MODE		equ (00010c11h)
CMD_FBZ_COLORPATH			equ (00010209h)
CMD_ALPHA_DIFF_MODE			equ (00028219h)


; AllenH 3 Sept, 2000
; There's a structure called TEXTURESTAGESTATE structure in d3global.h.
; There's an array of 8 of these in the rendering context, right before
; the wrap flags.  "texXformFlags" a DWORD from D3DTSS_TEXTURETRANSFORMFLAGS 
; had to be added so we have to adjust all the pointers by 8*4.  This is
; the hack I put in to do this ... these hard-coded offsets are really bad!!!

TEXGENOFFSET = (8*4)	; add texXformFlags for 8 textures

IFDEF WINNT
OFFSET_RC_PPDEV             equ (0004h)
OFFSET_RC_SHADE_MODE		equ (0010h) 
OFFSET_RC_ZWRITE_ENABLE		equ (001Ch) 
OFFSET_RC_Z_ENABLE			equ (0020h)
OFFSET_RC_TEXTURE_PTR       equ (002Ch) 
OFFSET_RC_ALPHABLEND_ENABLE	equ (0050h) 
OFFSET_RC_FOG_ENABLE		equ (0068h) 
OFFSET_RC_SPECULAR_FLAG     equ (009Ch) 
OFFSET_RC_WRAPT0			equ (0430h+TEXGENOFFSET) 
OFFSET_RC_WRAPT1			equ (0434h+TEXGENOFFSET) 
OFFSET_RC_FBZ_MODE			equ (0460h+TEXGENOFFSET) 
OFFSET_RC_ALPHA_MODE		equ (0464h+TEXGENOFFSET) 
OFFSET_RC_FOG_MODE			equ (0468h+TEXGENOFFSET) 
OFFSET_RC_FBZ_COLORPATH		equ (047Ch+TEXGENOFFSET) 
OFFSET_RC_COMBINE_MODE_FBI	equ (04d0h+TEXGENOFFSET) 
ELSE
OFFSET_RC_PPDEV             equ (0008h)
OFFSET_RC_SHADE_MODE		equ (0014h)
OFFSET_RC_ZWRITE_ENABLE		equ (0020h)
OFFSET_RC_Z_ENABLE			equ (0024h)
OFFSET_RC_TEXTURE_PTR       equ (0030h)
OFFSET_RC_ALPHABLEND_ENABLE	equ (0054h)
OFFSET_RC_FOG_ENABLE		equ (006ch)
OFFSET_RC_SPECULAR_FLAG     equ (00A0h)
OFFSET_RC_WRAPT0			equ (0434h+TEXGENOFFSET)
OFFSET_RC_WRAPT1			equ (0438h+TEXGENOFFSET)
OFFSET_RC_FBZ_MODE			equ (0464h+TEXGENOFFSET)
OFFSET_RC_ALPHA_MODE		equ (0468h+TEXGENOFFSET)
OFFSET_RC_FOG_MODE			equ (046ch+TEXGENOFFSET)
OFFSET_RC_FBZ_COLORPATH		equ (0480h+TEXGENOFFSET)
; OFFSET_RC_VENDOR_DEVICE_ID	equ (04b0h+TEXGENOFFSET)
OFFSET_RC_COMBINE_MODE_FBI	equ (04d4h+TEXGENOFFSET)
ENDIF

IFDEF WINNT
OFFSET_PDEV_VENDOR_DEVICE_ID equ (00ECh)
ELSE
OFFSET_PDEV_VENDOR_DEVICE_ID equ (04b0h)
ENDIF


; from h3g.h
SST_VENDOR_DEVICE_ID_H4		equ (0005121ah)	; Voodoo3
SST_VENDOR_DEVICE_ID_H5_6	equ (0006121ah)	; Napalm
SST_VENDOR_DEVICE_ID_H5_7	equ (0007121ah)	; Napalm
SST_VENDOR_DEVICE_ID_H5_8	equ (0008121ah)	; Napalm
SST_VENDOR_DEVICE_ID_H5_9	equ (0009121ah)	; Napalm
SST_VENDOR_DEVICE_ID_H5_A	equ (000A121ah)	; Napalm2
SST_VENDOR_DEVICE_ID_H5_B	equ (000B121ah)	; Napalm2
SST_VENDOR_DEVICE_ID_H5_C	equ (000C121ah)	; Napalm2
SST_VENDOR_DEVICE_ID_H5_D	equ (000D121ah)	; Napalm2
SST_VENDOR_DEVICE_ID_H5_E	equ (000E121ah)	; Reserved
SST_VENDOR_DEVICE_ID_H5_F	equ (000F121ah)	; Reserved

; from d3dtypes.h
D3DSHADE_FLAT				equ	(1)
D3DSHADE_GOURAUD			equ	(2)
D3DSHADE_PHONG				equ	(3)
D3DWRAP_U					equ (1)
D3DWRAP_V					equ (2)


; from h3defs.h
SST_FOGADD							equ	(1 SHL 1)
SST_FOGMULT							equ	(1 SHL 2)
SST_ENALPHABLEND					equ	(1 SHL 4)
SST_A_ONE							equ	(4)
SST_RGBSRCFACT_SHIFT				equ (8)		; RGB source factor
SST_RGBDSTFACT_SHIFT				equ	(12)	; RGB destination factor
SST_RGBDSTFACT						equ (0fh SHL SST_RGBDSTFACT_SHIFT)
SST_ZFUNC_LT						equ (1 SHL 5)
SST_ZFUNC_EQ						equ (1 SHL 6)
SST_ZFUNC_GT						equ (1 SHL 7)
SST_ZFUNC_SHIFT						equ (5)
SST_ZFUNC							equ (7 SHL SST_ZFUNC_SHIFT)
SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK	equ	(1 SHL 29)
SST_CM_DISABLE_CHROMA_SUBSTITUTION	equ	(1 SHL 30)
SST_CM_USE_COMBINE_MODE				equ	(1 SHL 31)
SST_CM_CCA_OTHERSELECT_SHIFT		equ (16)
SST_CM_CCA_OTHERSELECT				equ (3 SHL SST_CM_CCA_OTHERSELECT_SHIFT)
SST_CM_CCA_OTHERSELECT_IA			equ (0 SHL SST_CM_CCA_OTHERSELECT_SHIFT) ;Iterated alpha
SST_CM_CCA_OTHERSELECT_TA			equ (1 SHL SST_CM_CCA_OTHERSELECT_SHIFT) ;Texture alpha
SST_CM_CCA_OTHERSELECT_C1_A			equ (2 SHL SST_CM_CCA_OTHERSELECT_SHIFT) ;Color 1 Alpha
SST_CM_CCA_OTHERSELECT_LFB_A		equ (3 SHL SST_CM_CCA_OTHERSELECT_SHIFT) ;LFB Alpha
SST_RGBSELECT_SHIFT					equ (0)
SST_RGBSELECT						equ (3 SHL SST_RGBSELECT_SHIFT)
SST_RGBSEL_RGBA						equ (0 SHL SST_RGBSELECT_SHIFT)
SST_CCOMBINE_SHIFT					equ (8)		; RGB COMBINE MODES
SST_CCOMBINE						equ (01FFh SHL SST_CCOMBINE_SHIFT)

; 2/14/00 added for single stage rendering for trilinear-mipmapping
SST_CM_CC_OTHERSELECT_SHIFT			equ ( 0)
SST_CM_CC_OTHERSELECT				equ (7 SHL SST_CM_CC_OTHERSELECT_SHIFT)
SST_CM_CC_OTHERSELECT_IRGB			equ (0 SHL SST_CM_CC_OTHERSELECT_SHIFT)	;Iterated RGB
SST_CM_CC_LOCALSELECT_SHIFT			equ ( 3)
SST_CM_CC_LOCALSELECT				equ (7 SHL SST_CM_CC_LOCALSELECT_SHIFT)   
SST_CM_CC_MSELECT_7_SHIFT			equ ( 6)
SST_CM_CC_MSELECT_7					equ (3 SHL SST_CM_CC_MSELECT_7_SHIFT)
SST_CM_CC_INVERT_OTHER_SHIFT		equ ( 9)
SST_CM_CC_INVERT_OTHER				equ (3 SHL SST_CM_CC_INVERT_OTHER_SHIFT)
SST_CM_CC_INVERT_LOCAL_SHIFT		equ (11)
SST_CM_CC_INVERT_LOCAL				equ (3 SHL SST_CM_CC_INVERT_LOCAL_SHIFT)
SST_CM_CC_INVERT_ADD_LOCAL			equ (1 SHL 13)
SST_CM_CC_OUTSHIFT_SHIFT			equ (14)
SST_CM_CC_OUTSHIFT					equ (3 SHL SST_CM_CC_OUTSHIFT_SHIFT)


; local flags
BIT_SPEC				equ ( 0)
BIT_FLAT				equ ( 1)
BIT_WFBI				equ ( 2)
BIT_ALPHA				equ ( 3)
BIT_TX0					equ ( 4)
BIT_TX1					equ ( 5)
BIT_WRAP_TX				equ ( 6)	; wrap s or t with a valid texture
BIT_Z					equ ( 7)

BIT_COLOR				equ	( 8)	
BIT_WBUFFER				equ ( 9)
;BIT_SPEC_AND_TEXTURES	equ (10)	; 2nd pass required for specular (specular on textures)
BIT_VFOG				equ (11)
BIT_WRAP_T0_S			equ (12)
BIT_WRAP_T0_T			equ (13)
BIT_WRAP_T1_S			equ (14)
BIT_WRAP_T1_T			equ (15)
BIT_W					equ (16)
BIT_PC					equ (17)
BIT_NAPALM				equ (18)
BIT_NEVER_AUTOSTRIP		equ (19)	; wrap, flat shaded triangles, or use for debugging autostrip
BIT_BADFVF      		equ (20)
BIT_FOG_ENABLE			equ (21)	; pRc->fogEnable
BIT_Z_ENABLE			equ (22)	; pRc->zEnable
BIT_Z_WRITE_ENABLE		equ (23)	; pRc->zWriteEnable

FLAG_SPEC				equ (1 SHL BIT_SPEC)
FLAG_FLAT				equ (1 SHL BIT_FLAT)
FLAG_WFBI				equ (1 SHL BIT_WFBI)
FLAG_ALPHA				equ (1 SHL BIT_ALPHA)
FLAG_TX0				equ (1 SHL BIT_TX0)
FLAG_TX1				equ (1 SHL BIT_TX1)
FLAG_WRAP_TX			equ (1 SHL BIT_WRAP_TX)
FLAG_Z					equ (1 SHL BIT_Z)
FLAG_COLOR				equ	(1 SHL BIT_COLOR)
FLAG_WBUFFER			equ (1 SHL BIT_WBUFFER)
;FLAG_SPEC_AND_TEXTURES	equ (1 SHL BIT_SPEC_AND_TEXTURES)
FLAG_VFOG				equ (1 SHL BIT_VFOG)
FLAG_WRAP_T0_S			equ (1 SHL BIT_WRAP_T0_S)
FLAG_WRAP_T0_T			equ (1 SHL BIT_WRAP_T0_T)
FLAG_WRAP_T1_S			equ (1 SHL BIT_WRAP_T1_S)
FLAG_WRAP_T1_T			equ (1 SHL BIT_WRAP_T1_T)
FLAG_W					equ (1 SHL BIT_W)
FLAG_PC					equ (1 SHL BIT_PC)
FLAG_NAPALM				equ (1 SHL BIT_NAPALM)
FLAG_NEVER_AUTOSTRIP	equ (1 SHL BIT_NEVER_AUTOSTRIP)
FLAG_BADFVF     		equ (1 SHL BIT_BADFVF)
FLAG_FOG_ENABLE			equ (1 SHL BIT_FOG_ENABLE)
FLAG_Z_ENABLE			equ (1 SHL BIT_Z_ENABLE)
FLAG_Z_WRITE_ENABLE		equ (1 SHL BIT_Z_WRITE_ENABLE)



IFDEF TnL_HAL


KNI_RC	STRUCT
	UNION	CMD_START
		DIFF_SPEC		QWORD	?	; full tri start cmds
		STRUCT
			DIFF		DWORD	?	; full non-spec tri start cmd
			SPEC		DWORD	?	; full specular tri start cmd
		ENDS
	ENDS
	UNION	CMD_CONT
		DIFF_SPEC		QWORD	?	; partial tri start cmds
		STRUCT
			DIFF		DWORD	?	; partial non-spec tri start cmd
			SPEC		DWORD	?	; partial specular tri start cmd
		ENDS
	ENDS
	UNION	
		PREVCB			QWORD	?	; previous vertex pointers
		STRUCT
			PREVC		DWORD	?	; pointer to previous vertex C
			PREVB		DWORD	?	; pointer to previous vertex B
		ENDS
	ENDS
	PREVA				DWORD	?	; pointer to previous vertex A
	LASTSPECRENDERED	DWORD	?	; last tri was rendered to specular buffer
	LASTTRITYPE			DWORD	?	; last tri was a fan or strip (for autostripping)
	CMDFIFOPTR			DWORD	?	; current command fifo pointer
	CMDFIFOROOM			DWORD	?	; space left in the command fifo (in bytes, not in entries)
	CMDFIFOBUMP			DWORD	?	; bump count for the P6 write-buffer bug
	SPECBUFFBASE		DWORD	?	; array where we store specular buffer data
	SPECBUFFPTR			DWORD	?	; pointer to last location written to in the specular buffer
	SPECBUFFTHRESHOLD	DWORD	?	; point at which to dump specular buffer
	RFBITS				DWORD	?	; rendering flags (a bitfield)
	LAST_RFBITS			DWORD	?	; RfBits from the last primitive
	RENDERFUNCTION_NC	DWORD	?	; pointer to the current render function
	PRC					DWORD	?	; pointer to the rendering context
KNI_RC	ENDS


TLBN	UNION
	STRUCT		; each dword
		X				REAL4	?
		Y				REAL4	?
		Z				REAL4	?
		WFBI			REAL4	?
		W				REAL4	?
		CLIPCODE		DWORD	?
		DIFFUSE			DWORD	?
		SPECULAR		DWORD	?
		TX0_S			REAL4	?
		TX0_T			REAL4	?
		TX1_S			REAL4	?
		TX1_T			REAL4	?
	ENDS
	STRUCT		; each qword
		XY				QWORD	?
		Z_WFBI			QWORD	?
		W_CLIPCODE		QWORD	?
		COLORS			QWORD	?
		TX0				QWORD	?
		TX1				QWORD	?
	ENDS
	STRUCT		; each oword
		XYZ_WFBI		OWORD	?
		W_CLIP_COLORS	OWORD	?
		TEXTURES		OWORD	?
	ENDS
TLBN	ENDS




SOA_SIZE	equ	4

AOS_UV	STRUCT
	U		REAL4	?
	V		REAL4	?
AOS_UV	ENDS

SOA_UV	UNION
	F		AOS_UV	SOA_SIZE dup(<?,?>)
	M		QWORD	SOA_SIZE dup(?)
SOA_UV	ENDS

MMX2	STRUCT
	LO		QWORD	?
	HI		QWORD	?
MMX2	ENDS

SOA_FLOAT	UNION
	F		REAL4	SOA_SIZE dup(<?,?,?,?>)
	M		MMX2	<>
	X		OWORD	?
SOA_FLOAT	ENDS

SOA_XYZ		STRUCT
	X		SOA_FLOAT	<>
	Y		SOA_FLOAT	<>
	Z		SOA_FLOAT	<>
SOA_XYZ		ENDS

SOA_XYZW	STRUCT
	X		SOA_FLOAT	<>
	Y		SOA_FLOAT	<>
	Z		SOA_FLOAT	<>
	W		SOA_FLOAT	<>
SOA_XYZW	ENDS

SOA_MATRIX	STRUCT
	_11		SOA_FLOAT	<>
	_12		SOA_FLOAT	<>
	_13		SOA_FLOAT	<>
	_14		SOA_FLOAT	<>
	_21		SOA_FLOAT	<>
	_22		SOA_FLOAT	<>
	_23		SOA_FLOAT	<>
	_24		SOA_FLOAT	<>
	_31		SOA_FLOAT	<>
	_32		SOA_FLOAT	<>
	_33		SOA_FLOAT	<>
	_34		SOA_FLOAT	<>
	_41		SOA_FLOAT	<>
	_42		SOA_FLOAT	<>
	_43		SOA_FLOAT	<>
	_44		SOA_FLOAT	<>
SOA_MATRIX	ENDS


ENDIF ;TnL_HAL
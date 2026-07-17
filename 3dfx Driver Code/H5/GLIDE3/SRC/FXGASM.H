
;----------------------------------------------------------------------
; Assembler offsets for SSTREGS struct
;----------------------------------------------------------------------



;----------------------------------------------------------------------
; Assembler offsets for GC struct
;----------------------------------------------------------------------

base_ptr		= 00000074h
reg_ptr			= 00000078h
lfb_ptr			= 00000080h
cull_mode		= 000001e0h
tsuDataList		= 00000124h
triPacketHdr	= 00000de0h
cullStripHdr	= 00000de4h
paramMask	= 00000de8h
fifoStart	= 00000e0ch
fifoEnd	= 00000e10h
fifoOffset	= 00000e14h
fifoSize	= 00000e18h
fifoJmpHdr	= 00000e1ch
fifoPtr	= 00000dech
fifoRead	= 00000df0h
fifoRoom	= 00000df4h
roomToReadPtr	= 00000e24h
roomToEnd	= 00000e28h
lfbLockCount	= 00000e2ch
triSetupProc	= 00000dcch
drawTrianglesProc	= 00000dd0h
drawVertexList	= 00000dcch
vertexSize	= 00000b78h
vertexStride	= 00000b74h
invalid	= 00000b80h
CoordinateSpace	= 00000da0h
paramIndex	= 000001e4h
vp_hwidth	= 00000b08h
vp_hheight	= 00000b0ch
vp_hdepth	= 00000b10h
vp_ox	= 00000afch
vp_oy	= 00000b00h
vp_oz	= 00000b04h
colorType	= 00000b7ch
wInfo_offset	= 00000b28h
qInfo_mode	= 00000b5ch
qInfo_offset	= 00000b60h
q0Info_offset	= 00000b68h
q1Info_offset	= 00000b70h
q0Info_mode	= 00000b64h
q1Info_mode	= 00000b6ch
fogInfo_offset	= 00000b38h
fogInfo_mode	= 00000b34h
depth_range	= 00000a4ch
tmu0_s_scale	= 00000a0ch
tmu0_t_scale	= 00000a10h
tmu1_s_scale	= 00000a2ch
tmu1_t_scale	= 00000a30h
fbi_fbzMode	= 00000210h
curTriSize		= 0000004ch
trisProcessed		= 0000000ch
trisDrawn		= 00000010h
lostContext		= 000096c4h
windowed		= 000096cch
bInfo		= 0000011ch

;----------------------------------------------------------------------
; Assembler offsets for GlideRoot struct
;----------------------------------------------------------------------

p6Fencer		= 00000000h
current_sst		= 0000000ch
CPUType			= 00000010h
tlsOffset			= 00000008h
pool_f255	= 0000002ch
pool_f1	= 00000028h
SIZEOF_GrState		= 00000bc4h
SIZEOF_GC		= 0000971ch

SIZEOF_GlideRoot	= 000bd26ch


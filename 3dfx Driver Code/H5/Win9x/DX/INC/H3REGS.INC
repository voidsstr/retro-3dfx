
option expr32
option casemap:none

; Begin of file ..\..\inc\h3regs.h
FxU32		TYPEDEF		DWORD

sstioregs		STRUCT 1t
status		FxU32		?
pciInit0		FxU32		?
sipMonitor		FxU32		?
lfbMemoryConfig		FxU32		?
miscInit0		FxU32		?
miscInit1		FxU32		?
dramInit0		FxU32		?
dramInit1		FxU32		?
agpInit		FxU32		?
tmuGbeInit		FxU32		?
vgaInit0		FxU32		?
vgaInit1		FxU32		?
dramCommand		FxU32		?
dramData		FxU32		?
strapInfo		FxU32		?
vidTvOutBlankVCount		FxU32		?
pllCtrl0		FxU32		?
pllCtrl1		FxU32		?
pllCtrl2		FxU32		?
dacMode		FxU32		?
dacAddr		FxU32		?
dacData		FxU32		?
vidMaxRGBDelta		FxU32		?
vidProcCfg		FxU32		?
hwCurPatAddr		FxU32		?
hwCurLoc		FxU32		?
hwCurC0		FxU32		?
hwCurC1		FxU32		?
vidInFormat		FxU32		?
vidTvOutBlankHCount		FxU32		?
vidSerialParallelPort		FxU32		?
vidInXDecimDeltas		FxU32		?
vidInDecimInitErrs		FxU32		?
vidInYDecimDeltas		FxU32		?
vidPixelBufThold		FxU32		?
vidChromaMin		FxU32		?
vidChromaMax		FxU32		?
vidCurrentLine		FxU32		?
vidScreenSize		FxU32		?
vidOverlayStartCoords		FxU32		?
vidOverlayEndScreenCoord		FxU32		?
vidOverlayDudx		FxU32		?
vidOverlayDudxOffsetSrcWidth		FxU32		?
vidOverlayDvdy		FxU32		?
vgaRegister		FxU32		12t DUP (?)
vidOverlayDvdyOffset		FxU32		?
vidDesktopStartAddr		FxU32		?
vidDesktopOverlayStride		FxU32		?
vidInAddr0		FxU32		?
vidInAddr1		FxU32		?
vidInAddr2		FxU32		?
vidInStride		FxU32		?
vidCurrOverlayStartAddr		FxU32		?
lfbMemoryTileCtrl		FxU32		?
lfbMemoryTileCompare		FxU32		?
sstioregs		ENDS

SstIORegs		TYPEDEF		sstioregs

cmdfifo		STRUCT 1t
baseAddrL		FxU32		?
baseSize		FxU32		?
bump		FxU32		?
readPtrL		FxU32		?
readPtrH		FxU32		?
aMin		FxU32		?
unusedA		FxU32		?
aMax		FxU32		?
unusedB		FxU32		?
depth		FxU32		?
holeCount		FxU32		?
reserved		FxU32		?
cmdfifo		ENDS

CmdFifo		TYPEDEF		cmdfifo

sstcregs		STRUCT 1t
agpReqSize		FxU32		?
hostAddrLow		FxU32		?
hostAddrHigh		FxU32		?
graphicsAddr		FxU32		?
graphicsStride		FxU32		?
moveCMD		FxU32		?
reservedL		FxU32		2t DUP (?)
cmdFifo0		CmdFifo		<>
cmdFifo1		CmdFifo		<>
cmdFifoThresh		FxU32		?
cmdHoleInit		FxU32		?
reservedO		FxU32		6t DUP (?)
reservedP		FxU32		8t DUP (?)
reservedQ		FxU32		8t DUP (?)
reservedR		FxU32		8t DUP (?)
yuvBaseAddr		FxU32		?
yuvStride		FxU32		?
reservedS		FxU32		6t DUP (?)
crc1		FxU32		?
reservedT		FxU32		3t DUP (?)
crc2		FxU32		?
sstcregs		ENDS

SstCRegs		TYPEDEF		sstcregs

sstgregs		STRUCT 1t
status		FxU32		?
unused0		FxU32		?
clip0min		FxU32		?
clip0max		FxU32		?
dstBaseAddr		FxU32		?
dstFormat		FxU32		?
srcColorkeyMin		FxU32		?
srcColorkeyMax		FxU32		?
dstColorkeyMin		FxU32		?
dstColorkeyMax		FxU32		?
bresError0		FxU32		?
bresError1		FxU32		?
rop		FxU32		?
srcBaseAddr		FxU32		?
commandEx		FxU32		?
lineStipple		FxU32		?
lineStyle		FxU32		?
pattern0alias		FxU32		?
pattern1alias		FxU32		?
clip1min		FxU32		?
clip1max		FxU32		?
srcFormat		FxU32		?
srcSize		FxU32		?
srcXY		FxU32		?
colorBack		FxU32		?
colorFore		FxU32		?
dstSize		FxU32		?
dstXY		FxU32		?
command		FxU32		?
reserved		FxU32		3t DUP (?)
launch		FxU32		32t DUP (?)
colorPattern		FxU32		64t DUP (?)
sstgregs		ENDS

SstGRegs		TYPEDEF		sstgregs

; End of file ..\..\inc\h3regs.h

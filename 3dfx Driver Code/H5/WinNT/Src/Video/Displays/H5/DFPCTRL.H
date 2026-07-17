#ifndef _DFPCTRL_H_
#define _DFPCTRL_H_




enum dfpPacketType {
				    dfpGetFlatPanelStatus,
				    dfpGetPanelAttributes,
				    dfpTurnOnFlatPanel,
				    dfpTurnOffFlatPanel};
				   
#if 0
typedef struct dfpPanelAttribsStruct {
FxU32 panelWidth;
FxU32 panelHeight;
FxU32 panelRefreshRate;
FxU32 panelIsScaling;
FxU32 panelIsActiveAtBoot;
} DFPATTRIBS, * PDFPATTRIBS;
#endif

typedef struct dfpStatusStruct {
FxU8 panelPresent;
FxU8 panelActiveAtBoot;
FxU8 panelActive;
} DFPSTATUS, * PDFPSTATUS;

typedef struct dfpRequestPkt {
   FxU32 dfpPacketFunc;                 // Type of packet
   union dfpReqOptData {
	  FxU32 dfpControl;
	  DFPSTATUS dfpStatusResponse;
   } dfpOptData;
} DFPDEVICEPACKET,
  * PDFPDEVICEPACKET;

#endif

#ifndef _DFPCTRL_H_
#define _DFPCTRL_H_




enum dfpPacketType {
					dfpGetPanelUniqueName,
				    dfpGetFlatPanelStatus,
				    dfpGetPanelAttributes,
				    dfpTurnOnFlatPanel,
				    dfpTurnOffFlatPanel};
				   
typedef struct dfpStatusStruct {
FxU8 panelPresent;
FxU8 panelActiveAtBoot;
FxU8 panelActive;
FxU8 panelScaling;
FxU8 dfpCapable;
} DFPSTATUS, * PDFPSTATUS;

#define UNIQUENAMELEN 16
typedef struct dfpRequestPkt {
   FxU32 dfpPacketFunc;                 // Type of packet
   union dfpReqOptData {
	  FxU32 dfpControl;
	  DFPSTATUS dfpStatusResponse;
	  char uniqueName[UNIQUENAMELEN];
   } dfpOptData;
} DFPDEVICEPACKET,
  * PDFPDEVICEPACKET;

#endif

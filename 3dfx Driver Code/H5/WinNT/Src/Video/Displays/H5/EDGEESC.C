/****************************************************************************/
/*
** Copyright© 1999 STB Systems Inc.  All rights reserved.
** Project			: Edge Tools
** Target Name		: 
** Author #1		: Paul Magee
** Author #2		: 
** Purpose			: Catches escape calls made for Edge Tools functions
** Uses Libraries	: 
** Date Started		: December 98
** Date Completed	: 
** Update History #1: 
**
** File Name              : edgeescape.c
** Source Safe Location   : \3dfx\voodoo3\dd16\; \edgetools\common\driver\
** Checked in by          : pmagee
*/
/****************************************************************************/


#include "precomp.h"
#include "edgecaps.h"
#ifdef STB_DFP_ENABLED
#include "dfpapi.h"
#endif
#include "edgedefs.h"
//???#include "infoapi.h"
#include "funcapi.h"
#include "edgeapi.h" /* for guid definitions */
#include "colorapi.h" /* for gamma and colour functions */
#include "edgeesc.h"
#ifdef MONITOR_CONTROL_ENABLED
#include "monitorapi.h"
#endif

//will know the structure of lpszInData by the escape call

int EdgeEscape(int nEscape, void * lpszInData, void * lpszOutData, PDEV * ppdev)
{


	switch (nEscape)
	{
	case STB_QUERYINTERFACE:
		{	
		//	STB_InterfaceSupported(lpszInData, lpszOutData);

			int match = 0;	/* match used to check whether a GUID has been matched */
			int i =0;		/* used as a count to check for a matching guid */
		
            if (lpszOutData == NULL)
                return STB_ESCAPE_NOT_SUPPORTED; /* error */

			/* need to search structure of guids to find if one exists */
			/* have to look up guid to return enumerated value */
			/* loop while guid is not the null guid */

			do
			{
				if STB_GUIDMATCH(STB_INTERFACE_GUIDS[i].Guid,((STB_INTERFACESUPPORTED *)lpszInData)->Guid)
				{
					((STB_INTERFACESUPPORTED *)lpszOutData)->ulResult = STB_INTERFACE_GUIDS[i].Enum;
					((STB_INTERFACESUPPORTED *)lpszOutData)->Guid = STB_INTERFACE_GUIDS[i].Guid;
					match=1;
					return STB_ESCAPE_HANDLED;
				}
				i++;
			}
			while (!(STB_GUIDMATCH(STB_INTERFACE_GUIDS[i].Guid,ZeroGuid)));
            
            ((STB_INTERFACESUPPORTED *)lpszOutData)->ulResult = 0UL;
			
			return STB_ESCAPE_NOT_SUPPORTED; /* error */
		}
      break;

	case STB_GETPROPERTY:
		{

			STB_INTERFACESUPPORTED InterfaceSupported;

			InterfaceSupported.Guid=((STB_PROPERTY *)lpszInData)->Guid;
		
			EdgeEscape(STB_QUERYINTERFACE,&InterfaceSupported, &InterfaceSupported, ppdev);
	
		/* check to see if interace supported, and only precede if it does */
		/* if interface is supported, pInterfaceSupported->ulResult will   */
		/* contain an identifier for the interface that can be switched on */
	
			if	(0!=(InterfaceSupported.ulResult))
			{
				switch (InterfaceSupported.ulResult)
				{
#ifdef STB_DFP_ENABLED
					case (STB_DFP_API):
					{
						return (DFPGetProperty(lpszInData,lpszOutData,ppdev));
					}
#endif
					case (STB_FUNCTION_API):
					{
						return(FunctionAPIGetProperty(lpszInData,lpszOutData,ppdev));
					}
#ifdef needsworkdano
					case (STB_INFO_API):
					{
						return(InfoAPIGetProperty(lpszInData,lpszOutData,ppdev));
					}
#endif //def needsworkdano
#ifdef MONITOR_CONTROL_ENABLED
					case (STB_MONITOR_API):
					{
						return(MonitorAPIGetProperty(lpszInData,lpszOutData,ppdev));
					}
#endif
#ifdef STB_COLOUR_ENABLED
					case (STB_COLOUR_API):
					{
						return(ColourAPIGetProperty(lpszInData,lpszOutData,ppdev));
					}
#endif  //def STB_COLOUR_ENABLED
					default :
					{
					}
				}
			} 
            if (lpszOutData != NULL)
			{
				((STB_PROPERTY *)lpszOutData)->Guid =  ((STB_PROPERTY *)lpszInData)->Guid;
				((STB_PROPERTY *)lpszOutData)->ulPropertyId =  ((STB_PROPERTY *)lpszInData)->ulPropertyId;
				((STB_PROPERTY *)lpszOutData)->ulResult=(DWORD)STB_INTERFACE_NOT_SUPPORTED;
			}
			return (STB_ESCAPE_NOT_SUPPORTED);
		}
      break;


	case STB_SETPROPERTY:
		{
			STB_INTERFACESUPPORTED InterfaceSupported;

			InterfaceSupported.Guid=((STB_PROPERTY *)lpszInData)->Guid;
		
			EdgeEscape(STB_QUERYINTERFACE,&InterfaceSupported, &InterfaceSupported, ppdev);
	
		/* check to see if interace supported, and only precede if it does */
		/* if interface is supported, pInterfaceSupported->ulResult will   */
		/* contain an identifier for the interface that can be switched on */

			if	(0!=(InterfaceSupported.ulResult))
			{
				switch (InterfaceSupported.ulResult)
				{
#ifdef STB_DFP_ENABLED
					case (STB_DFP_API):
					{
						return (DFPSetProperty(lpszInData,lpszOutData,ppdev));
					}
#endif /* STB_DFP_ENABLED */
					case (STB_FUNCTION_API):
					{
						return (FunctionAPISetProperty(lpszInData,lpszOutData,ppdev));
					}
#ifdef MONITOR_CONTROL_ENABLED
					case (STB_MONITOR_API):
					{
						return(MonitorAPISetProperty(lpszInData,lpszOutData));
					}
#endif
					default :
					{
					}
				}
			} 
            if (lpszOutData != NULL)
			{
				((STB_PROPERTY *)lpszOutData)->Guid =  ((STB_PROPERTY *)lpszInData)->Guid;
				((STB_PROPERTY *)lpszOutData)->ulPropertyId =  ((STB_PROPERTY *)lpszInData)->ulPropertyId;
				((STB_PROPERTY *)lpszOutData)->ulResult=(DWORD)STB_INTERFACE_NOT_SUPPORTED;
			}
			return (STB_ESCAPE_NOT_SUPPORTED);
		}
      break;

	case STB_GETGROUPPROPERTY:
		{

			STB_INTERFACESUPPORTED InterfaceSupported;

			InterfaceSupported.Guid=((STB_GROUPPROPERTY *)lpszInData)->Guid;
		
			EdgeEscape(STB_QUERYINTERFACE,&InterfaceSupported, &InterfaceSupported, ppdev);
	
		/* check to see if interace supported, and only precede if it does */
		/* if interface is supported, pInterfaceSupported->ulResult will   */
		/* contain an identifier for the interface that can be switched on */
	
			if	(0!=(InterfaceSupported.ulResult))
			{
				switch (InterfaceSupported.ulResult)
				{
#ifdef STB_DFP_ENABLED
					case (STB_DFP_API):
					{
						return (DFPGetGroupProperty(lpszInData,lpszOutData,ppdev));
					}
#endif /* STB_DFP_ENABLED */
					case (STB_FUNCTION_API):
					{
						return (FunctionAPIGetGroupProperty(lpszInData,lpszOutData,ppdev));
					}
#ifdef STB_COLOUR_ENABLED
					case (STB_COLOUR_API):
					{
						return(ColourAPIGetGroupProperty(lpszInData,lpszOutData,ppdev));
					}
#endif
					default :
					{
					}
				}
			} 
            if (lpszOutData != NULL)
			{
				((STB_GROUPPROPERTY *)lpszOutData)->Guid =  ((STB_GROUPPROPERTY *)lpszInData)->Guid;
				((STB_GROUPPROPERTY *)lpszOutData)->ulPropertyId =  ((STB_GROUPPROPERTY *)lpszInData)->ulPropertyId;
				((STB_GROUPPROPERTY *)lpszOutData)->ulResult=(DWORD)STB_INTERFACE_NOT_SUPPORTED;
			}
			return (STB_ESCAPE_NOT_SUPPORTED);
		}
      break;


	case STB_SETGROUPPROPERTY:
		{
			STB_INTERFACESUPPORTED InterfaceSupported;

			InterfaceSupported.Guid=((STB_GROUPPROPERTY *)lpszInData)->Guid;
		
			EdgeEscape(STB_QUERYINTERFACE,&InterfaceSupported, &InterfaceSupported, ppdev);
	
		/* check to see if interace supported, and only precede if it does */
		/* if interface is supported, pInterfaceSupported->ulResult will   */
		/* contain an identifier for the interface that can be switched on */

			if	(0!=(InterfaceSupported.ulResult))
			{
				switch (InterfaceSupported.ulResult)
				{
#ifdef STB_DFP_ENABLED
					case (STB_DFP_API):
					{
						return (DFPSetGroupProperty(lpszInData,lpszOutData,ppdev));
					}
#endif /* STB_DFP_ENABLED */
					case (STB_FUNCTION_API):
					{
						return (FunctionAPISetGroupProperty(lpszInData,lpszOutData,ppdev));
					}
#ifdef STB_COLOUR_ENABLED
					case (STB_COLOUR_API):
					{
					    return (ColourAPISetGroupProperty(lpszInData,lpszOutData,ppdev));
					}
#endif
					default :
					{
					}
				}
			} 
            if (lpszOutData != NULL)
			{
				((STB_GROUPPROPERTY *)lpszOutData)->Guid =  ((STB_GROUPPROPERTY *)lpszInData)->Guid;
				((STB_GROUPPROPERTY *)lpszOutData)->ulPropertyId =  ((STB_GROUPPROPERTY *)lpszInData)->ulPropertyId;
				((STB_GROUPPROPERTY *)lpszOutData)->ulResult=(DWORD)STB_INTERFACE_NOT_SUPPORTED;
			}
			return (STB_ESCAPE_NOT_SUPPORTED);
		}
      break;



	default:
		{
			return (STB_ESCAPE_NOT_SUPPORTED); /* escape is not handled */
		}
      break;
	}

}

int EdgeInit(PDEV*   ppdev)
{
#ifdef STB_DFP_ENABLED
   {
      STB_GROUPPROPERTY GroupProperty;

      GroupProperty.ulPropertyId = STB_DFP_DFPCONNECTED;

      DFPGetProperty(&GroupProperty, &GroupProperty, ppdev);
   }
#endif

#ifdef STB_TV_ENABLED
#endif

#ifdef STB_COLOUR_ENABLED
	ColourAPIInit(ppdev);
#endif

#ifdef STB_FUNCTION_ENABLED
	FunctionAPIInit(ppdev);

#endif
	return ((int)STB_ESCAPE_HANDLED);
	
}



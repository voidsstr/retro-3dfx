#ifndef _DDVPE32_H_
#define _DDVPE32_H_

//Flags for WeaveDeinterlacing
#define INTERLEAVED_VIDEO    0x1
#define BOB_INTERLEAVED      0x2
#define CAN_SIMULATE         0x4

//VideoSyncs Polarity combinations
#define VSYNC_CCIR656           0x00
#define VSYNC_AHHREF_AHVREF     0x01
#define VSYNC_AHHREF_ALVREF     0x02
#define VSYNC_ALHREF_AHVREF     0x03
#define VSYNC_ALHREF_ALVREF     0x04
#define VSYNC_BROOKTREE         0x05
#define VSYNC_PHILIPS           0x06

#define VSYNC_TYPES             5

#define H3_PORT_ID              0


#define H3_VIDEOIN_FORMATS      2     	//H3 support YUV422 & RGB565
#define H3_MAX_VID_IN_X         4096
#define H3_MAX_VID_IN_Y         2048
#define H3_VMI_DATA_BITS		8
#define H3_MAX_TIME_PER_FIELD   10000   //10 sec

#define H3_VIDEOIN_FORMATS      2       //H3 support UYVY422, YUYV422, and YUV411

#define H3_VIDEOIN_UYVY         0       //UYVY Format
#define H3_VIDEOIN_YUYV         1       //YUYV Format
#define H3_VIDEOIN_YUV411       2       //YUV411 will not be supported


#define H3_MBUS_WIDTH_BYTES		16		//Memory bus width

typedef struct
{
    RECT  rVidIn;                       //Video-in rectangle size
    GUID  gConnectTypeID;               //Videoport connect type
    DWORD dwConnectFlags;               //Misc connect type
    DWORD dwPixFmt;                     //Pixel format
    DWORD dwBpp;                        //Bytes per pixel
    BOOL  WeaveDeinterlacing;
    DWORD dwVidSrcWidth;
    DWORD dwVidSrcHeight;
    DWORD dwXScale;
    DWORD dwYScale;
    DWORD dwVPEFlags;

} VIDEO_IN_DATA;
//flags for dwVPEFlags
#define VPORT_STOP    0x0001
#define VPORT_START   0x0002

typedef struct
{
    RECT  rVidSrc;                      //Video overlay rectangle size
    RECT  rVidSrcRgn;                   //Rectangle region video source on overlay surface
    RECT  rMaxVidSrc;                   //Maximum allowed video overlay rectangle size
    DWORD dwVidCurWidth;                //Effective width of video data being stored in the buffer
    DWORD dwVidCurHeight;               //Effective height of video data being stored in the buffer
    DWORD dwVidSrcStride;               //Video source stride
    WORD  wNumbufs;                     //Number of buffers    
        

} VIDEO_SRC_DATA;


typedef struct
{
    RECT rVidDst;                       //Video destination displayed on desktop rectangle
        

} VIDEO_DST_DATA;

typedef struct                          //Bandwidth info
{
    WORD MClk;
    WORD DClk;
    WORD VidProcClk;
    WORD FSBltPerSec;
    WORD MBusWidth;
    WORD MAvgCycle;
    WORD PgBreakPlty;
    WORD DesktopBPP;
    WORD DesktopPPW;
    WORD OverlayBPP;
    WORD OverlayPPW;
    WORD BurstSize;
    DWORD MBDesktop;
    DWORD MBOverlay;
    DWORD MBTotal;

} BANDWIDTH_DATA;

typedef struct _VPORTFLIPRECORD
{
    __int64  liFlipTime;            // Time in ms that we were asked to flip */
    DWORD    dwFlipDuration;        // How long (max) the flip should take */
    BOOL     bFlipFlag;             // If we have been asked to flip */
} VPORTFLIPRECORD, *LPVPORTFLIPRECORD;

//H3 vidSerialParallel bitmasks
#define H3_VMI_CS_N_MASK					0x00000002
#define H3_VMI_OUTPUT_ENABLE_MASK			0x00000020
#define H3_VMI_DDC_ENABLE_MASK				0x00040000		//DDC bitmasks
#define H3_VMI_DDC_HOST_CLOCK_MASK			0x00080000
#define H3_VMI_DDC_HOST_DATA_MASK			0x00100000
#define H3_VMI_DDC_MON_CLOCK_MASK			0x00200000
#define H3_VMI_DDC_MON_DATA_MASK			0x00400000
#define H3_VMI_I2C_ENABLE_MASK				0x00800000		//I2C bitmasks
#define H3_VMI_I2C_CLOCK_MASK				0x01000000	
#define H3_VMI_I2C_DATA_MASK				0x02000000
#define H3_VMI_I2C_CLOCK_STATE_MASK			0x04000000
#define H3_VMI_I2C_DATA_STATE_MASK			0x08000000
#define H3_VMI_RESET_MASK					0x10000000
#define H3_VMI_GPIO1_MASK					0x20000000
#define H3_VMI_GPIO2_MASK					0x40000000
#define H3_VMI_HOST_MODE_MASK				0x80000000

#define H3_VMI_ENABLE						0x00000001
#define H3_VMI_OUTPUT_DISABLE				0x00000020
#define H3_VMI_RESET_DISABLE 				0x10000000
#define H3_VMI_HOST_MODE_B					0x80000000

//H3 vidInFormat bitmasks
#define H3_VMI_DATA_FORMAT_MASK             0x0000000e
#define H3_VMI_DEINTERLACE_MASK             0x00000010
#define H3_VMI_VSYNC_POLAR_MASK             0x00000020
#define H3_VMI_HSYNC_POLAR_MASK             0x00000040
#define H3_VMI_VACT_POLAR_MASK              0x00000080
#define H3_VMI_BUFFER_MODE_MASK             0x00000600
#define H3_VMI_TILE_SPACE_MASK              0x00000800
#define H3_VMI_MODE_MASK                    0x0000c000
#define H3_VMI_TVOUT_GENLOCK_MASK           0x00010000
#define H3_VMI_VGA_TIMING_MASK              0x00020000
#define H3_VMI_HDECIMATION_MASK             0x00100000
#define H3_VMI_VDECIMATION_MASK             0x00200000

#define H3_VMI_DATA_YVV411                  0x0000000c
#define H3_VMI_DATA_UYVY                    0x0000000c
#define H3_VMI_DATA_YUYV                    0x0000000a
#define H3_VMI_DEINTERLACE_WEAVE            0x00000010
#define H3_VMI_VSYNC_ACTIVE_LOW             0x00000020
#define H3_VMI_HSYNC_ACTIVE_LOW             0x00000040
#define H3_VMI_VACT_ACTIVE_LOW              0x00000080
#define H3_VMI_SINGLE_BUFFER                0x00000000
#define H3_VMI_DOUBLE_BUFFER                0x00000200
#define H3_VMI_TRIPLE_BUFFER                0x00000400
#define H3_VMI_MODE_VMI                     0x00004000
#define H3_VMI_MODE_TV                      0x00008000
#define H3_VMI_HDECIMATION_ENABLE           0x00100000
#define H3_VMI_VDECIMATION_ENABLE           0x00200000
#define H5_VMI_VFREF_FLUSH                  0x00400000
#define H5_VMI_VACT_FLUSH                   0x00800000

//H4 vidInFormat bitmasks
#define H4_VMI_MODE_MASK                    0x00004000
#define H4_VMI_MODE_VMI                     0x00004000

//H3 vidInStatus bitmasks
#define H3_VMI_BUFFER_MASK                  0x00000003
#define H3_VMI_FIELD_MASK                   0x00000004

//H4 vidInStatus bitmasks
#define H4_VMI_BUFFER_MASK                  0x00030000
#define H4_VMI_FIELD_MASK                   0x00040000

//h3 intrCtrl bitmasks
#define H3_VMI_INT_ENABLE                   0x00200000
#define H3_VMI_INTERRUPT					     0x00800000
#define H3_VSYNC_INT_ENABLE				     0x00000004
#define H3_VSYNC_INTERRUPT					     0x00000100

#endif

#ifndef __ATDEMO_H__
#define __ATDEMO_H__

#ifdef __WIN32__
#include <windows.h>
#include <windowsx.h>
#endif

#if macintosh
#include <Processes.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
// #include <search.h>
#include <atrender.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ATD_PERF_NONE 0
#define ATD_PERF_FPS  1
#define ATD_PERF_ALL  2 /* fps tri/sec, pixels/sec */

#define ATD_SAMPLE_FREQ 100

#ifdef __DOS32__
#define LPSTR char *
#define HINSTANCE long
#endif

#ifdef BENCHMARK
#define BENCHMARK_FPS_SCALE1  1109
#define BENCHMARK_FPS_SCALE2  13
#endif

typedef void (* AtdResetCB)(void);

/*-------------------------------------------------------------------
  Structure: AtGlobals
  Date: 10/11
  Implementor(s): mlwp
  Library: AT Demo Main
  Description:
    Describes generic application state
  Members:
    driverName          - default driver name
    driverInfo          - default driver information
    emulation           - use software emulation
    hWndMain;           - application window handle
    ctx                 - application graphics context
    caps                - graphics capabilities
    hInstApp            - application instance for dialog boxes
    bShowFrameRate      - show the frame rate
    bShowInfo           - show window information at the bottom
    bAppActive          - the app is active
    bMinimized          - app window is minimized
    bPaused             - the app is paused
    bResized            - the window has resized or some other 
                          drastic change, the entire client area 
                          should be cleared
    argc                - number of arguments passed to AppMain
    argv                - arguments passed to AppMain
    oneTMU              - force use of single TMU
    dithering           - enable/disable dithering
    graphicsEnabled     - graphics rendering is possible
    argc                - number of arguments passed to application
    argv                - arguments passed to application
    helpFile            - name of help file
    helpImage           - help image
    canvas              - default canvas
    camera              - default camera
    light               - default light
    environment         - default environment
    haveJoystick        - do we have a joystick
    haveKeyboard        - do we have a keyboard
    haveGameframe       - do we have a gameframe
    done                - program is about to terminate
    showPerf            - show performance statistics
    numFrames           - number of pre recoded frames
    curFrame            - current frame
    eventMode           - application event mode
    resetFunc           - function to reset application state
    joyFunc             - application joystick callback
    joyx                - current x for joystick
    joyy                - current y for joystick
    joyBut              - current joystick buttons
    kbFunc              - application keyboard callback
    keyMap              - current keyboard state
    showEvents          - display event numbers
    totalTime           - total execution time
    showPerformance     - show performance statistics
    fog                 - fog scene
    texture             - texture scene
    bilinear            - bilinear filter scene
    mipMap              - mipmap scene
    soundAvailable      - is sound available
    sound               - use sound
    wireframe           - draw in wireframe
    help                - show help screen
    plug                - show plug
    historyFileName     - event history file name
    repeatCount         - number of times to replay event history file
    numSamples          - number of samples taken for statistics
    stats               - current statistics
    perfStats           - buffer for statistics text
    perfStats1          - second buffer for statistics
    totalTris           - total tris rendered
    pixelsOut           - total pixels out
    plugImg             - image for plug
    fps                 - framerate
    tps                 - triangle rendering rate
    mps                 - mega pixels per second
    helpMaterial        - material used to display help
    plugMaterial        - material used to display plug
    print               - save frames to file
    sync                - sync display
    peakTexCacheMisses  - texture cache misses per tmu
    hasConsole          - application has console
    debugFile           - file to send debug messages to
  -------------------------------------------------------------------*/

typedef struct {
    char         *driverName;
    AtrDriverInfo driverInfo;
    FxBool        emulation;
    void         *hWndMain;          
    void         *hMainMenu;
    AtrContext    ctx;
    AtrDriverCaps caps;
    void         *hInstApp;     
    #if macintosh
    ProcessInfoRec  *appInfo;
    #endif
    FxBool        bShowFrameRate;  
    FxBool        bShowInfo;      
    FxBool        bAppActive;        
    FxBool        bMinimized;       
    FxBool        bPaused;           
    FxBool        bResized;         
    FxBool        bQuit;
    FxBool        graphicsEnabled;
    int           argc;
    char        **argv;
    char         *helpFile;
    AtrImg       *helpImage;
    AtrCanvas    *canvas;
    AtrCamera    *camera;
    AtrLight     *light;
    AtrEnv       *environment;
    int           auto_camera;
    FxBool        haveJoystick;
    FxBool        haveKeyboard;
    FxBool        haveGameframe;
    FxBool        done;
    FxBool        showPerf;
    FxU32         numFrames;
    FxU32         curFrame;
    FxU32         eventMode;
    AtdResetCB    resetFunc;
    AtiJoystickCB joyFunc;
    float         joyx;
    float         joyy;
    int           joyBut[4];
    AtiKeyboardCB kbFunc;
    char          keyMap[255];
    FxBool        showEvents ;
    float         totalTime ;
    FxU32         showPerformance ;
    FxBool        fog;
    FxBool        texture;
    FxBool        bilinear;
    FxBool        mipMap;
    FxBool        soundAvailable;
    FxBool        sound;
    FxBool        wireframe;
    FxBool        help;
    FxBool        plug;
    char         *historyFileName ;
    int           repeatCount ;
    int           numSamples;
    AtrStats      stats;
    char          perfStats[64];
    char          perfStats1[64];
    FxU32         totalTris;
    FxU32         pixelsOut;
    AtrImg       *plugImg;
    float         fps;
    float         tps;
    float         mps;
    AtrMaterial  *helpMaterial;
    AtrMaterial  *plugMaterial;
    FxBool        print;
    FxBool        sync;
    FxBool        oneTMU;
    FxBool        dithering;
    FxU32         peakTexCacheMisses[2];
    FxBool        fullScreen;
    FxBool        fullView;
    FxBool        hasConsole;
    FILE         *debugFile;
    char          appName[40];
} AtGlobals;

extern AtGlobals _atGlobals;     /* collection of global variables */

#ifdef BENCHMARK
typedef struct
{
  char     *filename;             /* file to dump benchmark stats to */
  int      numFrames;             /* number of frames rendered */
  clock_t  startTime;             /* time at start of frame render in ms */
  clock_t  endTime;               /* time at end of frame render in ms */
  clock_t  frameTime;             /* time to render the frame in ms */
  double   totalTime;             /* total time of benchmark in ms */
  double   totalTimeSec;          /* total time of benchmark in sec */
  FxBool   playbackOn;            /* playback path flag */
  FxBool   playbackDone;          /* playback path finished flag -- benchmark done*/
  FxBool   recordOn;              /* recording path flag */
  FxBool   dumpStats;             /* dump the benchmark stats ?? */
  double   fps;                   /* frames per second */
  double   scaledFps;             /* scaled frames per second -- benchmark value */
  int      numBenchmarks;         /* number of times to run the benchmark */
  int      currentBenchmarks;     /* current time running benchmark */
  double   averagedBenchmarks;    /* average benchmark values */
  time_t   timeStamp;             /* time stamp of the benchmark */
  char     *timeStampString;      /* characterized version of the time stamp */
} AtBenchmarkInfo;

extern AtBenchmarkInfo _atBenchmarkInfo;
#endif

#if !(macintosh)
void __cdecl Msg( LPSTR fmt, ... );
#endif

void atdSetName(char *name);

FxBool atdPause(FxBool flag);

FxBool AppInitGraphics(void);

void AppTermGraphics(void);

#if !(macintosh)
HINSTANCE atdGetInstance(void);
#endif

/* Denis. Adding user defined windows message proc */
void  atSetUserWindowProc( void (*user_message_proc)( HWND, UINT, UINT, LONG ) );

FxBool atdDrawHelp(void);
FxBool atdDrawHelpBuffer(FxU32 buffer);

void atdSetHelpFile(char *name);

void atdShamelessPlug( FxBool on );

void atdResetState(void);

void atdJoyFunc(AtiJoystickEvent *ev, FxBool preRecorded);

FxBool atdInitJoystick( void );

void atdKbdFunc(AtiKeyEvent *ev, FxBool preRecorded);

FxBool atdInitKeyboard( void );

void atdFocusFunc(FxBool gained);

void atdWinCloseFunc(void);

void atdEventInit( void );

void atdHandleEvents( void );

void atdEventClose( void );

void atdPushMaterial( AtrMaterial *m );

void atdPopMaterial( FxBool update );

void atdKeyboardFunc(AtiKeyboardCB kbFunc);

void atdJoystickFunc(AtiJoystickCB joyFunc);

void atdResetFunc(AtdResetCB resetFunc);

int atdEventLoop(void);

FxBool atdDumpFrameBuffer(char *fileName);

void atdSetFocus(void);

void atdJoystickCalibrate(void);

void atdPrintf( const char *format, ... );
char *atdGetString(char *c, int buffSize);
int atdGetChar(void);

void atdClearScreen( void );

int AppMain(int argc, char **argv);
void AppRenderScene( FxU32 displayWidth, FxU32 displayHeight);

void atdErrorCallback( FxBool fatal, const char *format, ... );

#ifdef __cplusplus
};
#endif

#endif

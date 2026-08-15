/* iqlab - 3dfx Image Quality Lab
 *
 * A tuning console for the Voodoo 3 / Voodoo 5 stack on Windows XP. Detects the
 * installed board, offers only the settings that board can actually do, renders a
 * live preview so you can see what a setting costs, and writes the choices where
 * EVERY Glide game will pick them up on its next launch.
 *
 * WHY SETTINGS PERSIST THE WAY THEY DO -- read from the vintage source, not guessed:
 *
 *   hwcGetenv()             H5/MINIHWC/MINIHWC.C:6519
 *       if (retVal = getenv(a)) return retVal;      <- environment wins
 *       ... RegOpenKey(HKEY_CURRENT_USER, regPath)  <- then HKCU
 *       ... RegOpenKey(HKEY_LOCAL_MACHINE, regPath) <- then HKLM
 *   getRegPath()            H5/MINIHWC/MINIHWC.C:6472
 *       "SYSTEM\\CurrentControlSet\\Services\\<svc>\\Device0\\glide"
 *   _grGlideInitEnvironment H5/GLIDE3/SRC/GPCI.C:1202
 *       if (_GlideRoot.initialized) return;         <- latched once per process
 *
 * So: writing a REG_SZ under HKCU ...\Device0\glide changes the setting for every
 * Glide application the next time it starts -- which is exactly "apply changes
 * that take effect when the game is re-run". Nothing can change them for a process
 * that is already running, ours included; that is a property of Glide, not a
 * limitation we chose. Hence Apply (persist for games) and Relaunch (restart our
 * own preview so you can see the result immediately) are two separate buttons.
 *
 * Build:   ./build.sh
 * Publish: python3 publish.py
 */

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define IQLAB_VERSION "0.5.0"
#define IQLAB_NAME    "3dfx Image Quality Lab"

/* ------------------------------------------------------------ card identity */

typedef enum { CARD_UNKNOWN=0, CARD_VOODOO3, CARD_V4_4500, CARD_V5_5500, CARD_V5_6000 } cardkind;

typedef struct {
    cardkind kind;
    char      name[128];      /* human name                                    */
    char      chipname[32];   /* Avenger / VSA-100                             */
    int       chips;          /* 1, 2 or 4                                     */
    int       vram_mb;
    int       has_tbuffer;    /* T-buffer FSAA: VSA-100 only                   */
    int       has_32bpp;      /* 32-bit rendering: VSA-100 only                */
    int       has_sli;        /* multi-chip                                    */
    int       max_aa;         /* 0, 2, 4 or 8                                  */
    char      driver[128];    /* the display driver actually bound             */
    char      pnp[160];
} card_t;

static card_t CARD;

static int reg_dword(HKEY root,const char*key,const char*val,DWORD*out){
    HKEY k; DWORD t=0,n=sizeof(DWORD),r; BYTE b[16];
    if(RegOpenKeyExA(root,key,0,KEY_READ,&k)!=ERROR_SUCCESS) return 0;
    n=sizeof b;
    r=RegQueryValueExA(k,val,NULL,&t,b,&n);
    RegCloseKey(k);
    if(r!=ERROR_SUCCESS||n<4) return 0;
    *out=(DWORD)(b[0]|(b[1]<<8)|(b[2]<<16)|((DWORD)b[3]<<24));
    return 1;
}
static int reg_str(HKEY root,const char*key,const char*val,char*out,int cap){
    HKEY k; DWORD t=0,n=(DWORD)cap; LONG r;
    if(RegOpenKeyExA(root,key,0,KEY_READ,&k)!=ERROR_SUCCESS) return 0;
    r=RegQueryValueExA(k,val,NULL,&t,(BYTE*)out,&n);
    RegCloseKey(k);
    if(r!=ERROR_SUCCESS) return 0;
    out[cap-1]=0; return 1;
}

/* Identify the board. Order matters: the PCI device id gives the family, then the
 * chip count separates a 5500 from a 6000. Our own miniport records the count at
 * Retro3dfxSliUnits; when a foreign driver (AmigaMerlin) is bound that value is
 * absent, so we fall back to VRAM, which tracks chip count on these boards
 * (32MB/chip: 64MB = 2 chips = 5500, 128MB = 4 chips = 6000). */
static void detect_card(void){
    DWORD d=0; char buf[128];
    const char *DEVKEY =
      "SYSTEM\\CurrentControlSet\\Services\\3dfxvs\\Device0";
    const char *CLSKEY =
      "SYSTEM\\CurrentControlSet\\Control\\Class\\"
      "{4D36E968-E325-11CE-BFC1-08002BE10318}\\0001";

    memset(&CARD,0,sizeof CARD);
    CARD.kind=CARD_UNKNOWN; CARD.chips=1; CARD.max_aa=0;
    strcpy(CARD.name,"No 3dfx board detected");
    strcpy(CARD.chipname,"-");

    /* The bound driver, not DriverDesc. DriverDesc is a leftover label from
     * whichever INF installed last -- on a box running our driver it still reads
     * "AMIGAMERLIN 3.1-R11", which is exactly the sort of confidently-wrong
     * identity string that has already cost us a day. The binding is what counts. */
    {
        char dd[128]={0}, disp[64]={0};
        reg_str(HKEY_LOCAL_MACHINE,CLSKEY,"DriverDesc",dd,sizeof dd);
        if(reg_str(HKEY_LOCAL_MACHINE,DEVKEY,"InstalledDisplayDrivers",disp,sizeof disp)
           && disp[0]){
            if(!strncmp(disp,"3dfxv5d",7) || !strncmp(disp,"3dfxv3d",7))
                _snprintf(CARD.driver,sizeof CARD.driver,"retro-3dfx  (%s.dll)",disp);
            else
                _snprintf(CARD.driver,sizeof CARD.driver,"%s  (%s.dll)",
                          dd[0]?dd:"third-party",disp);
        } else if(dd[0]) {
            _snprintf(CARD.driver,sizeof CARD.driver,"%s",dd);
        }
    }
    if(reg_str(HKEY_LOCAL_MACHINE,CLSKEY,"MatchingDeviceId",buf,sizeof buf))
        strncpy(CARD.pnp,buf,sizeof CARD.pnp-1);

    /* family from the PCI device id in MatchingDeviceId */
    {
        const char *p = CARD.pnp;
        int dev = -1;
        char *q = strstr((char*)p,"dev_");
        if(!q) q = strstr((char*)p,"DEV_");
        if(q) dev = (int)strtol(q+4,NULL,16);
        if(dev==0x0005){ CARD.kind=CARD_VOODOO3; strcpy(CARD.chipname,"Avenger"); }
        else if(dev==0x0009||dev==0x000B){ CARD.kind=CARD_V5_5500; strcpy(CARD.chipname,"VSA-100"); }
    }

    /* chip count: prefer what our miniport measured */
    if(reg_dword(HKEY_LOCAL_MACHINE,DEVKEY,"Retro3dfxSliUnits",&d) && d>=1 && d<=4)
        CARD.chips=(int)d;
    else if(reg_dword(HKEY_LOCAL_MACHINE,DEVKEY,"HardwareInformation.MemorySize",&d) && d)
        CARD.chips = (d >= 96u*1024*1024) ? 4 : (d >= 48u*1024*1024 ? 2 : 1);

    if(reg_dword(HKEY_LOCAL_MACHINE,DEVKEY,"HardwareInformation.MemorySize",&d) && d)
        CARD.vram_mb=(int)(d/(1024*1024));

    if(CARD.kind==CARD_VOODOO3){
        CARD.chips=1; CARD.has_tbuffer=0; CARD.has_32bpp=0; CARD.has_sli=0; CARD.max_aa=0;
        if(!CARD.vram_mb) CARD.vram_mb=16;
        _snprintf(CARD.name,sizeof CARD.name,"Voodoo 3  (%d MB)",CARD.vram_mb);
    } else if(CARD.kind!=CARD_UNKNOWN){
        CARD.has_tbuffer=1; CARD.has_32bpp=1;
        CARD.has_sli=(CARD.chips>1);
        if(CARD.chips>=4){ CARD.kind=CARD_V5_6000; CARD.max_aa=8;
            _snprintf(CARD.name,sizeof CARD.name,"Voodoo 5 6000  (4x VSA-100, %d MB)",
                      CARD.vram_mb?CARD.vram_mb:128); }
        else if(CARD.chips==2){ CARD.kind=CARD_V5_5500; CARD.max_aa=4;
            _snprintf(CARD.name,sizeof CARD.name,"Voodoo 5 5500  (2x VSA-100, %d MB)",
                      CARD.vram_mb?CARD.vram_mb:64); }
        else { CARD.kind=CARD_V4_4500; CARD.max_aa=2;
            _snprintf(CARD.name,sizeof CARD.name,"Voodoo 4 4500  (1x VSA-100, %d MB)",
                      CARD.vram_mb?CARD.vram_mb:32); }
    }
    CARD.name[sizeof CARD.name-1]=0;
}

/* --------------------------------------------------------------- settings */

typedef struct {
    const char *env;          /* the Glide variable / registry value name      */
    const char *label;
    const char *tip;
    int   vals[8];
    const char *names[8];
    int   n;
    int   idx;
    int   needs_tbuffer;      /* gate: VSA-100 only                            */
    int   needs_sli;          /* gate: multi-chip only                         */
    int   needs_32bpp;
    HWND  combo;
} setting_t;

static setting_t SET[] = {
{"FX_GLIDE_SWAPINTERVAL","Vertical sync","Caps frame rate to the refresh rate. Off for benchmarking.",
   {0,1,2},{"Off","On","Every 2nd"},3,1,0,0,0,NULL},
{"FX_GLIDE_AA_SAMPLE","Anti-aliasing","T-buffer full-scene AA. VSA-100 only; costs fill rate.",
   {0,2,4,8},{"Off","2x","4x","8x"},4,0,1,0,0,NULL},
{"SSTH3_ALPHADITHERMODE","Dithering","Subtract mode is the 22-bit post-filter: smoother 16-bit output.",
   {1,2,3},{"None","2x2 ordered","Subtract (22-bit)"},3,2,0,0,0,NULL},
{"FX_GLIDE_LOD_DITHER","Mip-map dithering","Dithers the band between mip levels instead of a hard seam.",
   {0,1},{"Off","On"},2,0,0,0,0,NULL},
{"FX_GLIDE_BPP","Colour depth","32-bit rendering is VSA-100 only and roughly halves fill rate.",
   {16,32},{"16-bit","32-bit"},2,0,0,0,1,NULL},
{"SSTH3_SLI_AA_CONFIGURATION","Multi-chip mode","How the chips are used: split the screen (SLI) or sample for AA.",
   {0,2,4,6},{"Single chip","2-way SLI","4-way SLI","Anti-aliasing"},4,1,0,1,0,NULL},
{"FX_GLIDE_ALLOC_COLOR","Buffering","Triple buffering smooths pacing at the cost of video memory.",
   {-1,3},{"Automatic","Triple"},2,0,0,0,0,NULL},
{"FX_GLIDE_SWAPPENDINGCOUNT","Queued frames","Lower is more responsive, higher is smoother.",
   {0,1,2,3},{"0","1","2","3"},4,1,0,0,0,NULL},
};
#define NSET ((int)(sizeof(SET)/sizeof(SET[0])))

static int lod_bias = 0;                    /* quarter-LOD units, -8..+8 in UI */
static int g_refresh = 0;                   /* 0 = leave the display mode alone   */

static int setting_supported(const setting_t*s){
    if(s->needs_tbuffer && !CARD.has_tbuffer) return 0;
    if(s->needs_sli     && !CARD.has_sli)     return 0;
    if(s->needs_32bpp   && !CARD.has_32bpp)   return 0;
    return 1;
}
static int setting_val(const setting_t*s){ return s->vals[s->idx]; }


/* ------------------------------------------------------------------- games
 *
 * Per-game tuning is real, not a pretence, and it works because of the lookup
 * order verified at the top of this file: getenv() beats both registry hives.
 * Glide has no per-title key, so a "profile" is applied by LAUNCHING the game
 * with that profile in its environment block. The game latches it at start-up
 * exactly as it would a global setting, and nothing else on the box is affected.
 *
 * Refresh rate is not a Glide knob at all -- it is a display mode -- so it is
 * applied with ChangeDisplaySettings before the game starts.
 *
 * Artwork is the game's OWN icon, pulled from its executable with ExtractIconEx.
 * Real, always correct, and never a stand-in for something we could not find.
 */

typedef struct {
    const char *name;
    const char *exe;
    const char *paths[6];      /* candidate install dirs                       */
    char        found[MAX_PATH];
    HICON       icon;
    int         present;
} game_t;

static game_t GAMES[] = {
 {"Quake III Arena","quake3.exe",
   {"C:\\Quake III Arena\\Quake3","C:\\Games\\Quake III Arena\\Quake3",
    "C:\\Program Files\\Quake III Arena",NULL}},
 {"Quake II","quake2.exe",
   {"C:\\Games\\Quake2","C:\\Quake2","C:\\Program Files\\Quake II",NULL}},
 {"Return to Castle Wolfenstein","WolfMP.exe",
   {"C:\\GOG Games\\Return to Castle Wolfenstein",
    "C:\\Program Files\\Return to Castle Wolfenstein",NULL}},
 {"Half-Life / Counter-Strike","hl.exe",
   {"C:\\Sierra\\Half-Life","C:\\Games\\Half-Life",
    "C:\\Program Files\\Counter-strike",NULL}},
 {"Unreal Tournament","UnrealTournament.exe",
   {"C:\\GOG Games\\Unreal Tournament GOTY\\System",
    "C:\\UnrealTournament\\System","C:\\Games\\UnrealTournament\\System",NULL}},
 {"Unreal Tournament 2004","UT2004.exe",
   {"C:\\UT2004\\System","C:\\Games\\UT2004\\System",NULL}},
 {"Medal of Honor: Allied Assault","MOHAA.exe",
   {"C:\\Program Files\\EA GAMES\\MOHAA",NULL}},
 {"Descent 3","descent3.exe",
   {"C:\\Games\\Descent3","C:\\Descent3",NULL}},
 {"Soldier of Fortune","SoF.exe",
   {"C:\\Games\\Soldier of Fortune","C:\\Program Files\\Soldier of Fortune",NULL}},
 {"SiN","SiN.exe",{"C:\\Games\\SiN Gold","C:\\Games\\SiN",NULL}},
 {"Battlezone","bzone.exe",{"C:\\Games\\Battlezone",NULL}},
 {"Battlefield 1942","BF1942.exe",
   {"C:\\Games\\Battlefield.1942","C:\\Program Files\\EA GAMES\\Battlefield 1942",NULL}},
};
#define NGAME ((int)(sizeof(GAMES)/sizeof(GAMES[0])))
static int g_sel_game = -1;

static void scan_games(void){
    int i,j; char p[MAX_PATH]; WIN32_FIND_DATAA fd; HANDLE h;
    for(i=0;i<NGAME;i++){
        GAMES[i].present=0; GAMES[i].icon=NULL; GAMES[i].found[0]=0;
        for(j=0;j<6 && GAMES[i].paths[j];j++){
            _snprintf(p,sizeof p,"%s\\%s",GAMES[i].paths[j],GAMES[i].exe);
            h=FindFirstFileA(p,&fd);
            if(h!=INVALID_HANDLE_VALUE){
                FindClose(h);
                strncpy(GAMES[i].found,p,MAX_PATH-1);
                GAMES[i].present=1;
                ExtractIconExA(p,0,NULL,&GAMES[i].icon,1);   /* the game's own art */
                break;
            }
        }
    }
}

/* Per-game profile: our own key, so nothing we write can confuse the driver's.
 * HKCU\Software\retro3dfx\iqlab\games\<exe> */
static void game_profile_key(int gi,char*out,int cap){
    _snprintf(out,cap,"Software\\retro3dfx\\iqlab\\games\\%s",GAMES[gi].exe);
    out[cap-1]=0;
}
static void load_game_profile(int gi){
    char key[256],v[32]; int i; HKEY k;
    game_profile_key(gi,key,sizeof key);
    if(RegOpenKeyExA(HKEY_CURRENT_USER,key,0,KEY_READ,&k)!=ERROR_SUCCESS) return;
    for(i=0;i<NSET;i++){
        DWORD t,n=sizeof v;
        if(RegQueryValueExA(k,SET[i].env,NULL,&t,(BYTE*)v,&n)==ERROR_SUCCESS){
            int want=atoi(v),j;
            for(j=0;j<SET[i].n;j++) if(SET[i].vals[j]==want){ SET[i].idx=j; break; }
        }
    }
    { DWORD t,n=sizeof v;
      if(RegQueryValueExA(k,"FX_GLIDE_LOD_BIAS",NULL,&t,(BYTE*)v,&n)==ERROR_SUCCESS)
          lod_bias=atoi(v);
      n=sizeof v;
      if(RegQueryValueExA(k,"RefreshHz",NULL,&t,(BYTE*)v,&n)==ERROR_SUCCESS)
          g_refresh=atoi(v); }
    RegCloseKey(k);
}
static int save_game_profile(int gi){
    char key[256],v[32]; int i,n=0; HKEY k; DWORD disp;
    game_profile_key(gi,key,sizeof key);
    if(RegCreateKeyExA(HKEY_CURRENT_USER,key,0,NULL,0,KEY_SET_VALUE,NULL,&k,&disp)
       !=ERROR_SUCCESS) return 0;
    for(i=0;i<NSET;i++){
        if(!setting_supported(&SET[i])) continue;
        _snprintf(v,sizeof v,"%d",setting_val(&SET[i]));
        if(RegSetValueExA(k,SET[i].env,0,REG_SZ,(const BYTE*)v,(DWORD)strlen(v)+1)
           ==ERROR_SUCCESS) n++;
    }
    _snprintf(v,sizeof v,"%d",lod_bias);
    RegSetValueExA(k,"FX_GLIDE_LOD_BIAS",0,REG_SZ,(const BYTE*)v,(DWORD)strlen(v)+1);
    _snprintf(v,sizeof v,"%d",g_refresh);
    RegSetValueExA(k,"RefreshHz",0,REG_SZ,(const BYTE*)v,(DWORD)strlen(v)+1);
    RegCloseKey(k);
    return n;
}

/* Launch the selected game WITH its profile in the environment. This is what
 * makes a per-game setting real: the child latches our values at grGlideInit,
 * and no other title is touched. */
static int launch_game(int gi,char*msg,int cap){
    char dir[MAX_PATH],*slash; STARTUPINFOA si; PROCESS_INFORMATION pi; int i; char v[32];
    if(gi<0||gi>=NGAME||!GAMES[gi].present){ _snprintf(msg,cap,"That game was not found."); return 0; }
    for(i=0;i<NSET;i++){
        if(!setting_supported(&SET[i])) continue;
        _snprintf(v,sizeof v,"%d",setting_val(&SET[i]));
        SetEnvironmentVariableA(SET[i].env,v);
    }
    _snprintf(v,sizeof v,"%d",lod_bias);
    SetEnvironmentVariableA("FX_GLIDE_LOD_BIAS",v);
    if(g_refresh>0){                       /* refresh is a display mode, not Glide */
        DEVMODEA dm; memset(&dm,0,sizeof dm); dm.dmSize=sizeof dm;
        if(EnumDisplaySettingsA(NULL,ENUM_CURRENT_SETTINGS,&dm)){
            dm.dmDisplayFrequency=g_refresh;
            dm.dmFields=DM_DISPLAYFREQUENCY|DM_PELSWIDTH|DM_PELSHEIGHT|DM_BITSPERPEL;
            ChangeDisplaySettingsA(&dm,0);
        }
    }
    strncpy(dir,GAMES[gi].found,MAX_PATH-1); dir[MAX_PATH-1]=0;
    slash=strrchr(dir,'\\'); if(slash) *slash=0;
    memset(&si,0,sizeof si); si.cb=sizeof si;
    if(CreateProcessA(GAMES[gi].found,NULL,NULL,NULL,FALSE,0,NULL,dir,&si,&pi)){
        CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
        _snprintf(msg,cap,"Launched %s with its profile.",GAMES[gi].name);
        return 1;
    }
    _snprintf(msg,cap,"Could not start %s.",GAMES[gi].name);
    return 0;
}

/* ------------------------------------------------------- persist for games */

static const char *glide_regpath(void){
    /* mirrors getRegPath(): the bound video service, then \Device0\glide */
    static char p[256];
    _snprintf(p,sizeof p,
      "SYSTEM\\CurrentControlSet\\Services\\3dfxvs\\Device0\\glide");
    return p;
}

/* Written to HKCU so it needs no admin rights and beats HKLM in hwcGetenv's
 * lookup order. Every Glide title picks these up the next time it launches. */
static int apply_to_games(char *msg,int cap){
    HKEY k; DWORD disp; int i,n=0; char v[32];
    if(RegCreateKeyExA(HKEY_CURRENT_USER,glide_regpath(),0,NULL,0,
                       KEY_SET_VALUE,NULL,&k,&disp)!=ERROR_SUCCESS){
        _snprintf(msg,cap,"Could not open the Glide settings key for writing.");
        return 0;
    }
    for(i=0;i<NSET;i++){
        if(!setting_supported(&SET[i])) continue;
        _snprintf(v,sizeof v,"%d",setting_val(&SET[i]));
        if(RegSetValueExA(k,SET[i].env,0,REG_SZ,(const BYTE*)v,(DWORD)strlen(v)+1)
           ==ERROR_SUCCESS) n++;
    }
    _snprintf(v,sizeof v,"%d",lod_bias);
    if(RegSetValueExA(k,"FX_GLIDE_LOD_BIAS",0,REG_SZ,(const BYTE*)v,(DWORD)strlen(v)+1)
       ==ERROR_SUCCESS) n++;
    RegCloseKey(k);
    _snprintf(msg,cap,
      "%d settings saved.\n\nThey apply to every Glide game the next time it is "
      "started. Games already running keep the settings they launched with -- "
      "Glide reads them once at start-up and cannot be changed after that.",n);
    return n;
}

static void clear_game_settings(void){
    HKEY k; int i;
    if(RegOpenKeyExA(HKEY_CURRENT_USER,glide_regpath(),0,KEY_SET_VALUE,&k)
       !=ERROR_SUCCESS) return;
    for(i=0;i<NSET;i++) RegDeleteValueA(k,SET[i].env);
    RegDeleteValueA(k,"FX_GLIDE_LOD_BIAS");
    RegCloseKey(k);
}

/* --------------------------------------------------------------- GL preview */

static HWND  g_main, g_gl, g_status, g_lodtrack, g_lodlabel, g_games, g_refcombo;
static HDC   g_dc; static HGLRC g_rc;
static GLuint g_tex;
static int   g_glw=520, g_glh=300, g_frame=0, g_ready=0;
static double g_fps=0; static LARGE_INTEGER g_qpf; static double g_tlast; static int g_fcount;
static char  g_renderer[128]="(not initialised)";

static double now_s(void){ LARGE_INTEGER t; QueryPerformanceCounter(&t);
    return (double)t.QuadPart/(double)g_qpf.QuadPart; }

/* mip levels are tinted so you can SEE which level the hardware picked -- with a
 * plain checker, LOD bias and mip dithering are invisible and the tool teaches
 * you nothing */
static void build_texture(void){
    static const GLubyte tint[6][3]={{255,255,255},{255,120,120},{120,255,140},
                                     {120,170,255},{255,225,120},{240,130,255}};
    int lvl,dim,x,y;
    glGenTextures(1,&g_tex); glBindTexture(GL_TEXTURE_2D,g_tex);
    for(lvl=0,dim=64; lvl<6&&dim>=1; lvl++,dim>>=1){
        GLubyte*b=(GLubyte*)malloc(dim*dim*3); if(!b) return;
        for(y=0;y<dim;y++)for(x=0;x<dim;x++){
            int c=((x>>2)+(y>>2))&1, base=c?235:55;
            GLubyte*p=b+(y*dim+x)*3;
            p[0]=(GLubyte)(base*tint[lvl][0]/255);
            p[1]=(GLubyte)(base*tint[lvl][1]/255);
            p[2]=(GLubyte)(base*tint[lvl][2]/255);
        }
        glTexImage2D(GL_TEXTURE_2D,lvl,3,dim,dim,0,GL_RGB,GL_UNSIGNED_BYTE,b);
        free(b); if(dim==1) break;
    }
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
}

static int gl_start(HWND h){
    PIXELFORMATDESCRIPTOR pfd; int pf;
    memset(&pfd,0,sizeof pfd); pfd.nSize=sizeof pfd; pfd.nVersion=1;
    pfd.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
    pfd.iPixelType=PFD_TYPE_RGBA; pfd.cColorBits=16; pfd.cDepthBits=16;
    g_dc=GetDC(h);
    pf=ChoosePixelFormat(g_dc,&pfd); if(!pf) return 0;
    if(!SetPixelFormat(g_dc,pf,&pfd)) return 0;
    g_rc=wglCreateContext(g_dc); if(!g_rc) return 0;
    wglMakeCurrent(g_dc,g_rc);
    {   const char*r;
        wglMakeCurrent(g_dc,g_rc);          /* must be current before glGetString */
        r=(const char*)glGetString(GL_RENDERER);
        if(r&&*r) strncpy(g_renderer,r,sizeof g_renderer-1);
        else      strcpy(g_renderer,"no GL renderer string");
        g_renderer[sizeof g_renderer-1]=0;
    }
    glClearColor(0.055f,0.062f,0.075f,1);
    glEnable(GL_TEXTURE_2D); glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
    build_texture();
    g_ready=1; return 1;
}

static void gl_frame(void){
    int seg; float z,zn;
    if(!g_ready) return;
    wglMakeCurrent(g_dc,g_rc);
    glViewport(0,0,g_glw,g_glh);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    { float f=(float)(1.0/tan(60.0*3.14159265/360.0)),a=(float)g_glw/(float)(g_glh?g_glh:1);
      float zf=800.f,zn2=1.f,m[16]; memset(m,0,sizeof m);
      m[0]=f/a; m[5]=f; m[10]=(zf+zn2)/(zn2-zf); m[11]=-1.f; m[14]=(2.f*zf*zn2)/(zn2-zf);
      glLoadMatrixf(m); }
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glTranslatef(0,-2.2f,-(float)fmod(g_frame*0.30,24.0));
    glBindTexture(GL_TEXTURE_2D,g_tex);
    glColor3f(1,1,1);
    for(seg=0;seg<70;seg++){
        z=-(float)seg*12.f; zn=z-12.f;
        glBegin(GL_QUADS);
          glTexCoord2f(0,0); glVertex3f(-8,0,z);
          glTexCoord2f(4,0); glVertex3f( 8,0,z);
          glTexCoord2f(4,6); glVertex3f( 8,0,zn);
          glTexCoord2f(0,6); glVertex3f(-8,0,zn);
          glTexCoord2f(0,0); glVertex3f(-8,9,zn);
          glTexCoord2f(4,0); glVertex3f( 8,9,zn);
          glTexCoord2f(4,6); glVertex3f( 8,9,z);
          glTexCoord2f(0,6); glVertex3f(-8,9,z);
          glTexCoord2f(0,0); glVertex3f(-8,0,zn);
          glTexCoord2f(6,0); glVertex3f(-8,0,z);
          glTexCoord2f(6,4); glVertex3f(-8,9,z);
          glTexCoord2f(0,4); glVertex3f(-8,9,zn);
          glTexCoord2f(0,0); glVertex3f( 8,0,z);
          glTexCoord2f(6,0); glVertex3f( 8,0,zn);
          glTexCoord2f(6,4); glVertex3f( 8,9,zn);
          glTexCoord2f(0,4); glVertex3f( 8,9,z);
        glEnd();
    }
    SwapBuffers(g_dc);
    g_frame++; g_fcount++;
    { double n=now_s();
      if(n-g_tlast>=0.5){ g_fps=g_fcount/(n-g_tlast); g_fcount=0; g_tlast=n; } }
}

/* ------------------------------------------------------------- board diagram
 * A schematic of the detected board, drawn to scale with the RIGHT NUMBER OF
 * CHIPS. It is a diagram, not a photograph -- it tells you what was detected,
 * which is the useful thing, and never pretends to be a picture of your card. */
static void draw_board(HDC dc,RECT r){
    int w=r.right-r.left, h=r.bottom-r.top;
    int bx=r.left+10, by=r.top+22, bw=w-20, bh=h-46;
    HBRUSH pcb   = CreateSolidBrush(RGB(22,58,42));
    HBRUSH chip  = CreateSolidBrush(RGB(28,30,34));
    HBRUSH ram   = CreateSolidBrush(RGB(46,50,58));
    HBRUSH gold  = CreateSolidBrush(RGB(196,158,60));
    HPEN   edge  = CreatePen(PS_SOLID,1,RGB(12,34,26));
    HPEN   chipe = CreatePen(PS_SOLID,1,RGB(70,74,84));
    HGDIOBJ op,ob;
    int i,n=CARD.chips<1?1:CARD.chips, gap, cx, cy, csz;
    RECT t;

    op=SelectObject(dc,edge); ob=SelectObject(dc,pcb);
    RoundRect(dc,bx,by,bx+bw,by+bh,6,6);

    /* AGP edge connector */
    SelectObject(dc,gold);
    Rectangle(dc,bx+bw/4,by+bh-9,bx+bw-14,by+bh);

    /* chips, laid out in a row (or 2x2 for a 6000) */
    SelectObject(dc,chipe); SelectObject(dc,chip);
    csz = (n>=4)? (bw/5) : (bw/(n+2));
    if(csz>54) csz=54;
    if(csz<18) csz=18;
    gap = csz/3;
    if(n>=4){
        int startx = bx + (bw - (2*csz+gap))/2;
        int starty = by + (bh - (2*csz+gap))/2 - 6;
        for(i=0;i<4;i++){
            cx = startx + (i%2)*(csz+gap);
            cy = starty + (i/2)*(csz+gap);
            Rectangle(dc,cx,cy,cx+csz,cy+csz);
        }
    } else {
        int startx = bx + (bw - (n*csz+(n-1)*gap))/2;
        cy = by + (bh-csz)/2 - 6;
        for(i=0;i<n;i++){
            cx = startx + i*(csz+gap);
            Rectangle(dc,cx,cy,cx+csz,cy+csz);
        }
    }
    /* memory packages along the top edge */
    SelectObject(dc,ram);
    for(i=0;i<6;i++){
        int mx=bx+12+i*((bw-24)/6);
        Rectangle(dc,mx,by+8,mx+((bw-24)/6)-6,by+20);
    }
    SelectObject(dc,op); SelectObject(dc,ob);
    DeleteObject(pcb); DeleteObject(chip); DeleteObject(ram);
    DeleteObject(gold); DeleteObject(edge); DeleteObject(chipe);

    /* caption */
    SetBkMode(dc,TRANSPARENT);
    SetTextColor(dc,RGB(150,158,170));
    t.left=r.left; t.top=r.top+2; t.right=r.right; t.bottom=r.top+20;
    DrawTextA(dc,"DETECTED BOARD",-1,&t,DT_LEFT|DT_SINGLELINE);
    SetTextColor(dc,RGB(228,234,242));
    t.top=r.bottom-22; t.bottom=r.bottom;
    DrawTextA(dc,CARD.name,-1,&t,DT_LEFT|DT_SINGLELINE|DT_END_ELLIPSIS);
}

/* --------------------------------------------------------------------- UI */

#define ID_BASE     2000
#define ID_APPLY    3001
#define ID_RELAUNCH 3002
#define ID_RESET    3003
#define ID_LOD      3004
#define ID_GAMES    3005
#define ID_LAUNCH   3006
#define ID_SAVEG    3007
#define ID_REFRESH  3008
#define ID_16BIT    3009

static HFONT g_f, g_fb, g_fs;
static HBRUSH g_bg;

static void set_status(const char*s){ if(g_status) SetWindowTextA(g_status,s); }

static void relaunch_self(void){
    char cmd[1024],part[96]; int i;
    STARTUPINFOA si; PROCESS_INFORMATION pi;
    for(i=0;i<NSET;i++){
        if(!setting_supported(&SET[i])) continue;
        _snprintf(part,sizeof part,"%d",setting_val(&SET[i]));
        SetEnvironmentVariableA(SET[i].env,part);
    }
    _snprintf(part,sizeof part,"%d",lod_bias);
    SetEnvironmentVariableA("FX_GLIDE_LOD_BIAS",part);
    SetEnvironmentVariableA("FX_GLIDE_SCREENSHOT_KEY","123");
    _snprintf(cmd,sizeof cmd,"\"%s\"",_pgmptr?_pgmptr:"iqlab.exe");
    for(i=0;i<NSET;i++){
        _snprintf(part,sizeof part," --set %s=%d",SET[i].env,SET[i].idx);
        strncat(cmd,part,sizeof(cmd)-strlen(cmd)-1);
    }
    _snprintf(part,sizeof part," --lod %d",lod_bias);
    strncat(cmd,part,sizeof(cmd)-strlen(cmd)-1);
    memset(&si,0,sizeof si); si.cb=sizeof si;
    if(CreateProcessA(NULL,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi)){
        CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
        PostMessage(g_main,WM_CLOSE,0,0);
    } else set_status("Could not restart the preview.");
}

static LRESULT CALLBACK glproc(HWND h,UINT m,WPARAM w,LPARAM l){
    if(m==WM_ERASEBKGND) return 1;
    return DefWindowProcA(h,m,w,l);
}

static LRESULT CALLBACK mainproc(HWND h,UINT m,WPARAM w,LPARAM l){
    switch(m){
    case WM_PAINT:{
        PAINTSTRUCT ps; HDC dc=BeginPaint(h,&ps); RECT rc,br;
        HBRUSH bg=CreateSolidBrush(RGB(24,27,32));
        GetClientRect(h,&rc); FillRect(dc,&rc,bg); DeleteObject(bg);
        SelectObject(dc,g_fb);
        SetBkMode(dc,TRANSPARENT);
        SetTextColor(dc,RGB(236,240,246));
        TextOutA(dc,20,14,IQLAB_NAME,(int)strlen(IQLAB_NAME));
        SelectObject(dc,g_fs);
        SetTextColor(dc,RGB(128,196,232));
        { char v[64]; int n=_snprintf(v,sizeof v,"version %s",IQLAB_VERSION);
          TextOutA(dc,22,38,v,n); }
        SetTextColor(dc,RGB(140,148,160));
        { char d[220]; int n=_snprintf(d,sizeof d,"Display driver: %s",
              CARD.driver[0]?CARD.driver:"(unknown)");
          TextOutA(dc,22,54,d,n); }
        br.left=20; br.top=64; br.right=252; br.bottom=228;
        draw_board(dc,br);
        SelectObject(dc,g_fs);
        SetTextColor(dc,RGB(150,158,170));
        TextOutA(dc,20,412,"LIVE PREVIEW",12);
        if(g_ready){
            SetTextColor(dc,RGB(176,186,198));
            { char rr[200]; int n=_snprintf(rr,sizeof rr,"%s",g_renderer);
              TextOutA(dc,20,428,rr,n); }
        } else {
            /* Not a bug in this app: the 3dfx windowed path checks the desktop
             * depth and refuses anything but 16-bit (MINIHWC/DXDRVR.C:373,
             * "Display is not in 16bpp format!"). Say so plainly and offer the fix
             * rather than showing an empty box. */
            RECT tr; tr.left=20; tr.top=250; tr.right=250; tr.bottom=406;
            SetTextColor(dc,RGB(214,168,96));
            DrawTextA(dc,
              "The preview needs a 16-bit desktop.\r\n\r\n"
              "3dfx hardware only accepts a windowed 3D surface when the desktop "
              "is 16-bit; at 32-bit the driver refuses it.\r\n\r\n"
              "Settings and per-game profiles work normally either way.",
              -1,&tr,DT_LEFT|DT_WORDBREAK);
        }
        EndPaint(h,&ps); return 0; }

    case WM_MEASUREITEM:{
        MEASUREITEMSTRUCT*mi=(MEASUREITEMSTRUCT*)l;
        if(mi->CtlID==ID_GAMES) mi->itemHeight=22;
        return TRUE; }
    case WM_DRAWITEM:{
        DRAWITEMSTRUCT*di=(DRAWITEMSTRUCT*)l;
        if(di->CtlID==ID_GAMES && (int)di->itemID<NGAME){
            game_t*g=&GAMES[di->itemID];
            int sel=(di->itemState&ODS_SELECTED)!=0;
            HBRUSH b=CreateSolidBrush(sel?RGB(38,72,104):RGB(30,34,40));
            RECT tr=di->rcItem;
            FillRect(di->hDC,&di->rcItem,b); DeleteObject(b);
            if(g->icon) DrawIconEx(di->hDC,di->rcItem.left+3,di->rcItem.top+3,
                                   g->icon,16,16,0,NULL,DI_NORMAL);
            SetBkMode(di->hDC,TRANSPARENT);
            SetTextColor(di->hDC, g->present?(sel?RGB(240,246,252):RGB(206,214,224))
                                            :RGB(104,110,120));
            tr.left+=24;
            SelectObject(di->hDC,g_f);
            DrawTextA(di->hDC,g->name,-1,&tr,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
            return TRUE;
        }
        return TRUE; }
    case WM_CTLCOLORSTATIC:
        /* A NULL_BRUSH here leaves the label transparent and the DESKTOP shows
         * through the text. Hand back a real brush in the window colour. */
        SetBkColor((HDC)w,RGB(24,27,32));
        SetTextColor((HDC)w,RGB(198,206,216));
        if(!g_bg) g_bg=CreateSolidBrush(RGB(24,27,32));
        return (LRESULT)g_bg;
    case WM_COMMAND:{
        int id=LOWORD(w),i;
        if(id>=ID_BASE&&id<ID_BASE+NSET&&HIWORD(w)==CBN_SELCHANGE){
            i=id-ID_BASE;
            SET[i].idx=(int)SendMessage(SET[i].combo,CB_GETCURSEL,0,0);
            set_status("Changed. Apply saves it for games; Restart preview shows it here.");
            return 0;
        }
        if(id==ID_GAMES && HIWORD(w)==LBN_SELCHANGE){
            int gi=(int)SendMessage(g_games,LB_GETCURSEL,0,0);
            if(gi>=0&&gi<NGAME){
                g_sel_game=gi;
                load_game_profile(gi);          /* per-title settings, if saved  */
                for(i=0;i<NSET;i++)
                    if(SET[i].combo) SendMessage(SET[i].combo,CB_SETCURSEL,SET[i].idx,0);
                SendMessage(g_lodtrack,TBM_SETPOS,TRUE,lod_bias+8);
                if(GAMES[gi].present){
                    char b[220];
                    _snprintf(b,sizeof b,"%s - %s",GAMES[gi].name,GAMES[gi].found);
                    set_status(b);
                } else set_status("Not installed on this machine.");
                InvalidateRect(h,NULL,FALSE);
            }
            return 0;
        }
        if(id==ID_REFRESH && HIWORD(w)==CBN_SELCHANGE){
            static const int HZ[5]={0,60,75,85,100};
            int k=(int)SendMessage(g_refcombo,CB_GETCURSEL,0,0);
            g_refresh=HZ[k<0?0:(k>4?4:k)];
            return 0;
        }
        if(id==ID_SAVEG){
            if(g_sel_game<0){ set_status("Pick a game first."); return 0; }
            save_game_profile(g_sel_game);
            { char b[200]; _snprintf(b,sizeof b,
                "Profile saved for %s. Use Launch so the game starts with it.",
                GAMES[g_sel_game].name); set_status(b); }
            return 0;
        }
        if(id==ID_LAUNCH){
            char msg[220];
            if(g_sel_game<0){ set_status("Pick a game first."); return 0; }
            save_game_profile(g_sel_game);
            launch_game(g_sel_game,msg,sizeof msg);
            set_status(msg);
            return 0;
        }
        if(id==ID_APPLY){ char msg[420]; apply_to_games(msg,sizeof msg);
            MessageBoxA(h,msg,"Settings saved",MB_OK|MB_ICONINFORMATION);
            set_status("Saved. Start a game to see the change."); return 0; }
        if(id==ID_RELAUNCH){ relaunch_self(); return 0; }
        if(id==ID_16BIT){
            DEVMODEA dm; memset(&dm,0,sizeof dm); dm.dmSize=sizeof dm;
            if(EnumDisplaySettingsA(NULL,ENUM_CURRENT_SETTINGS,&dm)){
                dm.dmBitsPerPel=16;
                dm.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFREQUENCY;
                if(ChangeDisplaySettingsA(&dm,0)==DISP_CHANGE_SUCCESSFUL){
                    set_status("Desktop is now 16-bit. Restart preview to enable it.");
                } else set_status("The display refused a 16-bit mode.");
            }
            return 0;
        }
        if(id==ID_RESET){ clear_game_settings();
            set_status("Game settings cleared - the driver defaults apply again.");
            return 0; }
        return 0; }
    case WM_HSCROLL:
        if((HWND)l==g_lodtrack){
            char b[64];
            lod_bias=(int)SendMessage(g_lodtrack,TBM_GETPOS,0,0)-8;
            _snprintf(b,sizeof b,"Texture sharpness: %+d",lod_bias);
            SetWindowTextA(g_lodlabel,b);
        }
        return 0;
    case WM_CLOSE: DestroyWindow(h); return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcA(h,m,w,l);
}

/* --------------------------------------------------------------------- main */

static void parse_args(int argc,char**argv){
    int i,j;
    for(i=1;i<argc;i++){
        if(!strcmp(argv[i],"--lod")&&i+1<argc) lod_bias=atoi(argv[++i]);
        else if(!strcmp(argv[i],"--set")&&i+1<argc){
            char*a=argv[++i],*eq=strchr(a,'=');
            if(eq){ *eq=0;
                for(j=0;j<NSET;j++) if(!strcmp(SET[j].env,a)){ SET[j].idx=atoi(eq+1)%SET[j].n; break; }
                *eq='='; }
        }
    }
    if(lod_bias<-8) lod_bias=-8;
    if(lod_bias>8)  lod_bias=8;
}

int main(int argc,char**argv){
    WNDCLASSA wc, wg; MSG msg; int i,y;
    INITCOMMONCONTROLSEX ic;

    QueryPerformanceFrequency(&g_qpf); g_tlast=now_s();
    detect_card();
    parse_args(argc,argv);

    ic.dwSize=sizeof ic; ic.dwICC=ICC_BAR_CLASSES|ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&ic);

    g_f =CreateFontA(-12,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,"Tahoma");
    g_fb=CreateFontA(-19,0,0,0,FW_BOLD,  0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,"Tahoma");
    g_fs=CreateFontA(-11,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,"Tahoma");

    memset(&wc,0,sizeof wc);
    wc.lpfnWndProc=mainproc; wc.hInstance=GetModuleHandleA(NULL);
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    wc.lpszClassName="iqlabmain";
    wc.hbrBackground=NULL;
    if(!RegisterClassA(&wc)) return 1;
    memset(&wg,0,sizeof wg);
    wg.style=CS_OWNDC; wg.lpfnWndProc=glproc; wg.hInstance=wc.hInstance;
    wg.lpszClassName="iqlabgl";
    RegisterClassA(&wg);

    g_main=CreateWindowExA(0,"iqlabmain",IQLAB_NAME,
        WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_VISIBLE,
        CW_USEDEFAULT,CW_USEDEFAULT,1010,580,NULL,NULL,wc.hInstance,NULL);
    if(!g_main) return 1;

    /* installed-games column */
    scan_games();
    CreateWindowA("STATIC","GAMES ON THIS MACHINE",WS_CHILD|WS_VISIBLE,
        640,50,220,16,g_main,NULL,wc.hInstance,NULL);
    g_games=CreateWindowA("LISTBOX","",
        WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_TABSTOP|LBS_NOTIFY|LBS_OWNERDRAWFIXED,
        640,70,340,268,g_main,(HMENU)ID_GAMES,wc.hInstance,NULL);
    SendMessage(g_games,WM_SETFONT,(WPARAM)g_f,TRUE);
    for(i=0;i<NGAME;i++) SendMessageA(g_games,LB_ADDSTRING,0,(LPARAM)GAMES[i].name);
    CreateWindowA("BUTTON","Save profile",WS_CHILD|WS_VISIBLE|WS_TABSTOP,
        640,346,110,26,g_main,(HMENU)ID_SAVEG,wc.hInstance,NULL);
    CreateWindowA("BUTTON","Launch with profile",WS_CHILD|WS_VISIBLE|WS_TABSTOP,
        758,346,150,26,g_main,(HMENU)ID_LAUNCH,wc.hInstance,NULL);
    SendMessage(GetDlgItem(g_main,ID_SAVEG),WM_SETFONT,(WPARAM)g_f,TRUE);
    SendMessage(GetDlgItem(g_main,ID_LAUNCH),WM_SETFONT,(WPARAM)g_f,TRUE);

    /* settings column */
    y=70;
    for(i=0;i<NSET;i++){
        HWND lab;
        int on=setting_supported(&SET[i]);
        lab=CreateWindowA("STATIC",SET[i].label,WS_CHILD|WS_VISIBLE,
            270,y,150,16,g_main,NULL,wc.hInstance,NULL);
        SendMessage(lab,WM_SETFONT,(WPARAM)g_f,TRUE);
        SET[i].combo=CreateWindowA("COMBOBOX","",
            WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|WS_TABSTOP,
            425,y-2,175,240,g_main,(HMENU)(ID_BASE+i),wc.hInstance,NULL);
        SendMessage(SET[i].combo,WM_SETFONT,(WPARAM)g_f,TRUE);
        {   int j;
            for(j=0;j<SET[i].n;j++){
                if(!strcmp(SET[i].env,"FX_GLIDE_AA_SAMPLE") && SET[i].vals[j]>CARD.max_aa)
                    continue;                       /* card-gated AA ladder    */
                if(!strcmp(SET[i].env,"SSTH3_SLI_AA_CONFIGURATION")
                   && SET[i].vals[j]==4 && CARD.chips<4)
                    continue;                       /* 4-way needs four chips  */
                SendMessageA(SET[i].combo,CB_ADDSTRING,0,(LPARAM)SET[i].names[j]);
            }
        }
        SendMessage(SET[i].combo,CB_SETCURSEL,SET[i].idx,0);
        if(!on){
            EnableWindow(SET[i].combo,FALSE); EnableWindow(lab,FALSE);
        }
        y+=30;
    }
    /* LOD bias slider */
    g_lodlabel=CreateWindowA("STATIC","Texture sharpness: +0",WS_CHILD|WS_VISIBLE,
        270,y+4,160,16,g_main,NULL,wc.hInstance,NULL);
    SendMessage(g_lodlabel,WM_SETFONT,(WPARAM)g_f,TRUE);
    g_lodtrack=CreateWindowA(TRACKBAR_CLASSA,"",WS_CHILD|WS_VISIBLE|TBS_HORZ|TBS_NOTICKS,
        425,y,175,24,g_main,(HMENU)ID_LOD,wc.hInstance,NULL);
    SendMessage(g_lodtrack,TBM_SETRANGE,TRUE,MAKELONG(0,16));
    SendMessage(g_lodtrack,TBM_SETPOS,TRUE,lod_bias+8);
    y+=34;
    {   static const char*HZN[5]={"Leave unchanged","60 Hz","75 Hz","85 Hz","100 Hz"};
        static const int  HZ[5]={0,60,75,85,100};
        HWND lab=CreateWindowA("STATIC","Refresh rate",WS_CHILD|WS_VISIBLE,
            270,y+4,150,16,g_main,NULL,wc.hInstance,NULL);
        SendMessage(lab,WM_SETFONT,(WPARAM)g_f,TRUE);
        g_refcombo=CreateWindowA("COMBOBOX","",
            WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|WS_TABSTOP,
            425,y,175,160,g_main,(HMENU)ID_REFRESH,wc.hInstance,NULL);
        SendMessage(g_refcombo,WM_SETFONT,(WPARAM)g_f,TRUE);
        for(i=0;i<5;i++) SendMessageA(g_refcombo,CB_ADDSTRING,0,(LPARAM)HZN[i]);
        { int k=0,j; for(j=0;j<5;j++) if(HZ[j]==g_refresh) k=j;
          SendMessage(g_refcombo,CB_SETCURSEL,k,0); }
    }
    y+=32;

    CreateWindowA("BUTTON","Apply to games",WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_DEFPUSHBUTTON,
        270,y,130,26,g_main,(HMENU)ID_APPLY,wc.hInstance,NULL);
    CreateWindowA("BUTTON","Restart preview",WS_CHILD|WS_VISIBLE|WS_TABSTOP,
        408,y,120,26,g_main,(HMENU)ID_RELAUNCH,wc.hInstance,NULL);
    CreateWindowA("BUTTON","Reset",WS_CHILD|WS_VISIBLE|WS_TABSTOP,
        536,y,64,26,g_main,(HMENU)ID_RESET,wc.hInstance,NULL);
    if(!g_ready)
        CreateWindowA("BUTTON","Switch desktop to 16-bit",
            WS_CHILD|WS_VISIBLE|WS_TABSTOP,20,406,232,24,
            g_main,(HMENU)ID_16BIT,wc.hInstance,NULL);
    {   HWND b; int ids[3]={ID_APPLY,ID_RELAUNCH,ID_RESET};
        for(i=0;i<3;i++){ b=GetDlgItem(g_main,ids[i]); SendMessage(b,WM_SETFONT,(WPARAM)g_f,TRUE); } }

    g_status=CreateWindowA("STATIC",
        "Apply = every Glide game.  Save profile + Launch = this game only.",
        WS_CHILD|WS_VISIBLE,20,y+40,960,18,g_main,NULL,wc.hInstance,NULL);
    SendMessage(g_status,WM_SETFONT,(WPARAM)g_fs,TRUE);

    /* live preview */
    g_gl=CreateWindowA("iqlabgl","",WS_CHILD|WS_VISIBLE|WS_BORDER,
        20,232,232,174,g_main,NULL,wc.hInstance,NULL);
    g_glw=230; g_glh=172;
    if(!gl_start(g_gl)) set_status("OpenGL preview unavailable - is a 3dfx ICD installed?");

    ShowWindow(g_main,SW_SHOW); UpdateWindow(g_main);

    for(;;){
        while(PeekMessageA(&msg,NULL,0,0,PM_REMOVE)){
            if(msg.message==WM_QUIT) goto done;
            TranslateMessage(&msg); DispatchMessageA(&msg);
        }
        gl_frame();
        {   static double lt=0; double n=now_s();
            if(n-lt>0.5){ char t[300];
                lt=n;
                _snprintf(t,sizeof t,"%s  %s   |   %.0f fps   |   %s",
                    IQLAB_NAME,IQLAB_VERSION,g_fps,g_renderer);
                SetWindowTextA(g_main,t);
            } }
        Sleep(1);
    }
done:
    if(g_rc){ wglMakeCurrent(NULL,NULL); wglDeleteContext(g_rc); }
    return 0;
}

/* -mwindows entry point. The real body stays in main() so the app can also be
 * driven from a console/CI (--set/--lod round-trip) without a second code path. */
int WINAPI WinMain(HINSTANCE h,HINSTANCE p,LPSTR cmd,int show){
    (void)h;(void)p;(void)cmd;(void)show;
    return main(__argc,__argv);
}

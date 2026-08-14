/* iqlab - live image-quality / performance lab for the 3dfx Voodoo stack on XP.
 *
 * Renders a continuous scene through the deployed OpenGL ICD and lets you change
 * image-quality settings and watch fps and pixels move together. Built to A/B two
 * driver builds (ours vs AmigaMerlin) on the same frame.
 *
 * WHY THE THREE TIERS -- verified in the vintage source, not assumed:
 *
 *   hwcGetenv()  H5/MINIHWC/MINIHWC.C:6519    if (retVal = getenv(a)) return retVal;
 *                                             ...only then HKCU, then HKLM.
 *   _grGlideInitEnvironment()  H5/GLIDE3/SRC/GPCI.C:1202
 *                                             if (_GlideRoot.initialized) return;
 *
 * So every FX_GLIDE_* / SSTH3_* knob is latched ONCE PER PROCESS, from the
 * environment, ahead of the registry. Two consequences drive this whole design:
 *   - SetEnvironmentVariable() mid-run does nothing. glide3x has its own CRT copy
 *     and has already latched. Recreating the GL context does not help either.
 *   - But CreateProcess(self) with an edited environment block costs ~2s and needs
 *     NO registry write and NO reboot. That turns the entire "restart required"
 *     tier into a keypress.
 *
 *   [live]     GL state. Next frame.
 *   [relaunch] env var + self re-exec, ~2s, full state round-tripped via argv.
 *   [reboot]   display-driver registry only. We WRITE it and say so; we never
 *              pretend it took effect.
 *
 * Determinism: the camera is driven by FRAME INDEX, never wall-clock, so two runs
 * on two drivers land on the same pixels. --shot renders N frames, dumps TGA at
 * fixed indices and exits; that is both the A/B capture and a regression gate.
 *
 * Build:  ./build.sh      (mingw cross, see the script)
 * Run:    iqlab.exe [-fs] [-w N] [-h N] [--shot N] [--out DIR]
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------ config */

#define APP_TITLE "iqlab"
#define TEX_N      6                /* mip levels in the procedural chain      */
#define TEX_DIM    64               /* base texture is TEX_DIM x TEX_DIM       */

/* Every latched knob we expose. `env` is the variable glide3x reads at init.
 * Keep FX_GLIDE_NUM_CHIPS OUT of this table on purpose: setting it to 2 on a
 * 4-chip board hard-wedges the card (open task #11), so it stays a deliberate
 * command-line thing, never a keypress. */
typedef struct {
    const char *env;                /* environment variable name               */
    const char *label;              /* what the user sees                      */
    int         vals[8];            /* value ladder                            */
    const char *names[8];           /* display name per value                  */
    int         n;                  /* ladder length                           */
    int         idx;                /* current position                        */
    char        key;                /* keyboard binding                        */
} latched_t;

static latched_t L[] = {
  { "FX_GLIDE_SWAPINTERVAL", "vsync",       {0,1,2},      {"off","on","every 2"},        3,0,'V' },
  { "FX_GLIDE_AA_SAMPLE",    "T-buffer AA", {0,2,4,8},    {"off","2x","4x","8x"},        4,0,'A' },
  { "SSTH3_ALPHADITHERMODE", "dither mode", {1,2,3},      {"none","2x2","subtract/22b"}, 3,2,'H' },
  { "FX_GLIDE_LOD_DITHER",   "mip dither",  {0,1},        {"off","on"},                  2,0,'K' },
  { "FX_GLIDE_BPP",          "render bpp",  {16,32},      {"16","32"},                   2,0,'C' },
  { "FX_GLIDE_ALLOC_COLOR",  "buffering",   {-1,3},       {"auto","triple"},             2,0,'3' },
  { "FX_GLIDE_SWAPPENDINGCOUNT","swap queue",{0,1,2,3},   {"0","1","2","3"},             4,1,'Q' },
  { "SSTH3_SLI_AA_CONFIGURATION","SLI/AA cfg",{0,2,4,6},  {"1chip","2-SLI","4-SLI","AA"},4,1,'S' },
};
#define NLATCH ((int)(sizeof(L)/sizeof(L[0])))

/* LOD bias is its own thing: signed, and unreachable from GL entirely --
 * S_CONTXT.C advertises only GL_EXT_paletted_texture / shared_texture_palette,
 * so there is no GL_EXT_texture_lod_bias to call. Environment is the only route. */
static int lod_bias = 0;            /* quarter-LOD units, -32..+31             */

/* --------------------------------------------------------------- live state */

typedef struct {
    int mag;        /* 0 nearest 1 linear                                      */
    int min;        /* index into MINF[]                                       */
    int tex;        /* texturing on/off                                        */
    int env;        /* texenv mode index                                       */
    int dither;     /* GL_DITHER                                               */
    int fog;        /* 0 off 1 linear 2 exp 3 exp2                             */
    int persp;      /* 0 fastest 1 nicest                                      */
    int depth;      /* depth test                                              */
    int blend;      /* blending                                                */
    int load;       /* scene load multiplier 1..8                              */
    int showhud;    /* in-GL HUD (off by default: in-GL text is a suspect path */
                    /* here -- see optimized/TEXT-GARBLE-*.md)                 */
} live_t;

static const GLenum MINF[6] = {
    GL_NEAREST, GL_LINEAR,
    GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST,
    GL_NEAREST_MIPMAP_LINEAR,  GL_LINEAR_MIPMAP_LINEAR
};
static const char *MINF_N[6] = {
    "nearest","linear","near/mip-near","bilinear/mip-near",
    "near/mip-lin","trilinear"
};
static const GLenum TENV[4] = { GL_MODULATE, GL_DECAL, GL_REPLACE, GL_BLEND };
static const char *TENV_N[4] = { "modulate","decal","replace","blend" };

static live_t S = { 1, 5, 1, 0, 1, 0, 1, 1, 0, 2, 0 };

/* ------------------------------------------------------------------ globals */

static HWND   g_wnd;
static HDC    g_dc;
static HGLRC  g_rc;
static int    g_w = 1024, g_h = 768, g_fs = 0;
static int    g_frame = 0;
static int    g_shot_frames = 0;          /* --shot N : headless capture mode  */
static char   g_out[MAX_PATH] = ".";
static int    g_quit = 0;
static GLuint g_tex;
static char   g_renderer[128] = "?", g_version[128] = "?";
static double g_fps = 0.0;
static LARGE_INTEGER g_qpf;

/* ------------------------------------------------------------ tiny helpers */

static double now_s(void){
    LARGE_INTEGER t; QueryPerformanceCounter(&t);
    return (double)t.QuadPart / (double)g_qpf.QuadPart;
}

static int latch_val(int i){ return L[i].vals[L[i].idx]; }

/* ------------------------------------------------------- procedural texture
 * Each mip level gets a DIFFERENT tint. That is the whole point: with a plain
 * checkerboard you cannot see which mip the hardware picked, so LOD bias, mip
 * dither and trilinear-vs-bilinear all look identical and the tool teaches you
 * nothing. Tinted levels make mip selection visible at a glance. */
static void build_texture(void){
    static const GLubyte tint[TEX_N][3] = {
        {255,255,255},{255,110,110},{110,255,110},
        {110,160,255},{255,230,110},{255,110,255}
    };
    int lvl, dim, x, y;
    glGenTextures(1,&g_tex);
    glBindTexture(GL_TEXTURE_2D,g_tex);
    for(lvl=0, dim=TEX_DIM; lvl<TEX_N && dim>=1; lvl++, dim>>=1){
        GLubyte *buf = (GLubyte*)malloc(dim*dim*3);
        if(!buf) return;
        for(y=0;y<dim;y++) for(x=0;x<dim;x++){
            int cell = ((x>>2)+(y>>2))&1;          /* checker keeps aliasing visible */
            int base = cell ? 235 : 60;
            GLubyte *p = buf + (y*dim+x)*3;
            p[0]=(GLubyte)(base*tint[lvl][0]/255);
            p[1]=(GLubyte)(base*tint[lvl][1]/255);
            p[2]=(GLubyte)(base*tint[lvl][2]/255);
        }
        glTexImage2D(GL_TEXTURE_2D,lvl,3,dim,dim,0,GL_RGB,GL_UNSIGNED_BYTE,buf);
        free(buf);
        if(dim==1) break;
    }
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
}

/* ---------------------------------------------------------- apply live state */

static void apply_live(void){
    glBindTexture(GL_TEXTURE_2D,g_tex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, S.mag?GL_LINEAR:GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, MINF[S.min]);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,(GLint)TENV[S.env]);
    if(S.tex) glEnable(GL_TEXTURE_2D); else glDisable(GL_TEXTURE_2D);
    if(S.dither) glEnable(GL_DITHER); else glDisable(GL_DITHER);
    if(S.depth){ glEnable(GL_DEPTH_TEST); glDepthMask(GL_TRUE); }
    else       { glDisable(GL_DEPTH_TEST); }
    if(S.blend){ glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); }
    else         glDisable(GL_BLEND);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, S.persp?GL_NICEST:GL_FASTEST);
    if(S.fog){
        static const GLenum fm[4]={0,GL_LINEAR,GL_EXP,GL_EXP2};
        GLfloat col[4]={0.05f,0.06f,0.09f,1.0f};
        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE,(GLint)fm[S.fog]);
        glFogfv(GL_FOG_COLOR,col);
        glFogf(GL_FOG_DENSITY,0.012f);
        glFogf(GL_FOG_START,40.0f); glFogf(GL_FOG_END,340.0f);
    } else glDisable(GL_FOG);
}

/* -------------------------------------------------------------- the scene
 * A long receding corridor. Chosen deliberately: it produces extreme texture
 * minification down the length, which is exactly where filtering, LOD bias and
 * mip dithering are visible -- and it generates real overdraw, so filter changes
 * actually move the fps number instead of being lost in CPU-bound noise. */
static void draw_quad(float x0,float y0,float z0, float x1,float y1,float z1,
                      float x2,float y2,float z2, float x3,float y3,float z3,
                      float u,float v){
    glBegin(GL_QUADS);
      glTexCoord2f(0,0); glVertex3f(x0,y0,z0);
      glTexCoord2f(u,0); glVertex3f(x1,y1,z1);
      glTexCoord2f(u,v); glVertex3f(x2,y2,z2);
      glTexCoord2f(0,v); glVertex3f(x3,y3,z3);
    glEnd();
}

static void draw_scene(void){
    int seg, pass;
    const int SEGS = 60 * S.load;          /* load knob: more geometry+overdraw */
    float z, zn;

    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    {   /* perspective without glu: 60 deg vertical fov */
        float fov=60.0f, aspect=(float)g_w/(float)(g_h?g_h:1), zn2=1.0f, zf=800.0f;
        float f = (float)(1.0/tan(fov*3.14159265/360.0));
        float m[16]={0};
        m[0]=f/aspect; m[5]=f; m[10]=(zf+zn2)/(zn2-zf); m[11]=-1.0f;
        m[14]=(2.0f*zf*zn2)/(zn2-zf);
        glLoadMatrixf(m);
    }
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    /* camera: FRAME INDEX only. Never GetTickCount -- two driver runs must land
     * on identical pixels or the A/B is worthless. */
    {
        float t   = (float)g_frame * 0.35f;
        float sway= (float)sin(g_frame*0.011) * 2.2f;
        float rise= (float)sin(g_frame*0.007) * 1.1f;
        glRotatef((float)sin(g_frame*0.005)*6.0f, 0,0,1);
        glTranslatef(-sway, -2.0f-rise, -(float)fmod(t, 24.0));
    }

    for(pass=0; pass < (S.blend?2:1); pass++){
        if(pass) glColor4f(1,1,1,0.45f); else glColor4f(1,1,1,1);
        for(seg=0; seg<SEGS; seg++){
            z  = -(float)seg*12.0f;  zn = z-12.0f;
            /* floor */
            draw_quad(-8,0,z,  8,0,z,  8,0,zn, -8,0,zn, 4.0f, 6.0f);
            /* ceiling */
            draw_quad(-8,9,zn, 8,9,zn, 8,9,z,  -8,9,z,  4.0f, 6.0f);
            /* walls */
            draw_quad(-8,0,zn,-8,0,z, -8,9,z,  -8,9,zn, 6.0f, 4.5f);
            draw_quad( 8,0,z,  8,0,zn, 8,9,zn,  8,9,z,  6.0f, 4.5f);
        }
    }
    glColor4f(1,1,1,1);
}

/* ------------------------------------------------------------- TGA capture */

static void write_tga(const char *path){
    int n = g_w*g_h*3, y;
    GLubyte *px = (GLubyte*)malloc(n);
    FILE *f;
    unsigned char hdr[18];
    if(!px) return;
    glPixelStorei(GL_PACK_ALIGNMENT,1);
    glReadBuffer(GL_FRONT);
    glReadPixels(0,0,g_w,g_h,GL_RGB,GL_UNSIGNED_BYTE,px);
    for(y=0;y<n;y+=3){ GLubyte t=px[y]; px[y]=px[y+2]; px[y+2]=t; }  /* RGB->BGR */
    memset(hdr,0,18);
    hdr[2]=2; hdr[12]=(unsigned char)(g_w&0xFF); hdr[13]=(unsigned char)(g_w>>8);
    hdr[14]=(unsigned char)(g_h&0xFF); hdr[15]=(unsigned char)(g_h>>8); hdr[16]=24;
    f=fopen(path,"wb");
    if(f){ fwrite(hdr,1,18,f); fwrite(px,1,n,f); fclose(f); }
    free(px);
}

/* Filename encodes the settings, so an A/B pair can never be silently mixed up.
 * (Glide's own hwcAAScreenShot writes glide%04d.tga with a bare counter, which
 * is why we keep this alongside it rather than relying on it for naming.) */
static void settings_tag(char *out,int cap){
    _snprintf(out,cap,"aa%d_dith%d_lod%+d_min%s_mag%s_bpp%d_vs%d",
        latch_val(1), latch_val(2), lod_bias,
        S.min==5?"tri":(S.min==3?"bi":"pt"), S.mag?"lin":"near",
        latch_val(4), latch_val(0));
    out[cap-1]=0;
}

/* ------------------------------------------------- latched tier: re-exec self
 * The only honest way to change a Glide knob, given it latches once per process.
 * We rebuild our own command line with --set NAME=VALUE pairs so the child comes
 * up in exactly the same visual state, then set the env in the child's block. */
static void relaunch(void){
    char cmd[2048], part[128], tag[128];
    STARTUPINFOA si; PROCESS_INFORMATION pi;
    int i;

    for(i=0;i<NLATCH;i++)
        SetEnvironmentVariableA(L[i].env, (_snprintf(part,sizeof part,"%d",latch_val(i)),part));
    _snprintf(part,sizeof part,"%d",lod_bias);
    SetEnvironmentVariableA("FX_GLIDE_LOD_BIAS",part);
    /* F12 -> Glide's own AA-aware, gamma-correct hardware screenshot
     * (hwcAAScreenShot, MINIHWC.C:5850, polled from grBufferSwap). It merges each
     * AA sample buffer in hardware order -- the only capture that is honest about
     * AA and gamma, and it sidesteps the GDI fullscreen garble entirely. */
    SetEnvironmentVariableA("FX_GLIDE_SCREENSHOT_KEY","123");

    _snprintf(cmd,sizeof cmd,"\"%s\" -w %d -h %d%s --out \"%s\"",
              _pgmptr?_pgmptr:"iqlab.exe", g_w, g_h, g_fs?" -fs":"", g_out);
    for(i=0;i<NLATCH;i++){
        _snprintf(part,sizeof part," --set %s=%d", L[i].env, L[i].idx);
        strncat(cmd,part,sizeof(cmd)-strlen(cmd)-1);
    }
    _snprintf(part,sizeof part," --lod %d --live %d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
        lod_bias, S.mag,S.min,S.tex,S.env,S.dither,S.fog,S.persp,S.depth,S.blend,S.load);
    strncat(cmd,part,sizeof(cmd)-strlen(cmd)-1);
    settings_tag(tag,sizeof tag);

    memset(&si,0,sizeof si); si.cb=sizeof si;
    if(CreateProcessA(NULL,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi)){
        CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
        g_quit=1;
    } else {
        MessageBoxA(g_wnd,"relaunch failed","iqlab",MB_OK|MB_ICONERROR);
    }
}

/* ------------------------------------------------------------- reboot tier
 * These live in the DISPLAY DRIVER's key, not Glide's, and genuinely cannot take
 * effect without a restart. We write them and say so plainly rather than pretend. */
static void write_reboot_knob(const char *name, DWORD val, int as_sz){
    HKEY k;
    if(RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Services\\3dfxvs\\Device0",0,KEY_SET_VALUE,&k)
        ==ERROR_SUCCESS){
        if(as_sz){ char b[32]; _snprintf(b,sizeof b,"%lu",(unsigned long)val);
                   RegSetValueExA(k,name,0,REG_SZ,(const BYTE*)b,(DWORD)strlen(b)+1); }
        else       RegSetValueExA(k,name,0,REG_DWORD,(const BYTE*)&val,sizeof val);
        RegCloseKey(k);
    }
}

/* ---------------------------------------------------------------- HUD/title */

static void update_title(void){
    char t[512], tag[128];
    settings_tag(tag,sizeof tag);
    _snprintf(t,sizeof t,
      "%s  %.1f fps  %dx%d | %s | min:%s mag:%s env:%s fog:%d dith:%d persp:%s load:%d"
      " | AA:%s vsync:%s dmode:%s lod:%+d | %s",
      APP_TITLE, g_fps, g_w, g_h, g_renderer,
      MINF_N[S.min], S.mag?"linear":"nearest", TENV_N[S.env], S.fog, S.dither,
      S.persp?"nicest":"fastest", S.load,
      L[1].names[L[1].idx], L[0].names[L[0].idx], L[2].names[L[2].idx], lod_bias, tag);
    t[sizeof t-1]=0;
    SetWindowTextA(g_wnd,t);
}

static void print_help(void){
    int i;
    printf("\n iqlab -- live 3dfx image-quality lab\n");
    printf(" GL_RENDERER : %s\n GL_VERSION  : %s\n\n",g_renderer,g_version);
    printf(" [live] next frame:\n");
    printf("   F mag filter    M min filter    T texturing   E texenv\n");
    printf("   D dither        G fog           P persp hint  Z depth\n");
    printf("   B blend         [ ] scene load  1 in-GL HUD\n\n");
    printf(" [relaunch] ~2s self re-exec, no reboot, no registry:\n");
    for(i=0;i<NLATCH;i++)
        printf("   %c %-13s = %s\n",L[i].key,L[i].label,L[i].names[L[i].idx]);
    printf("   L / ; LOD bias = %+d (quarter-LOD; unreachable from GL, env only)\n",lod_bias);
    printf("\n   ENTER apply relaunch    F9 screenshot    F12 Glide AA screenshot\n");
    printf("   R cycle resolution      ESC quit\n\n");
}

/* ------------------------------------------------------------------- input */

static LRESULT CALLBACK wndproc(HWND h,UINT m,WPARAM w,LPARAM l){
    int i;
    switch(m){
    case WM_CLOSE: case WM_DESTROY: g_quit=1; return 0;
    case WM_SIZE:  g_w=LOWORD(l); g_h=HIWORD(l); if(g_h<1)g_h=1;
                   glViewport(0,0,g_w,g_h); return 0;
    case WM_KEYDOWN:
        switch(w){
        case VK_ESCAPE: g_quit=1; return 0;
        case VK_RETURN: relaunch(); return 0;
        case VK_F9: {
            char p[MAX_PATH], tag[128];
            settings_tag(tag,sizeof tag);
            _snprintf(p,sizeof p,"%s\\iqlab_%s_f%05d.tga",g_out,tag,g_frame);
            write_tga(p); printf("shot -> %s\n",p);
            return 0; }
        case 'F': S.mag^=1; return 0;
        case 'M': S.min=(S.min+1)%6; return 0;
        case 'T': S.tex^=1; return 0;
        case 'E': S.env=(S.env+1)%4; return 0;
        case 'D': S.dither^=1; return 0;
        case 'G': S.fog=(S.fog+1)%4; return 0;
        case 'P': S.persp^=1; return 0;
        case 'Z': S.depth^=1; return 0;
        case 'B': S.blend^=1; return 0;
        case '1': S.showhud^=1; return 0;
        case VK_OEM_4: if(S.load>1) S.load--; return 0;   /* [ */
        case VK_OEM_6: if(S.load<8) S.load++; return 0;   /* ] */
        case 'L': if(lod_bias>-32) lod_bias--; return 0;
        case VK_OEM_1: if(lod_bias<31) lod_bias++; return 0; /* ; */
        default:
            for(i=0;i<NLATCH;i++)
                if((int)w==(int)L[i].key){ L[i].idx=(L[i].idx+1)%L[i].n; return 0; }
            return 0;
        }
    }
    return DefWindowProcA(h,m,w,l);
}

/* --------------------------------------------------------------- GL bring-up */

static int gl_init(void){
    PIXELFORMATDESCRIPTOR pfd;
    int pf;
    memset(&pfd,0,sizeof pfd);
    pfd.nSize=sizeof pfd; pfd.nVersion=1;
    pfd.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
    pfd.iPixelType=PFD_TYPE_RGBA;
    pfd.cColorBits=16;                 /* 16-bit: the 3dfx native path          */
    pfd.cDepthBits=16;
    pfd.iLayerType=PFD_MAIN_PLANE;
    g_dc=GetDC(g_wnd);
    pf=ChoosePixelFormat(g_dc,&pfd);
    if(!pf){ MessageBoxA(0,"ChoosePixelFormat failed","iqlab",MB_OK); return 0; }
    if(!SetPixelFormat(g_dc,pf,&pfd)){ MessageBoxA(0,"SetPixelFormat failed","iqlab",MB_OK); return 0; }
    g_rc=wglCreateContext(g_dc);
    if(!g_rc){ MessageBoxA(0,"wglCreateContext failed","iqlab",MB_OK); return 0; }
    wglMakeCurrent(g_dc,g_rc);

    {   const char *r=(const char*)glGetString(GL_RENDERER);
        const char *v=(const char*)glGetString(GL_VERSION);
        strncpy(g_renderer, r?r:"?", sizeof g_renderer-1);
        strncpy(g_version,  v?v:"?", sizeof g_version-1);
    }
    /* Refuse to benchmark Microsoft's software GL. The vintage WINUTIL.C asks for
     * PFD_GENERIC_FORMAT and would happily do exactly that -- do not inherit it. */
    if(!strstr(g_renderer,"3Dfx") && !strstr(g_renderer,"3dfx") &&
       !strstr(g_renderer,"Voodoo") && !strstr(g_renderer,"Mesa")){
        char m[320];
        _snprintf(m,sizeof m,
          "Not a 3dfx context.\n\nGL_RENDERER: %s\nGL_VERSION: %s\n\n"
          "Refusing to run: any number from here would describe Microsoft's\n"
          "software GL, not the driver under test.",g_renderer,g_version);
        MessageBoxA(0,m,"iqlab",MB_OK|MB_ICONERROR);
        return 0;
    }
    glClearColor(0.04f,0.05f,0.07f,1.0f);
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glDepthFunc(GL_LEQUAL);
    build_texture();
    return 1;
}

/* --------------------------------------------------------------------- argv */

static void parse_args(int argc,char**argv){
    int i,j;
    for(i=1;i<argc;i++){
        if(!strcmp(argv[i],"-fs")) g_fs=1;
        else if(!strcmp(argv[i],"-w") && i+1<argc) g_w=atoi(argv[++i]);
        else if(!strcmp(argv[i],"-h") && i+1<argc) g_h=atoi(argv[++i]);
        else if(!strcmp(argv[i],"--shot") && i+1<argc) g_shot_frames=atoi(argv[++i]);
        else if(!strcmp(argv[i],"--out") && i+1<argc) strncpy(g_out,argv[++i],MAX_PATH-1);
        else if(!strcmp(argv[i],"--lod") && i+1<argc) lod_bias=atoi(argv[++i]);
        else if(!strcmp(argv[i],"--live") && i+1<argc){
            sscanf(argv[++i],"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
              &S.mag,&S.min,&S.tex,&S.env,&S.dither,&S.fog,&S.persp,&S.depth,&S.blend,&S.load);
        }
        else if(!strcmp(argv[i],"--set") && i+1<argc){
            char *eq, *a=argv[++i];
            if((eq=strchr(a,'='))){
                *eq=0;
                for(j=0;j<NLATCH;j++) if(!strcmp(L[j].env,a)){ L[j].idx=atoi(eq+1)%L[j].n; break; }
                *eq='=';
            }
        }
    }
    if(S.min<0||S.min>5) S.min=5;
    if(S.env<0||S.env>3) S.env=0;
    if(S.fog<0||S.fog>3) S.fog=0;
    if(S.load<1||S.load>8) S.load=2;
}

/* --------------------------------------------------------------------- main */

int main(int argc,char**argv){
    WNDCLASSA wc;
    MSG msg;
    double t0, tlast; int fcount=0;
    DWORD style;
    RECT r;

    QueryPerformanceFrequency(&g_qpf);
    parse_args(argc,argv);

    memset(&wc,0,sizeof wc);
    wc.style=CS_OWNDC; wc.lpfnWndProc=wndproc;
    wc.hInstance=GetModuleHandleA(NULL);
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    wc.lpszClassName="iqlabwnd";
    if(!RegisterClassA(&wc)) return 1;

    style = g_fs ? (WS_POPUP|WS_VISIBLE) : (WS_OVERLAPPEDWINDOW|WS_VISIBLE);
    r.left=0; r.top=0; r.right=g_w; r.bottom=g_h;
    if(!g_fs) AdjustWindowRect(&r,style,FALSE);
    g_wnd=CreateWindowA("iqlabwnd",APP_TITLE,style,
        g_fs?0:CW_USEDEFAULT, g_fs?0:CW_USEDEFAULT,
        r.right-r.left, r.bottom-r.top, NULL,NULL,wc.hInstance,NULL);
    if(!g_wnd) return 1;

    if(g_fs){
        DEVMODEA dm; memset(&dm,0,sizeof dm); dm.dmSize=sizeof dm;
        dm.dmPelsWidth=g_w; dm.dmPelsHeight=g_h; dm.dmBitsPerPel=16;
        dm.dmFields=DM_PELSWIDTH|DM_PELSHEIGHT|DM_BITSPERPEL;
        ChangeDisplaySettingsA(&dm,CDS_FULLSCREEN);
        SetWindowPos(g_wnd,HWND_TOP,0,0,g_w,g_h,SWP_SHOWWINDOW);
    }
    if(!gl_init()) return 2;
    glViewport(0,0,g_w,g_h);
    print_help();

    t0=tlast=now_s();
    while(!g_quit){
        while(PeekMessageA(&msg,NULL,0,0,PM_REMOVE)){
            if(msg.message==WM_QUIT){ g_quit=1; break; }
            TranslateMessage(&msg); DispatchMessageA(&msg);
        }
        if(g_quit) break;

        apply_live();
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        draw_scene();
        SwapBuffers(g_dc);
        g_frame++; fcount++;

        {   double n=now_s();
            if(n-tlast >= 0.5){
                g_fps = fcount/(n-tlast);
                fcount=0; tlast=n;
                if(!g_shot_frames) update_title();
            }
            /* --shot: deterministic capture at fixed frame indices, then exit.
             * This is both the A/B image capture and the regression gate. */
            if(g_shot_frames){
                if(g_frame==g_shot_frames/2 || g_frame==g_shot_frames-1){
                    char p[MAX_PATH],tag[128];
                    settings_tag(tag,sizeof tag);
                    _snprintf(p,sizeof p,"%s\\iqlab_%s_f%05d.tga",g_out,tag,g_frame);
                    write_tga(p);
                    printf("shot -> %s\n",p);
                }
                if(g_frame>=g_shot_frames){
                    printf("RESULT iqlab fps=%.2f frames=%d renderer=%s\n",
                           fcount?g_fps:g_fps, g_frame, g_renderer);
                    g_quit=1;
                }
            }
        }
    }
    printf("RESULT iqlab fps=%.2f frames=%d renderer=%s\n",g_fps,g_frame,g_renderer);
    wglMakeCurrent(NULL,NULL);
    if(g_rc) wglDeleteContext(g_rc);
    if(g_dc) ReleaseDC(g_wnd,g_dc);
    if(g_fs) ChangeDisplaySettingsA(NULL,0);
    return 0;
}

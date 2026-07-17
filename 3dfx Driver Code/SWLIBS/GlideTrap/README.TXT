GlideTrap & Glide Play 1.0

Hector Yee
yee@3dfx.com
yhy1@cornell.edu
http://www.people.cornell.edu/pages/yhy1

Introduction

GlideTrap / Glide Play is a collection of utilities that allow you to
trap and play back glide calls on different hardware.


GlideTrap Use:

GlideTrap: copy the glide3x.dll into the directory of the program you
wish to trap call from.

WARNING! DO NOT copy this glide3x.dll over your actual glide3x.dll!

Set the following environment variables:

TRAP_GLIDE = <full path to actual glide3x.dll>         e.g. c:\3dfx\devel\cvg\bin\glide3x.dll
TRAP_DIR   = <full path to trapping dir>         e.g. d:\trap\quake\
TRAP_ECHO  = 1 - turns on screen echo            e.g. 1
TRAP_FASTCOMP = 1 - do a full compare for textures    e.g. 1
TRAP_NUM_TMU    = <number of tmus to report from grGet> e.g. 1
TRAP_TEXALIGN   = <number of bytes for texture align>   e.g. 1024

All commands are stored in the form of
<TOKEN> <PARAM1 PARAM2 ...> etc.
Commands that use pointers have been serialized.
Textures are trapped and output to files t<texture id>.<num>

Please refer to fileformat.txt for more details.

GlidePlay Use:

There are three ways to use GlidePlay.  The first is an interactive mode
where you can step through primitives and/or frames and have information
output to the screen.  To use this mode, just run GlidePlay and look
at the table of choices it presents to you.

The second way of running GlidePlay is with command line options.
In this case, run GlidePlay as:

    GlidePlay trap_dir start_frame end_frame

    (ex: GlidePlay data_dir 15 25)

The first parameter is the directory which contains all the recorded
information (this was the directory TRAP_DIR was set to when recording
was done).  The next two parameters allow you to specify the frames
of interest you wish to see.  In the above example, only frames 15
through 25 would actually update the screen.

The third way of running GlidePlay is in filtering mode.  In this case,
run GlidePlay as:

    GlidePlay -f

You will then be prompted for a directory which contains a GlideTrap
recording, and a starting and ending frame of interest.  GlidePlay will
not send any state to Glide until the starting frame of interest.  Up
until that point, GlidePlay keeps track of all state setting.  At the
start frame, GlidePlay sends all of the current state to Glide.  This
drastically reduces the amount of state setting to get to the starting
frame of interest.  GlidePlay also filters textures.  As textures get
downloaded, older textures become completely overwritten by newer
textures.  GlidePlay notices this and then is free to throw out the
over-written texture (as it cann0t possibly contribute to the frames
of interest).

GlidePlay -f and the trapping glide3x.dll can be used in conjunction
to generate very compact tracing files.  Suppose you wanted to extract
just frame 500 from a recording.  Use GlidePlay -f to filter out
everything except frame 500, and use the trapping glide3x.dll to
generate a new recording.


POSSIBLE THINGS TO CHANGE IN SOURCE

1. If vertex layout changes in fxglide.h, the following have to be updated:

    Glideres.h:

    struct GrVetrexLayoutInfo definition

    Trap.c:
    void my_grVertexLayout(FxU32 param, FxI32 offset, FxU32 mode)
    void my_outputvertex(void *p)
    void * my_inputvertex(FILE *infile)

2.  If a new function is added to glide, the following needs to be done:

    Trap.h:

    create a new token in the enum
    create function prototype for trap function
    create function prototype for play function

    Trap.c:

    implement trap function
    (output token in outfile, with parameter list following immediately.)
    implement play function
    (input parameter list and call actual glide call)


    glidetrap.h:

    add in
    typedef void ( FX_CALL *gr<FUNCTION NAME>_fpt )(<PARAM LIST>);
    void FX_CALL STUB_gr<FUNCTION NAME>(<PARAM LIST>);

    glidetrap.c:

    add in

    gr<FUNCTION NAME>_fpt GNX_gu<FUNCTION NAME>=STUB_gr<FUNCTION NAME>;

    FX_ENTRY void FX_CALL gr<FUNCTION NAME>(<PARAM LIST>);
    {
        trap_gr<FUNCTION NAME>(<PARAM LIST>);
        GNX_gr<FUNCTION NAME>(<PARAM LIST>);
    }    

    void FX_CALL STUB_gr<FUNCTION NAME>(<PARAM LIST>);
    {
        /* This function is stubbed out. */

    }

    in getproc addresses add in:

    GNX_gr<FUNCTION NAME> = (gr<FUNCTION NAME>_fpt)GetProcAddress(glideDLLInst,"_gr<FUNCTION NAME>@params");
    if(! GNX_gr<FUNCTION NAME>) return FALSE;

    GlidePlay.cpp:

    add in function handler in case statement


3.    If a new extension to glide via grProcAddress is added, change:

    Trap.h

    Add to enum token_gr<EXTENSION NAME>
    Add function prototype trap_gr<EXTENSION NAME>
    Add function prototype play_gr<EXTENSION NAME>
    
    Trap.c

    Add GrProc MYEXT_gr<EXTENSION NAME>
    Implement trap_gr<EXTENSION NAME>
    Implement play_gr<EXTENSION NAME>
    Modify grGetProcAddress

4.    If mipmap sizes change in ditex.c, change:

    glideres.h


KNOWN LIMITATIONS

1. EXTENSION MECHANISM

New extensions have to be added by modifying the source of glidetrap
and recompiling.

2. LFBLOCKS

LFB Lock failures are treated as NOOPs during playback - a grLfbLock will
not be issued on playback hardware if the trapping hardware fails the
LFBLock. No frame buffer reads/writes during lfb lock are trapped.

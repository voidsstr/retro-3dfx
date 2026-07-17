#include "ogtst.h"

void
ogLibGetEnvironment(EnvironmentPtr ep)
{
    ep->rgba_mode = ogEnvCurVisualInfo(GLX_RGBA);
    ep->index_mode = !ep->rgba_mode;
    ep->stereo = ogEnvCurVisualInfo(GLX_STEREO);
    ep->index_bits = ogEnvCurVisualInfo(GLX_BUFFER_SIZE);
    ep->rgba_bits[0] = ogEnvCurVisualInfo(GLX_RED_SIZE);
    ep->rgba_bits[1] = ogEnvCurVisualInfo(GLX_GREEN_SIZE);
    ep->rgba_bits[2] = ogEnvCurVisualInfo(GLX_BLUE_SIZE);
    ep->rgba_bits[3] = ogEnvCurVisualInfo(GLX_ALPHA_SIZE);
    ep->depth_bits = ogEnvCurVisualInfo(GLX_DEPTH_SIZE);
    ep->stencil_bits = ogEnvCurVisualInfo(GLX_STENCIL_SIZE);
    ep->accum_bits[0] = ogEnvCurVisualInfo(GLX_ACCUM_RED_SIZE);
    ep->accum_bits[1] = ogEnvCurVisualInfo(GLX_ACCUM_GREEN_SIZE);
    ep->accum_bits[2] = ogEnvCurVisualInfo(GLX_ACCUM_BLUE_SIZE);
    ep->accum_bits[3] = ogEnvCurVisualInfo(GLX_ACCUM_ALPHA_SIZE);
    ep->aux_buff = ogEnvCurVisualInfo(GLX_AUX_BUFFERS);
    glGetIntegerv(GL_DRAW_BUFFER, &ep->draw_buff);
    glGetIntegerv(GL_READ_BUFFER, &ep->read_buff);
}

void
ogLibShowEnvironment(void)
{
    Environment env;

    ogLibGetEnvironment(&env);
    ogEnvLog(OG_LINTERNALDEBUG,"\n");
    ogEnvLog(OG_LINTERNALDEBUG,"Mode: rgba=%d, index=%d stereo=%d\n", 
	env.rgba_mode, env.index_mode, env.stereo);
    ogEnvLog(OG_LINTERNALDEBUG,"Bits: index=%d, red=%d, green=%d, blue=%d, alpha=%d, depth=%d, stencil=%d\n",
        env.index_bits, env.rgba_bits[0], env.rgba_bits[1], env.rgba_bits[2],
             env.rgba_bits[3], env.depth_bits, env.stencil_bits);
    ogEnvLog(OG_LINTERNALDEBUG,"Accum: red=%d, green=%d, blue=%d, alpha=%d\n",
        env.accum_bits[0], env.accum_bits[1], env.accum_bits[2], env.accum_bits[3]);
    ogEnvLog(OG_LINTERNALDEBUG, "Buffers: draw=0x%X read=0x%X aux=%d\n", 
	env.draw_buff, env.read_buff, env.aux_buff);

}


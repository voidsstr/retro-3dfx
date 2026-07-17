#include <stdlib.h>
#include "context.h"
#include "global.h"

/* stubs to make the non-codegen build work */

#if !__GL_CODEGEN
void __glGenerateMMXFill(__GLcontext *gc, void *code)
{
}

int __glIsMmx(void)
{
  return 1;
}

void __glPickVertexShape(__GLcontext *gc)
{
}

#endif

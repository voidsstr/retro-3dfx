#include <stdlib.h>
#include "ogtst.h"

typedef struct node {
    GLint       tag;
    struct node *next;
    int		pad[2];         /* For quad word alignment */
    GLvoid      *data;
} Node, *NodePtr;

static NodePtr alloc_head = NULL;
#ifdef DEBUG_MALLOC
static GLint tag = 1;
#endif

GLvoid* ogLibMalloc(GLint size)
{
    NodePtr np;   

    np = (NodePtr) malloc(sizeof(Node) + size - 1);
    if(np == NULL)
	return NULL;
#ifdef DEBUG_MALLOC
    np->tag = tag++;
#endif
    np->next = alloc_head;
    /* add node into list */
    alloc_head = np;

    return (void *) &np->data;
}

GLvoid ogLibFree(GLvoid *data)
{
    NodePtr np, lnp; 

    /* sanity check */
    if(data == NULL) {
	ogEnvLog(OG_LALWAYS, "ogLibFree: NULL pointer passed in\n");
	return;
    }
    for (lnp = NULL, np = alloc_head; np != NULL; lnp = np, np = np->next) {
        if (data == &np->data) {
	    np->data = NULL;
            if (lnp == NULL)
                alloc_head = np->next;
            else
                lnp->next = np->next;
	    free(np);
	    return;
	}
    }
    ogEnvLog(OG_LALWAYS, "ogLibFree: try to free bad pointer (0x%x)\n", data);
}

GLvoid ogLibFreeAll(void)
{
    NodePtr np, nnp; 

    for (np = alloc_head; np != NULL; np = nnp) {
	nnp = np->next;
	free(np);
    }
    alloc_head = NULL;
#ifdef DEBUG_MALLOC
    tag = 1;
#endif
}

GLint ogLibCheckAllFreed(GLvoid)
{
    NodePtr np; 
    GLint count;
    
    for (np = alloc_head, count = 0; np != NULL; np = np->next, count++)
	ogEnvLog (OG_LALWAYS, "ogLibCheckAllFreed: 0x%x not freed\n", &np->data); 
    return count;
}

/*************************************************************
* Function to clean up any allocated memory lying around after
* test completion. Should be called at the end of each CLEANUP
* function.
*************************************************************/
GLvoid ogLibCleanMem(int level)         
{
    ogEnvLog(level, 
             "ogLibCleanMem: Freed %d additional blocks of memory\n",
             ogLibCheckAllFreed());
    ogLibFreeAll(); 
}

#ifdef DEBUG_MALLOC

GLvoid print_tags(GLvoid)
{
    NodePtr np; 

    printf("\nMemory tags: ");
    for (np = alloc_head; np != NULL; np = np->next)
	printf("%d, ",np->tag);
    printf("\nDone\n");
}

#endif

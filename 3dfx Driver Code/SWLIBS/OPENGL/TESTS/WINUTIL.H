extern BOOL initApplication( HANDLE hInstance, int nCmdShow, BOOL db );

extern getkey( void );
extern void swap(void);
extern int quit;

extern HDC hDC;

/* so we don't have to put ".f" behind all floats */
#pragma warning(disable : 4244)

/* convenient for debugging gl errors */
#define CHKERROR() \
{ \
    int e = glGetError(); \
    if (e) printf("got error %d at line %d\n", e, __LINE__); \
}



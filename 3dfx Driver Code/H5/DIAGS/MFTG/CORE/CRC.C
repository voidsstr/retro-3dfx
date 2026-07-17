/* Modified from Craig Bruce's C code */
/* http://www.cyberus.ca/~csbruce/unix/ */

#include <stdio.h>
#include <conio.h>
#include <string.h>

#include "crc.h"

int     main( int argc, char *argv[] );
unsigned long getcrc( FILE *fp );
unsigned long getcrc_buffer(void *buffer, long size);
void    crcgen( void );

unsigned long mdc_crcTable[256];

static int mdc_crcinit = 0;

void mdc_init_crc() {
  if (!mdc_crcinit) {
   mdc_crcgen();
   mdc_crcinit = 1;
  }
}

#if 0
int main( int argc, char *argv[] )
{
	char * buff, *buff2;
	unsigned long crc,crc2;

	crcgen();

	buff=strdup("The checksum for this string is: ");
	buff2=strdup("The checksum for This string is: ");
	crc=getcrc_buffer(buff,strlen(buff));
	crc2=getcrc_buffer(buff2,strlen(buff2));

	printf("%s %X\n",buff,crc);
	printf("%s %X\n",buff2,crc2);

	getch();

    return( 0 );
}
#endif


unsigned long mdc_getcrc( FILE *fp )
{
    register unsigned long crc;
    int     c;

    mdc_init_crc();
    crc = 0xFFFFFFFF;
    while( (c=getc(fp)) != EOF ) {
        crc = ((crc>>8) & 0x00FFFFFF) ^ mdc_crcTable[ (crc^c) & 0xFF ];
    }
    return( crc^0xFFFFFFFF );
}

unsigned long mdc_getcrc_buffer(void *buffer, long size)
{
	/* caution! crc is calculated backwards in this version,for speed */

    register unsigned long crc;
    int     c;

    mdc_init_crc();
    crc = 0xFFFFFFFF;
    while(size) {
		c=* ((unsigned char *) buffer +size);
		size--;
        crc = ((crc>>8) & 0x00FFFFFF) ^ mdc_crcTable[ (crc^c) & 0xFF ];
    }
    return( crc^0xFFFFFFFF );
}

void mdc_crcgen( void )
{
    unsigned long crc, poly;
    int     i, j;

    poly = 0xEDB88320L;
    for (i=0; i<256; i++) {
        crc = i;
        for (j=8; j>0; j--) {
            if (crc&1) {
                crc = (crc >> 1) ^ poly;
            } else {
                crc >>= 1;
            }
        }
        mdc_crcTable[i] = crc;
    }
}

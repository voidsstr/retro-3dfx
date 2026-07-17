/*
 * misc.h
 */


void printPciInfo(void);
int VD3I2cRead(unsigned char, FxU32, unsigned char*, FxU32, unsigned char*);
int VD3I2cWrite(unsigned char, FxU32, unsigned char*, FxU32, unsigned char*);
void VD3I2CInit(FxU32);

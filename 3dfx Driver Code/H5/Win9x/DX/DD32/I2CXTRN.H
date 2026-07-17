#ifndef _I2CXTRN_H_
#define _I2CXTRN_H_

//I2C functions:

extern void WRITEI2CREG(NT9XDEVICEDATA * ppdev, BYTE addr, BYTE subaddr, BYTE data);
extern BYTE READI2CREGCACHE(NT9XDEVICEDATA * ppdev, BYTE addr, BYTE subaddr);
extern BYTE READI2CSTATUS(NT9XDEVICEDATA * ppdev, BYTE addr);
extern void WRITEI2CDATA(NT9XDEVICEDATA * ppdev, BYTE addr, BYTE numbytes, BYTE *sptr);
extern BYTE READI2CDATA(NT9XDEVICEDATA * ppdev, BYTE addr, BYTE subaddr, BYTE numbytes, BYTE *dptr);
extern void I2C_INIT(NT9XDEVICEDATA * ppdev);
extern int 	I2CDEV_INIT(NT9XDEVICEDATA * ppdev, I2C_DEV *devptr);
extern void I2CDEV_CLOSE(NT9XDEVICEDATA * ppdev, BYTE addr);
extern void POWEROFF_7003(NT9XDEVICEDATA * ppdev, BYTE addr, BYTE subaddr, BYTE data);

#endif

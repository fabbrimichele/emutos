#ifndef CF_H
#define CF_H

#if CONF_WITH_CF

void cf_init(void);
LONG cf_ioctl(WORD dev, UWORD ctrl, void *arg);
LONG cf_rw(WORD rw, LONG sector, WORD count, UBYTE *buf, WORD dev, BOOL need_byteswap);


#endif /* CONF_WITH_CF */

#endif /* CF_H */

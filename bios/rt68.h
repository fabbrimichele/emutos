#ifndef RT68_H
#define RT68_H

#ifdef MACHINE_RT68

extern void rt68_init(void);

extern void rt68_rs232_init(void);
extern BOOL rt68_rs232_can_write(void);
extern void rt68_rs232_writeb(UBYTE b);

#endif /* MACHINE_RT68 */

#endif /* DANA_RT68 */

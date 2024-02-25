#ifndef RT68_H
#define RT68_H
#ifdef MACHINE_RT68

#define RT68_PLANES             1
#define RT68_SCREEN_WIDTH       640
#define RT68_SCREEN_HEIGHT      400


// General
extern void rt68_init(void);
extern void rt68_init_system_timer(void);
extern void rt68_int_1(void);

// Serial
extern void rt68_rs232_init(void);
extern BOOL rt68_rs232_can_write(void);
extern void rt68_rs232_writeb(UBYTE b);
extern void rt68_rs232_interrupt(UBYTE b);
// _rt68_68681_init is for driver internal use 
extern void rt68_68681_init(void);


// Screen
extern ULONG rt68_initial_vram_size(void);
extern void rt68_setphys(const UBYTE *addr);
extern const UBYTE *rt68_physbase(void);


// Debug
extern void blink_led(void);


#endif /* MACHINE_RT68 */
#endif /* RT68_H */

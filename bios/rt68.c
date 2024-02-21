#include "emutos.h"
#include "rt68.h"

#define SERDATR     *(volatile UBYTE*)0x37e801
#define SERDAT      *(volatile UBYTE*)0x37e803
#define SERDAT_TBE  4 /* bit2: Transmit Buffer */

// TODO: https://github.com/emutos/emutos/blob/9c42ef0f2c2fa9d1e217fced9da68901b2326e8b/bios/dana.c


BOOL rt68_rs232_initialized = FALSE;

extern void rt68_init(void) {
    // TODO: temporary to see as earlier as 
    //       possible the debug messages
    rt68_68681_init();
    rt68_rs232_initialized = TRUE;
}

void rt68_rs232_init(void) 
{ 
    // TODO: I should probably reimplement the intialization
    //       from ASM to C, it's not time critical
}

BOOL rt68_rs232_can_write(void) 
{ 
    if (rt68_rs232_initialized == FALSE)
        return FALSE;

    return SERDATR & SERDAT_TBE;
}

void rt68_rs232_writeb(UBYTE b) 
{ 
    if (rt68_rs232_initialized == TRUE) 
    {
        while (!rt68_rs232_can_write()) {}
        SERDAT = b;
    }
}

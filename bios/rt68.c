/*
 * rt68.c - rt68 specific functions
 *
 * Copyright (C) 2013-2024 The EmuTOS development team
 *
 * Authors:
 *  MF      Michele Fabbri
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#include "emutos.h"
#include "rt68.h"
#include "tosvars.h"

#ifdef MACHINE_RT68

/* 
TODOs: 
- Check missing functions here: https://github.com/emutos/emutos/blob/9c42ef0f2c2fa9d1e217fced9da68901b2326e8b/bios/dana.c
*/


//*******************************************
// General
//*******************************************
BOOL rt68_rs232_initialized = FALSE;

extern void rt68_init(void) 
{
    // TODO: temporary to see as earlier as 
    //       possible the debug messages
    rt68_68681_init();
    rt68_rs232_initialized = TRUE;

    phystop = (UBYTE*) (512L*1024L);
}

extern void rt68_init_system_timer(void)
{
    // TODO: rewrite it in C
    // Configure 68681 timer to interrupt with 200Hz frequency
    rt68_int_1();
} 


//*******************************************
// Serial
//*******************************************
#define SERDATR     *(volatile UBYTE*)0x37e801
#define SERDAT      *(volatile UBYTE*)0x37e803
#define SERDAT_TBE  4 /* bit2: Transmit Buffer */

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


//*******************************************
// Screen
//*******************************************

const UBYTE *rt68_screenbase;


ULONG rt68_initial_vram_size(void)
{
    return (RT68_SCREEN_WIDTH * RT68_SCREEN_HEIGHT) / 8;
}

void rt68_setphys(const UBYTE *addr)
{
    rt68_screenbase = addr;
}

const UBYTE *rt68_physbase(void)
{
    return rt68_screenbase;
}

#endif /* MACHINE_RT68 */
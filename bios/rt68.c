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

#ifdef MACHINE_RT68

/* 
TODOs: 
- Check missing functions here: https://github.com/emutos/emutos/blob/9c42ef0f2c2fa9d1e217fced9da68901b2326e8b/bios/dana.c

Notes:
- when powered on it hangs at `bios_init() end`
- but when resetted it goes further until `bootflags = 0x00`
  and then it prints the following message (but there a lot of missing things...):
```
Panic: Address Error

misc=2165 opcode=2179

addr=ffffffe1 sr=2000 pc=00381538



D0-3: ffffffd9 00000000 00381538 003842d3

D4-7: 00000009 ffff7fff 000025ff 00000000

A0-3: ffffffd9 00380f38 0038bc64 0038361c

A4-7: 00383602 003821d0 00000ffc 00000f78

 USP: ffff7fff



basepage=00003408

text=00000000 data=00000000 bss=00000000
```

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
}

extern void rt68_init_system_timer(void)
{
    // TODO: rewrite it in C
    // Configure 68681 timer to interrupt with 200Hz frequency
    rt68_int_1();
} 

/* Copied from https://github.com/emutos/emutos/pull/29/files

    As far as I understand it is required an interrupt that 
    triggeres every 5ms (200Hz), when it triggers it has to 
    call the system function vector_5ms
    There are example of calling the vector_5ms in
    - amiga2.S (assembly)
    - dana2.S (assembly)

    I am not sure how are interrupt assigned, for example
    in Dana it's interrupt 6, for amiga interrupt 2
    perhaps it depends on the hardware?
    
    I think the simplest way is to copy coldfire:
    1. configure the time in C as in the function void `coldfire_init_system_timer(void)`
    2. in that C function the assembly subroutine `_coldfire_int_61` is assigned to an 
       interrupt vector: `INTERRUPT_VECTOR(61) = coldfire_int_61;`
    3. then the assembly subroutine `_coldfire_int_61` jumps/calls to the system C function
       `.extern _vector_5ms`
*/

/* Another example
void coldfire_init_system_timer(void)
{
    // Disable the timer before configuration
    MCF_GPT1_GMS = MCF_GPT_GMS_TMS(0UL);

    // Set the interrupt handler
    INTERRUPT_VECTOR(61) = coldfire_int_61;

    // Interrupt priority.
    // Never assign the same Level and Priority to several interrupts,
    // otherwise an exception 127 will occur on RTE!
    MCF_INTC_ICR61 = MCF_INTC_ICR_IL(0x6UL) | // Level 
                     MCF_INTC_ICR_IP(0x0UL);  // Priority within the level 

    // Enable the reception of the interrupt 
    MCF_INTC_IMRH &= ~MCF_INTC_IMRH_INT_MASK61;

    // Set the frequency to 200 Hz (SDCLK / PRE / CNT) 
    MCF_GPT1_GCIR = MCF_GPT_GCIR_PRE((ULONG)cookie_mcf.sysbus_frequency) |
                    MCF_GPT_GCIR_CNT(5000UL);

    // Enable the timer 
    MCF_GPT1_GMS = MCF_GPT_GMS_CE       | // Enable 
                   MCF_GPT_GMS_SC       | // Continuous mode 
                   MCF_GPT_GMS_IEN      | // Interrupt enable 
                   MCF_GPT_GMS_TMS(4UL);  // Internal timer 
}

// Dana code

    ULONG prescale = 1;
    ULONG dc = (clock_frequency + (CLOCKS_PER_SEC/2)) / CLOCKS_PER_SEC;
    ULONG cv = dc;
    while (cv > 0xffff) {
      prescale += 1;
      cv = dc / prescale;
    }

    VEC_USER(6) = dana_int_6;

    // Assuming the system clock is at 16MHz.
    TPRER = prescale - 1;
    TCMP1 = cv;

    TCTL1 = 0x33;
    IMR &= 0xfffffd;    
*/    



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
#include "asmdefs.h"
#include "rt68asm.h"

// Initialize DUART 
// port A to 38.4k baud, 8-bits, no parity, with RTS/CTS flow control
// port B to 9.6k baud, 8-bits, no parity, with no flow control
    .globl _rt68_68681_init
_rt68_68681_init:
    move.l a0,-(sp)    
    lea     _duart,a0

    // The following three instruction are are not
    // necessary after a hardware reset to the DUART
    move.b  #0x30,DUART_CRA(a0)     // Reset port A transmitter
    move.b  #0x20,DUART_CRA(a0)     // Reset port A receiver
    move.b  #0x10,DUART_CRA(a0)     // Reset port A MR (mode register) pointer
    move.b  #0x30,DUART_CRB(a0)     // Reset port B transmitter
    move.b  #0x20,DUART_CRB(a0)     // Reset port B receiver
    move.b  #0x10,DUART_CRB(a0)     // Reset port B MR (mode register) pointer

    // Select baud rate, data format and operationg mode
    // by setting up the ACR, MR1 and MR2 registers
    move.b  #0x00,DUART_ACR(a0)     // Select baud rate set 1
    move.b  #0xCC,DUART_CSRA(a0)    // Set both Rx and Tx speed to 38.4k baud
    move.b  #0x93,DUART_MR1A(a0)    // Set port A to 8-bits, no parity,
                                    // enable RxRTS output
    move.b  #0x17,DUART_MR2A(a0)    // Select normal operating mode,
                                    // disable TxRTS, enable TxCTS, one stop bit

    move.b  #0xBB,DUART_CSRB(a0)    // Set both Rx and Tx speed to 9.6k baud
    move.b  #0x13,DUART_MR1B(a0)    // Set port A to 8-bits, no parity,
                                    // disable RxRTS output
    move.b  #0x07,DUART_MR2B(a0)    // Select normal operating mode,
                                    // disable TxRTS, disable TxCTS, one stop bit

    // Enable transmission
    move.b  #0x05,DUART_CRA(a0)     // Enable port A transmitter and receiver
    move.b  #0x05,DUART_CRB(a0)     // Enable port B transmitter and receiver

    // Set RTSA (OP0 = 1) to accept data (only required once)
    // see application note: AN405 - RTS/CTS Functions
    move.b #0x01,DUART_SOPR(a0)

    // Serial init completed
    move.l (sp)+,a0
    rts


    .globl _rt68_int_1
_rt68_int_1:
    move.l a0,-(sp)    
    lea     _duart,a0
      
    // It should not override the other bits!
    or.b    #0b0111000,DUART_ACR(a0)     // xCCCxxxx: CCC = 011 Timer Clk divided by 16

    // CTU|CTL = (68681 clock FQ)/(counter freq*16) =
    //         = 3686400Hz/(200Hz*16) = 3686400/3200 = 1152
    // CTU=4/CTL=128      
    move.b  #4,DUART_CTU(a0)        
    move.b  #128,DUART_CTL(a0)        

    move.b  #0b00001000,DUART_IMR(a0)    // xxxxCxxx: (C)ounter ready int = 1 ON
    move.b  #64,DUART_IVR(a0)           // Interrupt vector = 64
    move.l  #duart_interrupt,0x100       // Vector (address) = $100 (64 x 4 = 256 = $100)

    // Start the counter 
    // (start command is a read with A4-A1 = 1110)
    tst.b   DUART_STRC(a0)

    // Set 68000 interrupts mask to 0
    // T1 T0 S M 0 I2 I1 I0 0 0 0 X N Z V C
    // Enable all interrupts (I should enable only the required ones)
    and.w  #0b1111100011111111,SR    // set I2 I1 I0 = 000
    move.l (sp)+,a0
    rts

duart_interrupt:
    movem.l d0/a0,-(sp)                        // Save working register
    lea     _duart,a0
    btst.b  #DUART_IMR_CNTRDY,DUART_ISR(a0)    // Test Counter Ready interrupt    
    beq     duart_interrupt_no_count_int       // No Counter Ready return
    // Reset 68681 counter interrupt with stop 
    // command, in timer mode the counting continues
    // (stop command is a read with A4-A1 = 1111)
    tst.b   DUART_STPC(a0)

    pea duart_interrupt_ret
    movw sr, d0
    movw d0, -(sp)
    .extern _vector_5ms
    movl _vector_5ms, a0
    jmp (a0)

    // eventually returns to...
duart_interrupt_ret:
    lea     _duart,a0
    tst.b   DUART_STRC(a0)                  // Enable timer interrupt
duart_interrupt_no_count_int:    
    movem.l (sp)+,d0/a0                     // Restore working register
    rte

    .globl _blink_led
_blink_led:
    move.b  #1,_led1_port
    jsr     pause
    move.b  #0,_led1_port
    jsr     pause
    rts

pause:
    move.l  d1,-(sp)    
    move.l  #64000,d1
pause_loop:    
    sub.l   #1,d1
    bne     pause_loop
    move.l  (sp)+,d1
    rts        

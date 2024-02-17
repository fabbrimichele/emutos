#include "asmdefs.h"
#include "rt68asm.h"

// Initialize DUART 
// port A to 38.4k baud, 8-bits, no parity, with RTS/CTS flow control
// port B to 9.6k baud, 8-bits, no parity, with no flow control

    .globl _rt68_rs232_init
_rt68_rs232_init:
    // Debug if the function is actually called
    move.b  #1,_led1_port
    
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


    // test sending a character
_DUART_PUTCR:    
    btst.b  #DUART_TXRDY,DUART_SRA(a0)     // Test transmitter ready status
    beq     _DUART_PUTCR                   // Until transmitter ready
    move.b  #'A',DUART_TBA(a0)               // Transmit character from port A

    rts
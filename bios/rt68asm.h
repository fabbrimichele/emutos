
// Single LED port
#define _led1_port          0x37FC00

// DUART - MC68681
#define _duart              0x37E800

// REGISTERS ADDRESSES
#define DUART_MR1A          0
#define DUART_MR2A          0
#define DUART_SRA           1
#define DUART_CSRA          1
#define DUART_CRA           2
#define DUART_RBA           3
#define DUART_TBA           3
#define DUART_ACR           4
#define DUART_ISR           5
#define DUART_IMR           5
#define DUART_MR1B          8
#define DUART_MR2B          8
#define DUART_SRB           9
#define DUART_CSRB          9
#define DUART_CRB           10
#define DUART_RBB           11
#define DUART_TBB           11
#define DUART_IVR           12
#define DUART_SOPR          14
#define DUART_ROPR          15

// REGISTERS BITS
#define DUART_RXRDY         0
#define DUART_TXRDY         2

// IMR/ISR
#define DUART_IMR_TXRDYA    0
#define DUART_IMR_RXRDYA    1
#define DUART_IMR_CNTRDY    3
#define DUART_IMR_TXRDYB    4
#define DUART_IMR_RXRDYB    5
#define DUART_IMR_IN_PRT    7
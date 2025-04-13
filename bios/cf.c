
/*#define ENABLE_KDEBUG*/

#include "emutos.h"
#include "asm.h"
#include "delay.h"
#include "gemerror.h"
#include "disk.h"
#include "blkdev.h"
#include "string.h"
#include "cf.h"

#if CONF_WITH_CF

#define NOT_INITIZED    0
#define INITIALIZED     1

#define DELAY_400NS     delay_loop(delay400ns)

#define cf_interface                ((volatile struct IDE *)0x0037F800)

#define CF_READ_STATUS()            cf_interface->command
#define CF_WRITE_COMMAND(a)         cf_interface->command = a
#define CF_READ_DATA()              cf_interface->data
#define CF_WRITE_DATA(a)            cf_interface->data = a
#define CF_WRITE_SECTOR_COUNT(a)    cf_interface->sector_count = a
#define CF_WRITE_LBA0(a)            cf_interface->lba0 = a
#define CF_WRITE_LBA1(a)            cf_interface->lba1 = a
#define CF_WRITE_LBA2(a)            cf_interface->lba2 = a
#define CF_WRITE_LBA3(a)            cf_interface->lba3 = a
#define CF_WRITE_FEATURE(a)         cf_interface->features = a

#define CF_STATUS_ERR  (1 << 0)
#define CF_STATUS_DRQ  (1 << 3)
#define CF_STATUS_DF   (1 << 5)
#define CF_STATUS_DRDY (1 << 6)
#define CF_STATUS_BSY  (1 << 7)

#define CF_CMD_RESET        0x04
#define CF_CMD_READ_SECTOR  0x20
#define CF_CMD_WRITE_SECTOR 0x30
#define CF_CMD_SET_FEATURE  0xef
#define CF_CMD_IDENTIFY_DEVICE 0xec

#define CF_MODE_LBA    (1 << 6)
#define CF_DEVICE(n)   ((n) << 4)

#define CF_MASTER      0


static LONG cf_wait_for_not_busy(void);
static LONG cf_wait_for_command_ready(void);
static LONG cf_wait_for_data_ready(void);
static LONG cf_check_for_error(void);
static LONG cf_check_for_error_and_disk_fault(void);
static LONG cf_write(UBYTE cmd, ULONG sector, UWORD count, UBYTE *buffer);
static LONG cf_read(UBYTE cmd, ULONG sector, UWORD count, UBYTE *buffer);
static LONG cf_set_command(UBYTE cmd);
static void cf_set_start_count(LONG sector, UBYTE count);
static LONG cf_put_data(UBYTE *buffer, ULONG bufferlen);
static LONG cf_get_data(UBYTE *buffer, ULONG bufferlen);
static LONG cf_identify(void);

static struct {
    UWORD general_config;       /* ATAPI */
    UWORD filler01[22];
    char firmware_revision[8];  /* also in ATAPI */
    char model_number[40];      /* also in ATAPI */
    UWORD multiple_io_info;
    UWORD filler2f;
    UWORD capabilities;
    UWORD filler32[10];
    UWORD numsecs_lba28[2]; /* number of sectors for LBA28 cmds */
    UWORD filler3e[20];
    UWORD cmds_supported[3];
    UWORD filler55[15];
    UWORD maxsec_lba48[4];  /* max sector number for LBA48 cmds */
    UWORD filler68[152];
} identify;

struct IDE
{
    UBYTE data;             /* Data(R/W)            */
    UBYTE features;         /* Error(R)/Feature(W)  */
    UBYTE sector_count;     /* Sector count (R/W)   */             
    UBYTE lba0;             
    UBYTE lba1;     
    UBYTE lba2;    
    UBYTE lba3;             
    UBYTE command;          
};

static BOOL is_initialized = FALSE;
static ULONG delay400ns;

/******************************************************
 * TODO: try without debug
 * TODO: check TODOs in the code
 ******************************************************/


void cf_init(void) 
{
    KDEBUG(("CF initialization\n"));

    delay400ns = loopcount_1_msec / 2500;
    KDEBUG(("CF_WRITE_COMMAND(CF_CMD_RESET)\n"));
    CF_WRITE_COMMAND(CF_CMD_RESET);
    if (cf_wait_for_not_busy() == ERR)
        return;

    CF_WRITE_LBA3(0xe0);                    /* LBA3=0, MASTER, MODE=LBA */           
    CF_WRITE_FEATURE(0x01);                 /* 8-bit transfer mode      */
    KDEBUG(("CF_WRITE_COMMAND(CF_CMD_SET_FEATURE)\n"));
    CF_WRITE_COMMAND(CF_CMD_SET_FEATURE);
    if (cf_wait_for_not_busy() == ERR)
        return;

    if (cf_check_for_error() == ERR)
        return;

    KDEBUG(("CF initialized\n"));
    is_initialized = TRUE;
}

LONG cf_ioctl(WORD dev, UWORD ctrl, void *arg)
{
    //KDEBUG(("cf_ioctl("));
    LONG ret = ERR;
    ULONG *info = arg;
    int i;

    if (is_initialized == FALSE)
        return EUNDEV;

    switch(ctrl) {
    case GET_DISKINFO:
        //KDEBUG(("GET_DISKINFO)\n"));
        ret = cf_identify();
        if (ret >= 0) {
            info[0] = MAKE_ULONG(identify.numsecs_lba28[1], identify.numsecs_lba28[0]);
            info[1] = SECTOR_SIZE;
            ret = E_OK;
        }
        break;
    case GET_DISKNAME:
        //KDEBUG(("GET_DISKNAME)\n"));
        ret = cf_identify();
        if (ret >= 0) {
            identify.model_number[39] = 0;  /* null terminate string */

            // TODO: name bytes are swapped, aSDnsi kDSFC-B23

            /* remove right padding with spaces */
            for (i = 38; i >= 0 && identify.model_number[i] == ' '; i--)
                identify.model_number[i] = 0;

            strcpy(arg,identify.model_number);
            ret = E_OK;
        }
        break;
    case GET_MEDIACHANGE:
        //KDEBUG(("GET_MEDIACHANGE)\n"));
        ret = MEDIANOCHANGE;
        break;
    }

    //KDEBUG(("UNKOWN)\n"));
    return ret;
}

static LONG cf_identify(void)
{
    return cf_read(CF_CMD_IDENTIFY_DEVICE, 0, 1, (UBYTE *)&identify);
}

LONG cf_rw(WORD rw, LONG sector, WORD count, UBYTE *buf, WORD dev, BOOL need_byteswap)
{
    if (is_initialized == FALSE)
        return EUNDEV;

    if (need_byteswap == TRUE)
        return ERR;

    rw &= RW_RW;    /* we just care about read or write for now */

    // TODO: splits r/w by MAXSECS_PER_IO (see  EmuTOS ide.c)
    return rw ? cf_write(CF_CMD_WRITE_SECTOR, sector, count, buf)
            : cf_read(CF_CMD_READ_SECTOR, sector, count, buf);
}

static LONG cf_write(UBYTE cmd, ULONG sector, UWORD count, UBYTE *buffer)
{
    KDEBUG(("cf_write\n"));

    cf_set_start_count(sector, count);
    if (cf_set_command(cmd) == ERR)
        return ERR;

    while (count > 0)
    {
        if (cf_put_data(buffer, SECTOR_SIZE))
            return EWRITF;

        buffer += SECTOR_SIZE;
        count -= 1;
    }

    cf_wait_for_not_busy();

    return E_OK;
}

static LONG cf_read(UBYTE cmd, ULONG sector, UWORD count, UBYTE *buffer) 
{
    KDEBUG(("cf_read\n"));

    cf_set_start_count(sector, count);
    if (cf_set_command(cmd) == ERR)
        return ERR;

    while (count > 0)
    {
        if (cf_get_data(buffer, SECTOR_SIZE) != E_OK)
            return EREADF;
        
        buffer += SECTOR_SIZE;
        count -= 1;
    }

    cf_wait_for_not_busy();

    return E_OK;
}

static LONG cf_put_data(UBYTE *buffer, ULONG bufferlen)
{
    KDEBUG(("cf_put_data\n"));
    UBYTE *end = (UBYTE *)(buffer + bufferlen);

    cf_wait_for_data_ready();
    while (buffer < end) 
    {
        CF_WRITE_DATA(*buffer++);
    }

    return cf_check_for_error_and_disk_fault();
}

static LONG cf_get_data(UBYTE *buffer, ULONG bufferlen)
{
    KDEBUG(("cf_get_data\n"));
    UBYTE *end = (UBYTE *)(buffer + bufferlen);
    
    cf_wait_for_data_ready();
    while (buffer < end) 
    {
        *buffer++ = CF_READ_DATA();
    }

    return cf_check_for_error_and_disk_fault();
}

static LONG cf_set_command(UBYTE cmd)
{
    KDEBUG(("cf_set_command\n"));

    cf_wait_for_not_busy();
    CF_WRITE_COMMAND(cmd);

    cf_wait_for_command_ready();
    return cf_check_for_error();
}

static void cf_set_start_count(LONG sector, UBYTE count)
{
    KDEBUG(("cf_set_start_count\n"));

    cf_wait_for_not_busy();
    CF_WRITE_LBA0((UBYTE)sector);
    CF_WRITE_LBA1((UBYTE)(sector >> 8));
    CF_WRITE_LBA2((UBYTE)(sector >> 16));
    CF_WRITE_LBA3(CF_MODE_LBA|CF_DEVICE(CF_MASTER)|(UBYTE)((sector >> 24)&0x0f));
    CF_WRITE_SECTOR_COUNT(count);
}

static LONG cf_wait_for_data_ready(void)
{
    KDEBUG(("cf_wait_for_data_ready\n"));
    DELAY_400NS;
    // TODO: add timeout and manage errors in the caller
    while ((CF_READ_STATUS() & (CF_STATUS_DRQ | CF_STATUS_BSY)) != CF_STATUS_DRQ) {}
    return E_OK;
}

static LONG cf_wait_for_command_ready(void)
{
    KDEBUG(("cf_wait_for_command_ready\n"));
    DELAY_400NS;
    // TODO: add timeout and manage errors in the caller
    while ((CF_READ_STATUS() & (CF_STATUS_DRDY | CF_STATUS_BSY)) != CF_STATUS_DRDY) {}
    return E_OK;
}

static LONG cf_wait_for_not_busy(void)
{
    KDEBUG(("cf_wait_for_not_busy\n"));
    DELAY_400NS;
    // TODO: add timeout and manage errors in the caller
    while((CF_READ_STATUS() & CF_STATUS_BSY) != 0) {}
    return E_OK;
}

static LONG cf_check_for_error(void) {
    KDEBUG(("cf_check_for_error\n"));
    DELAY_400NS;
    UBYTE status = CF_READ_STATUS();
    if ((status & CF_STATUS_ERR) != 0)
    {
        KDEBUG(("Error, status = %x\n", status));
        return ERR;
    }
    else 
        return E_OK;
}

static LONG cf_check_for_error_and_disk_fault(void) {
    KDEBUG(("cf_check_for_error_and_disk_fault\n"));
    DELAY_400NS;
    UBYTE status = CF_READ_STATUS();
    if ((status & (CF_STATUS_ERR | CF_STATUS_DF)) != 0)
    {
        KDEBUG(("Error, status = %x\n", status));
        return ERR;
    }
    else 
        return E_OK;
}

#endif /* CONF_WITH_CF */

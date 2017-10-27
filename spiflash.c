#include "crc16.h"
#include "spiflash.h"
#include "spi.h"

#define BOARD_ID 0x0AB0B0FA
#define CLK_FREQ 72000000
#define SPIOFFSET 0

#define VERSION_HIGH 0x01
#define VERSION_LOW  0x09

/* Commands for programmer */

#define BOOTLOADER_CMD_VERSION 0x01
#define BOOTLOADER_CMD_IDENTIFY 0x02
#define BOOTLOADER_CMD_WAITREADY 0x03
#define BOOTLOADER_CMD_RAWREADWRITE 0x04
#define BOOTLOADER_CMD_ENTERPGM 0x05
#define BOOTLOADER_CMD_LEAVEPGM 0x06
#define BOOTLOADER_CMD_SSTAAIPROGRAM 0x07
#define BOOTLOADER_CMD_SETBAUDRATE 0x08
#define BOOTLOADER_CMD_PROGMEM 0x09
#define BOOTLOADER_CMD_START 0x0A
#define BOOTLOADER_MAX_CMD 0x0A

#ifdef SIMULATION
# define BOOTLOADER_WAIT_MILLIS 10
#else
# define BOOTLOADER_WAIT_MILLIS 1000
#endif

#define REPLY(X) (X|0x80)

#define HDLC_frameFlag 0x7E
#define HDLC_escapeFlag 0x7D
#define HDLC_escapeXOR 0x20

typedef void(*cmdhandler_t)(unsigned char *);

static unsigned char vstring[] = {
    VERSION_HIGH,
    VERSION_LOW,
    (SPIOFFSET>>16) & 0xff,
    (SPIOFFSET>>8) & 0xff,
    SPIOFFSET&0xff,
    0,
    0,
    0,
    (CLK_FREQ >> 24) & 0xff,
    (CLK_FREQ >> 16) & 0xff,
    (CLK_FREQ >> 8) & 0xff,
    (CLK_FREQ) & 0xff,
    (BOARD_ID >> 24) & 0xff,
    (BOARD_ID >> 16) & 0xff,
    (BOARD_ID >> 8) & 0xff,
    (BOARD_ID) & 0xff,
    0,
    0,
    0,
    0  /* Memory top, to pass on to application */
};

static unsigned bufferpos = 0;
static unsigned char buffer[256 + 32];
static unsigned char syncSeen;
static unsigned char unescaping;
static crc_t crc;
static crc_t txcrc;

void spiflash_init()
{
    syncSeen = 0;
    unescaping = 0;
    crc16__reset(&crc);
}
#if 0
int inbyte(unsigned char *dest)
{
    return -1;
}

void outbyte(unsigned char c)
{
}
#endif


static void sendByte(unsigned int i)
{
    i &= 0xff;
    crc16__append(&txcrc, i);

    if (i==HDLC_frameFlag || i==HDLC_escapeFlag) {
        outbyte(HDLC_escapeFlag);
        outbyte(i ^ HDLC_escapeXOR);
    } else
        outbyte(i);
}

static void sendBuffer(const unsigned char *buf, unsigned int size)
{
    while (size--!=0)
        sendByte(*buf++);
}

static void prepareSend()
{
    crc16__reset(&txcrc);
    outbyte(HDLC_frameFlag);
}


static void finishSend()
{
    uint16_t crc = txcrc;
    sendByte(crc>>8);
    sendByte(crc&0xff);
    outbyte(HDLC_frameFlag);
}

static void simpleReply(unsigned int r)
{
    prepareSend();
    sendByte(REPLY(r));
    finishSend();
}

static int spi_read_status()
{
    unsigned int status;

    spi_enable();
    spi_write(0x05);

    spi_write(0x00);
    status = spi_read() & 0xff;
    spi_disable();
    return status;
}

static unsigned int spi_read_id()
{
    unsigned int ret;

    spi_enable();
    spi_write32(0x9f000000);
    ret = spi_read();
    spi_disable();
    return ret;
}

static void cmd_raw_send_receive(unsigned char *buffer)
{
    unsigned int count;
    unsigned int rxcount;
    unsigned int txcount;

    // buffer[1-2] is number of TX bytes
    // buffer[3-4] is number of RX bytes
    // buffer[5..] is data to transmit.

    // NOTE - buffer will be overwritten in read.

    spi_enable();
    txcount = buffer[1];
    txcount<<=8;
    txcount += buffer[2];

    for (count=0; count<txcount; count++) {
        spi_write(buffer[5+count]);
    }
    rxcount = buffer[3];
    rxcount<<=8;
    rxcount += buffer[4];
    // Now, receive and write buffer
    for(count=0;count <rxcount;count++) {
        spi_write(0x00);
        buffer[count] = spi_read();
    }
    spi_disable();

    // Send back
    prepareSend();
    sendByte(REPLY(BOOTLOADER_CMD_RAWREADWRITE));
    sendByte(rxcount>>8);
    sendByte(rxcount);
    for(count=0;count<rxcount;count++) {
        sendByte(buffer[count]);
    }
    finishSend();
}


static void cmd_set_baudrate(unsigned char *buffer)
{
#if 0
    unsigned int bsel = buffer[1];
    bsel<<=8;
    bsel |= buffer[2];
    bsel<<=8;
    bsel |= buffer[3];
    bsel<<=8;
    bsel |= buffer[4];

    simpleReply(BOOTLOADER_CMD_SETBAUDRATE);

    // We ought to wait here, to ensure output is properly drained.
    outbyte(0xff);
    while ((UARTCTL&0x2)==2);

    UARTCTL = bsel | BIT(UARTEN);
#endif
}


static void cmd_waitready(unsigned char *buffer)
{
    int status;

    do {
        status = spi_read_status();
    } while (status & 1);

    prepareSend();
    sendByte(REPLY(BOOTLOADER_CMD_WAITREADY));
    sendByte(status);
    finishSend();
}


static void cmd_version(unsigned char *buffer)
{
    prepareSend();
    sendByte(REPLY(BOOTLOADER_CMD_VERSION));

    sendBuffer(vstring,sizeof(vstring));
    finishSend();
}

static void cmd_identify(unsigned char *buffer)
{
    int id;
    unsigned flash_id;
    prepareSend();
    sendByte(REPLY(BOOTLOADER_CMD_IDENTIFY));
    flash_id = spi_read_id();
    sendByte(flash_id>>16);
    sendByte(flash_id>>8);
    sendByte(flash_id);
    id = spi_read_status();
    sendByte(id);
    finishSend();
}


static void cmd_enterpgm(unsigned char *buffer)
{
    simpleReply(BOOTLOADER_CMD_ENTERPGM);
}

static void cmd_leavepgm(unsigned char *buffer)
{
    simpleReply(BOOTLOADER_CMD_LEAVEPGM);
}
 

static void cmd_none(unsigned char *buffer)
{
}


static const cmdhandler_t handlers[] = {
	&cmd_version,         /* CMD1 */
	&cmd_identify,        /* CMD2 */
	&cmd_waitready,       /* CMD3 */
	&cmd_raw_send_receive,/* CMD4 */
	&cmd_enterpgm,        /* CMD5 */
	&cmd_leavepgm,        /* CMD6 */
	&cmd_none,            /* CMD7 */
	&cmd_set_baudrate,    /* CMD8 */
	&cmd_none,            /* CMD9 */
	&cmd_none             /* CMD10 */
};


static void processCommand(unsigned char *buffer, unsigned bufferpos)
{
    unsigned int pos=0;
    crc_t computed_crc;

    if (bufferpos<3)
        return; // Too few data

    crc16__reset(&computed_crc);

    for (pos=0;pos<bufferpos-2;pos++) {
        crc16__append(&crc,buffer[pos]);
    }
    unsigned int tcrc = buffer[--bufferpos];
    tcrc|=buffer[--bufferpos]<<8;
    if (computed_crc!=tcrc) {
        //printstring("C!");
        return;
    }

    pos=buffer[0];

    if (pos>BOOTLOADER_MAX_CMD)
        return;
    pos--;
    handlers[pos](buffer);
}

void spiflash_check()
{
    int r;
    unsigned char c;
    r = inbyte(&c);

    if(r<0)
        return;

    if (syncSeen) {
        if (c==HDLC_frameFlag) {
            if (bufferpos>0) {
                syncSeen=0;
                processCommand(buffer, bufferpos);
            }
        } else if (c==HDLC_escapeFlag) {
            unescaping=1;
        } else if (bufferpos<sizeof(buffer)) {
            if (unescaping) {
                unescaping=0;
                c^=HDLC_escapeXOR;
            }
            buffer[bufferpos++]=c;
        } else {
            syncSeen=0;
        }
    } else {
        if (c==HDLC_frameFlag) {
            bufferpos=0;
            crc16__reset(&crc);
            syncSeen=1;
            unescaping=0;
        } else {
#ifdef VERBOSE_LOADER
            //outbyte(i); // Echo back.
#endif
        }
    }
}



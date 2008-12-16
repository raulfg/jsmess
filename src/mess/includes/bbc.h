/*****************************************************************************
 *
 * includes/bbc.h
 *
 * BBC Model B
 *
 * Driver by Gordon Jefferyes <mess_bbc@gjeffery.dircon.co.uk>
 *
 ****************************************************************************/

#ifndef BBC_H_
#define BBC_H_

#include "machine/6850acia.h"
#include "machine/i8271.h"
#include "machine/upd7002.h"


/*----------- defined in machine/bbc.c -----------*/

DRIVER_INIT( bbc );
DRIVER_INIT( bbcm );

MACHINE_START( bbca );
MACHINE_START( bbcb );
MACHINE_START( bbcbp );
MACHINE_START( bbcm );

MACHINE_RESET( bbca );
MACHINE_RESET( bbcb );
MACHINE_RESET( bbcbp );
MACHINE_RESET( bbcm );

INTERRUPT_GEN( bbcb_keyscan );
INTERRUPT_GEN( bbcm_keyscan );

WRITE8_HANDLER ( memorya1_w );
WRITE8_HANDLER ( page_selecta_w );

WRITE8_HANDLER ( memoryb3_w );
WRITE8_HANDLER ( memoryb4_w );
WRITE8_HANDLER ( page_selectb_w );


WRITE8_HANDLER ( memorybp1_w );
//READ8_HANDLER  ( memorybp2_r );
WRITE8_HANDLER ( memorybp2_w );
WRITE8_HANDLER ( memorybp4_w );
WRITE8_HANDLER ( memorybp4_128_w );
WRITE8_HANDLER ( memorybp6_128_w );
WRITE8_HANDLER ( page_selectbp_w );


WRITE8_HANDLER ( memorybm1_w );
//READ8_HANDLER  ( memorybm2_r );
WRITE8_HANDLER ( memorybm2_w );
WRITE8_HANDLER ( memorybm4_w );
WRITE8_HANDLER ( memorybm5_w );
WRITE8_HANDLER ( memorybm7_w );
READ8_HANDLER  ( bbcm_r );
WRITE8_HANDLER ( bbcm_w );
READ8_HANDLER  ( bbcm_ACCCON_read );
WRITE8_HANDLER ( bbcm_ACCCON_write );


//WRITE8_HANDLER ( bbc_bank4_w );

/* disc support */

DEVICE_IMAGE_LOAD ( bbcb_cart );
DEVICE_IMAGE_LOAD( bbc_floppy );

READ8_HANDLER  ( bbc_disc_r );
WRITE8_HANDLER ( bbc_disc_w );

READ8_HANDLER  ( bbc_wd1770_read );
WRITE8_HANDLER ( bbc_wd1770_write );

READ8_HANDLER  ( bbc_opus_read );
WRITE8_HANDLER ( bbc_opus_write );


READ8_HANDLER  ( bbcm_wd1770l_read );
WRITE8_HANDLER ( bbcm_wd1770l_write );
READ8_HANDLER  ( bbcm_wd1770_read );
WRITE8_HANDLER ( bbcm_wd1770_write );


/* tape support */

WRITE8_HANDLER ( BBC_6850_w );
READ8_HANDLER (BBC_6850_r);

WRITE8_HANDLER ( BBC_SerialULA_w );

extern const acia6850_interface bbc_acia6850_interface;
extern const i8271_interface bbc_i8271_interface;
extern const uPD7002_interface BBC_uPD7002;

/*----------- defined in video/bbc.c -----------*/

extern VIDEO_START( bbca );
extern VIDEO_START( bbcb );
extern VIDEO_START( bbcbp );
extern VIDEO_START( bbcm );
extern VIDEO_UPDATE( bbc );

extern unsigned char vidmem[0x10000];

void set_video_memory_lookups(int ramsize);
void bbc_frameclock(void);
void setscreenstart(int b4,int b5);
void bbcbp_setvideoshadow(running_machine *machine, int vdusel);

WRITE8_HANDLER ( videoULA_w );

WRITE8_HANDLER ( BBC_6845_w );
READ8_HANDLER ( BBC_6845_r );


#endif /* BBC_H_ */

/*****************************************************************************
 *
 * includes/mikrosha.h
 *
 ****************************************************************************/

#ifndef MIKROSHA_H_
#define MIKROSHA_H_


/*----------- defined in machine/mikrosha.c -----------*/

extern DRIVER_INIT( mikrosha );
extern MACHINE_RESET( mikrosha );

extern const ppi8255_interface mikrosha_ppi8255_interface_1;
extern const i8275_interface mikrosha_i8275_interface;

/*----------- defined in video/mikrosha.c -----------*/

extern I8275_DISPLAY_PIXELS(mikrosha_display_pixels);

extern VIDEO_UPDATE( mikrosha );
extern PALETTE_INIT( mikrosha );


#endif /* MIKROSHA_H_ */

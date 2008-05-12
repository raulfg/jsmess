/***************************************************************************

		Orion machine driver by Miodrag Milanovic

		22/04/2008 Orion Pro added
		02/04/2008 Preliminary driver.
		     
****************************************************************************/


#include "driver.h"
#include "cpu/i8085/i8085.h"
#include "devices/cassette.h"
#include "devices/basicdsk.h"
#include "machine/mc146818.h"
#include "machine/8255ppi.h"
#include "machine/wd17xx.h"
#include "sound/speaker.h"
#include "sound/ay8910.h"

#define SCREEN_WIDTH_384 48
#define SCREEN_WIDTH_480 60
#define SCREEN_WIDTH_512 64

UINT8 romdisk_lsb,romdisk_msb;
UINT8 orion_keyboard_line;
UINT8 orion128_video_mode;
UINT8 orion128_video_page;
UINT8 orion128_memory_page;

UINT8 orion128_video_width;

UINT8 orionz80_memory_page;
UINT8 orionz80_dispatcher;

UINT8 orion_video_mode_mask;

READ8_HANDLER (orion_romdisk_porta_r )
{
	UINT8 *romdisk = memory_region(REGION_CPU1) + 0x10000;		
	return romdisk[romdisk_msb*256+romdisk_lsb];	
}

WRITE8_HANDLER (orion_romdisk_portb_w )
{	
	romdisk_lsb = data;
}

WRITE8_HANDLER (orion_romdisk_portc_w )
{		
	romdisk_msb = data;	
}

READ8_HANDLER (orion_keyboard_portb_r )
{		
	return input_port_read_indexed(machine, orion_keyboard_line);
}

READ8_HANDLER (orion_keyboard_portc_r )
{
	double level = cassette_input(image_from_devtype_and_index(IO_CASSETTE, 0));	 									 					
	UINT8 dat = input_port_read_indexed(machine, 8);
	if (level <  0) { 
		dat ^= 0x10;
 	}	
	return dat;		
}

WRITE8_HANDLER (orion_keyboard_porta_w )
{	
	switch (data ^ 0xff) {
	  	case 0x01 : orion_keyboard_line = 0;break;
	  	case 0x02 : orion_keyboard_line = 1;break;
	  	case 0x04 : orion_keyboard_line = 2;break;
	  	case 0x08 : orion_keyboard_line = 3;break;
	  	case 0x10 : orion_keyboard_line = 4;break;
	  	case 0x20 : orion_keyboard_line = 5;break;
	  	case 0x40 : orion_keyboard_line = 6;break;
	  	case 0x80 : orion_keyboard_line = 7;break;
	}	
}

WRITE8_HANDLER (orion_cassette_portc_w )
{
	cassette_output(image_from_devtype_and_index(IO_CASSETTE, 0),data & 0x01 ? 1 : -1);	
}

const ppi8255_interface orion128_ppi8255_interface_1 =
{
	orion_romdisk_porta_r,
	NULL,
	NULL,
	NULL,
	orion_romdisk_portb_w,
	orion_romdisk_portc_w
};

const ppi8255_interface orion128_ppi8255_interface_2 =
{
	NULL,
	orion_keyboard_portb_r,
	orion_keyboard_portc_r,
	orion_keyboard_porta_w,
	NULL,
	orion_cassette_portc_w
};

/* Driver initialization */
DRIVER_INIT( orion128 )
{
	memset(mess_ram,0,256*1024);
}


MACHINE_START( orion128 )
{
	wd17xx_init(machine, WD_TYPE_1793, NULL , NULL);
	wd17xx_set_density (DEN_FM_HI);	
	orion_video_mode_mask = 7;
}

READ8_HANDLER ( orion128_system_r ) 
{
	return ppi8255_r((device_config*)device_list_find_by_tag( machine->config->devicelist, PPI8255, "ppi8255_2" ), offset & 3);
}

WRITE8_HANDLER ( orion128_system_w ) 
{
	ppi8255_w((device_config*)device_list_find_by_tag( machine->config->devicelist, PPI8255, "ppi8255_2" ), offset & 3, data);	
}

READ8_HANDLER ( orion128_romdisk_r ) 
{
	return ppi8255_r((device_config*)device_list_find_by_tag( machine->config->devicelist, PPI8255, "ppi8255_1" ), offset & 3);	
}

WRITE8_HANDLER ( orion128_romdisk_w ) 
{	
	ppi8255_w((device_config*)device_list_find_by_tag( machine->config->devicelist, PPI8255, "ppi8255_1" ), offset & 3, data);	
}

void orion_set_video_mode(running_machine *machine, int width) {
		rectangle visarea;
		
		visarea.min_x = 0;
		visarea.min_y = 0;
		visarea.max_x = width-1;
		visarea.max_y = 255;				
		video_screen_configure(machine->primary_screen, width, 256, &visarea, video_screen_get_frame_period(machine->primary_screen).attoseconds);	
}

WRITE8_HANDLER ( orion128_video_mode_w )
{				
	if ((data & 0x80)!=(orion128_video_mode & 0x80)) {
		if ((data & 0x80)==0x80) {
			if (orion_video_mode_mask == 31) {
				orion128_video_width = SCREEN_WIDTH_512;
				orion_set_video_mode(machine,512);		
			} else {
				orion128_video_width = SCREEN_WIDTH_480;
				orion_set_video_mode(machine,480);		
			}			
		} else {
			orion128_video_width = SCREEN_WIDTH_384;
			orion_set_video_mode(machine,384);
		}
	}				
	
	orion128_video_mode = data;
}

WRITE8_HANDLER ( orion128_video_page_w )
{	
	if (orion128_video_page != data) {
		if ((data & 0x80)!=(orion128_video_page & 0x80)) {
			if ((data & 0x80)==0x80) {
				if (orion_video_mode_mask == 31) {
					orion128_video_width = SCREEN_WIDTH_512;
					orion_set_video_mode(machine,512);		
				} else {
					orion128_video_width = SCREEN_WIDTH_480;
					orion_set_video_mode(machine,480);		
				}			
			} else {
				orion128_video_width = SCREEN_WIDTH_384;
				orion_set_video_mode(machine,384);
			}
		}				
	}
	orion128_video_page = data;
}


WRITE8_HANDLER ( orion128_memory_page_w )
{				
	if (data!=orion128_memory_page ) {
		memory_set_bankptr(1, mess_ram + (data & 3) * 0x10000);
		orion128_memory_page = (data & 3);
	}
}

MACHINE_RESET ( orion128 ) 
{		
	wd17xx_reset();
	wd17xx_set_density (DEN_FM_HI);	
	orion_keyboard_line = 0;
	orion128_video_page = 0;
	orion128_video_mode = 0;
	orion128_memory_page = -1;
	memory_set_bankptr(1, memory_region(REGION_CPU1) + 0xf800);
	memory_set_bankptr(2, mess_ram + 0xf000);
	orion128_video_width = SCREEN_WIDTH_384;
	orion_set_video_mode(machine,384);
}


DEVICE_IMAGE_LOAD( orion_floppy )
{
	int size;

	if (! image_has_been_created(image))
		{
		size = image_length(image);

		switch (size)
			{
			case 800*1024:
				break;
			case 720*1024:
				break;
			default:
				return INIT_FAIL;
			}
		}
	else
		return INIT_FAIL;

	if (device_load_basicdsk_floppy (image) != INIT_PASS)
		return INIT_FAIL;

	if (size==800*1024) {
		basicdsk_set_geometry (image, 80, 2, 5, 1024, 1, 0, FALSE);
	} else {
		basicdsk_set_geometry (image, 80, 2, 9, 512, 1, 0, FALSE);
	}
	return INIT_PASS;
}

WRITE8_HANDLER ( orion_disk_control_w )
{
	wd17xx_set_side(((data & 0x10) >> 4) ^ 1);
 	wd17xx_set_drive(data & 3);				
}

READ8_HANDLER ( orion128_floppy_r )
{	
	switch(offset) {
		case 0x0	:
		case 0x10 : return wd17xx_status_r(machine,0);
		case 0x1 	:
		case 0x11 : return wd17xx_track_r(machine,0);
		case 0x2  :
		case 0x12 : return wd17xx_sector_r(machine,0);
		case 0x3  :
		case 0x13 : return wd17xx_data_r(machine,0);
	}	
	return 0xff;
}

WRITE8_HANDLER ( orion128_floppy_w )
{		
	switch(offset) {
		case 0x0	:
		case 0x10 : wd17xx_command_w(machine,0,data); break;
		case 0x1 	:
		case 0x11 : wd17xx_track_w(machine,0,data);break;
		case 0x2  :
		case 0x12 : wd17xx_sector_w(machine,0,data);break;
		case 0x3  :
		case 0x13 : wd17xx_data_w(machine,0,data);break;
		case 0x4  : 
		case 0x14 : 
		case 0x20 : orion_disk_control_w(machine, offset, data);break;
	}
}
READ8_HANDLER ( orionz80_floppy_rtc_r )
{	
	if ((offset >= 0x60) && (offset <= 0x6f)) {
		return mc146818_port_r(machine,offset-0x60);
	} else { 
		return orion128_floppy_r(machine,offset);
	}	
}

WRITE8_HANDLER ( orionz80_floppy_rtc_w )
{		
	if ((offset >= 0x60) && (offset <= 0x6f)) {
		return mc146818_port_w(machine,offset-0x60,data);
	} else { 
		return orion128_floppy_w(machine,offset,data);
	}	
}


DRIVER_INIT( orionz80 )
{
	memset(mess_ram,0,512*1024);	
}


MACHINE_START( orionz80 )
{
	wd17xx_init(machine, WD_TYPE_1793, NULL , NULL);
	wd17xx_set_density (DEN_FM_HI);
	mc146818_init(MC146818_IGNORE_CENTURY);
	orion_video_mode_mask = 7;
}

UINT8 orion_speaker;
WRITE8_HANDLER ( orionz80_sound_w )
{	
	if (orion_speaker==0) {
		orion_speaker = data;		
	} else {
		orion_speaker = 0 ;	
	}
	speaker_level_w(0,orion_speaker);
		
}

WRITE8_HANDLER ( orionz80_sound_fe_w )
{	
	speaker_level_w(0,(data>>4) & 0x01);
}


WRITE8_HANDLER ( orionz80_memory_page_w );
WRITE8_HANDLER ( orionz80_dispatcher_w );

static void orionz80_switch_bank(running_machine *machine)
{
	UINT8 bank_select;
	UINT8 segment_select;
	
	bank_select = (orionz80_dispatcher & 0x0c) >> 2;
	segment_select = orionz80_dispatcher & 0x03;
	
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x0000, 0x3fff, 0, 0, SMH_BANK1);
	if ((orionz80_dispatcher & 0x80)==0) { // dispatcher on
		memory_set_bankptr(1, mess_ram + 0x10000 * bank_select + segment_select * 0x4000 );		
	} else { // dispatcher off
		memory_set_bankptr(1, mess_ram + 0x10000 * orionz80_memory_page);		
	}
		
	memory_set_bankptr(2, mess_ram + 0x4000 + 0x10000 * orionz80_memory_page);		
	
	if ((orionz80_dispatcher & 0x20) == 0) {
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf400, 0xf4ff, 0, 0, orion128_system_w);
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf500, 0xf5ff, 0, 0, orion128_romdisk_w);
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf700, 0xf7ff, 0, 0, orionz80_floppy_rtc_w);	
		memory_install_read8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf400, 0xf4ff, 0, 0, orion128_system_r);
		memory_install_read8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf500, 0xf5ff, 0, 0, orion128_romdisk_r);
		memory_install_read8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf700, 0xf7ff, 0, 0, orionz80_floppy_rtc_r);	
		
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf800, 0xf8ff, 0, 0, orion128_video_mode_w);
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf900, 0xf9ff, 0, 0, orionz80_memory_page_w);
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xfa00, 0xfaff, 0, 0, orion128_video_page_w);
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xfb00, 0xfbff, 0, 0, orionz80_dispatcher_w);
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xfc00, 0xfeff, 0, 0, SMH_UNMAP);
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xff00, 0xffff, 0, 0, orionz80_sound_w);
		
		memory_set_bankptr(3, mess_ram + 0xf000);				
		memory_set_bankptr(5, memory_region(REGION_CPU1) + 0xf800);		
		
	} else {
		/* if it is full memory access */
		memory_set_bankptr(3, mess_ram + 0xf000 + 0x10000 * orionz80_memory_page);		
		memory_set_bankptr(4, mess_ram + 0xf400 + 0x10000 * orionz80_memory_page);		
		memory_set_bankptr(5, mess_ram + 0xf800 + 0x10000 * orionz80_memory_page);		
	}		
}

WRITE8_HANDLER ( orionz80_memory_page_w )
{	
	orionz80_memory_page = data & 7;
	orionz80_switch_bank(machine);
}

WRITE8_HANDLER ( orionz80_dispatcher_w )
{	
	orionz80_dispatcher = data;
	orionz80_switch_bank(machine);
}

MACHINE_RESET ( orionz80 ) 
{	
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x0000, 0x3fff, 0, 0, SMH_UNMAP);
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x4000, 0xefff, 0, 0, SMH_BANK2);
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf000, 0xf3ff, 0, 0, SMH_BANK3);

	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf400, 0xf4ff, 0, 0, orion128_system_w);
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf500, 0xf5ff, 0, 0, orion128_romdisk_w);
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf700, 0xf7ff, 0, 0, orionz80_floppy_rtc_w);	
	memory_install_read8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf400, 0xf4ff, 0, 0, orion128_system_r);
	memory_install_read8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf500, 0xf5ff, 0, 0, orion128_romdisk_r);
	memory_install_read8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf700, 0xf7ff, 0, 0, orionz80_floppy_rtc_r);	
	
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf800, 0xf8ff, 0, 0, orion128_video_mode_w);
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf900, 0xf9ff, 0, 0, orionz80_memory_page_w);
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xfa00, 0xfaff, 0, 0, orion128_video_page_w);
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xfb00, 0xfbff, 0, 0, orionz80_dispatcher_w);
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xfc00, 0xfeff, 0, 0, SMH_UNMAP);
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xff00, 0xffff, 0, 0, orionz80_sound_w);
	
	
	memory_set_bankptr(1, memory_region(REGION_CPU1) + 0xf800);		
	memory_set_bankptr(2, mess_ram + 0x4000);		
	memory_set_bankptr(3, mess_ram + 0xf000);		
	memory_set_bankptr(5, memory_region(REGION_CPU1) + 0xf800);		
	
	wd17xx_reset();
	orion_keyboard_line = 0;
	orion128_video_page = 0;
	orion128_video_mode = 0;
	orionz80_memory_page = 0;
	orionz80_dispatcher = 0;
	orion_speaker = 0;
	orion128_video_width = SCREEN_WIDTH_384;
	orion_set_video_mode(machine,384);
}

INTERRUPT_GEN( orionz80_interrupt ) 
{
	if ((orionz80_dispatcher & 0x40)==0x40) {
		cpunum_set_input_line(machine, 0, 0, HOLD_LINE);			
	}	
}

READ8_HANDLER ( orionz80_io_r ) {
	if (offset == 0xFFFD) {
		return AY8910_read_port_0_r (machine, 0);
	}
	return 0xff;
}

WRITE8_HANDLER ( orionz80_io_w ) {
	switch (offset & 0xff) {
		case 0xf8 : orion128_video_mode_w(machine,0,data);break;
		case 0xf9 : orionz80_memory_page_w(machine,0,data);break;
		case 0xfa : orion128_video_page_w(machine,0,data);break;
		case 0xfb : orionz80_dispatcher_w(machine,0,data);break;
		case 0xfe : orionz80_sound_fe_w(machine,0,data);break;
		case 0xff : orionz80_sound_w(machine,0,data);break;
	}
	switch(offset) {
		case 0xfffd : AY8910_control_port_0_w(machine, 0, data);
					  break;
		case 0xbffd :
		case 0xbefd : AY8910_write_port_0_w(machine, 0, data);
		 			  break;		
	}
}

UINT8 orionpro_ram0_segment;
UINT8 orionpro_ram1_segment;
UINT8 orionpro_ram2_segment;

UINT8 orionpro_page;
UINT8 orionpro_128_page;
UINT8 orionpro_rom2_segment;

UINT8 orionpro_dispatcher;
UINT8 orionpro_pseudo_color;

DRIVER_INIT( orionpro )
{
	memset(mess_ram,0,512*1024);	
}


MACHINE_START( orionpro )
{
	wd17xx_init(machine, WD_TYPE_1793, NULL , NULL);
	wd17xx_set_density (DEN_FM_HI);
}

WRITE8_HANDLER ( orionpro_memory_page_w );

void orionpro_bank_switch(running_machine *machine)
{
	int page = orionpro_page & 7; // we have only 8 pages	
	int is128 = (orionpro_dispatcher & 0x80) ? 1 : 0;
	if (is128==1) {
		page = orionpro_128_page & 7;
	}
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x0000, 0x1fff, 0, 0, SMH_BANK1);
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x2000, 0x3fff, 0, 0, SMH_BANK2);
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x4000, 0x7fff, 0, 0, SMH_BANK3);
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x8000, 0xbfff, 0, 0, SMH_BANK4);
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xc000, 0xefff, 0, 0, SMH_BANK5);
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf000, 0xf3ff, 0, 0, SMH_BANK6);
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf400, 0xf7ff, 0, 0, SMH_BANK7);
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf800, 0xffff, 0, 0, SMH_BANK8);
	
	
	if ((orionpro_dispatcher & 0x01)==0x00) {	// RAM0 segment disabled
		memory_set_bankptr(1, mess_ram + 0x10000 * page);
		memory_set_bankptr(2, mess_ram + 0x10000 * page + 0x2000);		
	} else {                
		memory_set_bankptr(1, mess_ram + (orionpro_ram0_segment & 31) * 0x4000);
		memory_set_bankptr(2, mess_ram + (orionpro_ram0_segment & 31) * 0x4000 + 0x2000);
	}
	if ((orionpro_dispatcher & 0x10)==0x10) {	// ROM1 enabled		
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x0000, 0x1fff, 0, 0, SMH_UNMAP);
		memory_set_bankptr(1, memory_region(REGION_CPU1) + 0x20000);		
	}
	if ((orionpro_dispatcher & 0x08)==0x08) {	// ROM2 enabled
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x2000, 0x3fff, 0, 0, SMH_UNMAP);
		memory_set_bankptr(2, memory_region(REGION_CPU1) + 0x22000 + (orionpro_rom2_segment & 7) * 0x2000);		
	}

	if ((orionpro_dispatcher & 0x02)==0x00) {	// RAM1 segment disabled
		memory_set_bankptr(3, mess_ram + 0x10000 * page + 0x4000);		
	} else {                     
		memory_set_bankptr(3, mess_ram + (orionpro_ram1_segment & 31) * 0x4000);		
	}
	
	if ((orionpro_dispatcher & 0x04)==0x00) {	// RAM2 segment disabled
		memory_set_bankptr(4, mess_ram + 0x10000 * page + 0x8000);
	} else {        
		memory_set_bankptr(4, mess_ram + (orionpro_ram2_segment & 31) * 0x4000);
	}
	
	memory_set_bankptr(5, mess_ram + 0x10000 * page + 0xc000);
	
	if (is128) {
		memory_set_bankptr(6, mess_ram + 0x10000 * 0 + 0xf000);	

		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf400, 0xf4ff, 0, 0, orion128_system_w);
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf500, 0xf5ff, 0, 0, orion128_romdisk_w);
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf600, 0xf6ff, 0, 0, SMH_UNMAP);
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf700, 0xf7ff, 0, 0, orion128_floppy_w);	
		memory_install_read8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf400, 0xf4ff, 0, 0, orion128_system_r);
		memory_install_read8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf500, 0xf5ff, 0, 0, orion128_romdisk_r);
		memory_install_read8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf600, 0xf6ff, 0, 0, SMH_UNMAP);
		memory_install_read8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf700, 0xf7ff, 0, 0, orion128_floppy_r);	
	
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf800, 0xf8ff, 0, 0, orion128_video_mode_w);
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf900, 0xf9ff, 0, 0, orionpro_memory_page_w);
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xfa00, 0xfaff, 0, 0, orion128_video_page_w);
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xfb00, 0xfeff, 0, 0, SMH_UNMAP);
		memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xff00, 0xffff, 0, 0, orionz80_sound_w);
	
	
		memory_set_bankptr(8, mess_ram + 0x10000 * 0 + 0xf800);	
	} else {
		if ((orionpro_dispatcher & 0x40)==0x40) {	// FIX F000 enabled
			memory_set_bankptr(6, mess_ram + 0x10000 * 0 + 0xf000);		
			memory_set_bankptr(7, mess_ram + 0x10000 * 0 + 0xf400);		
			memory_set_bankptr(8, mess_ram + 0x10000 * 0 + 0xf800);		
		} else {
			memory_set_bankptr(6, mess_ram + 0x10000 * page + 0xf000);		
			memory_set_bankptr(7, mess_ram + 0x10000 * page + 0xf400);		
			memory_set_bankptr(8, mess_ram + 0x10000 * page + 0xf800);		
		}
	}	
}

WRITE8_HANDLER ( orionpro_memory_page_w )
{	
	orionpro_128_page = data;
	orionpro_bank_switch(machine);
}

MACHINE_RESET ( orionpro ) 
{	
	
	wd17xx_reset();

	orion_keyboard_line = 0;
	orion128_video_page = 0;
	orion128_video_mode = 0;
	orionpro_ram0_segment = 0;
	orionpro_ram1_segment = 0;
	orionpro_ram2_segment = 0;

	orionpro_page = 0;
	orionpro_128_page = 0;
	orionpro_rom2_segment = 0;

	orionpro_dispatcher = 0x50;
	orionpro_bank_switch(machine);
	
	orion_speaker = 0;
	orion128_video_width = SCREEN_WIDTH_384;
	orion_set_video_mode(machine,384);

	orion_video_mode_mask = 31;
	orionpro_pseudo_color = 0;
}

READ8_HANDLER ( orionpro_io_r ) {
	switch (offset & 0xff) {		
		case 0x00 : return 0x56;
		case 0x04 : return orionpro_ram0_segment;
		case 0x05 : return orionpro_ram1_segment;
		case 0x06 : return orionpro_ram2_segment;
		case 0x08 : return orionpro_page;
		case 0x09 : return orionpro_rom2_segment;
		case 0x0a : return orionpro_dispatcher;
		case 0x10 : return wd17xx_status_r(machine,0); 
		case 0x11 : return wd17xx_track_r(machine,0);
		case 0x12 : return wd17xx_sector_r(machine,0);
		case 0x13 : return wd17xx_data_r(machine,0);
		case 0x18 : 
		case 0x19 : 
		case 0x1a : 
		case 0x1b : 
					return orion128_system_r(machine,(offset & 0xff)-0x18); break;
		case 0x28 : return orion128_romdisk_r(machine,0); break;
		case 0x29 : return orion128_romdisk_r(machine,1); break;
		case 0x2a : return orion128_romdisk_r(machine,2); break;
		case 0x2b : return orion128_romdisk_r(machine,3); break;
	}
	if (offset == 0xFFFD) {
		return AY8910_read_port_0_r (machine, 0);
	}
	return 0xff;
}

WRITE8_HANDLER ( orionpro_io_w ) {
	switch (offset & 0xff) {		
		case 0x04 : orionpro_ram0_segment = data; orionpro_bank_switch(machine); break;		
		case 0x05 : orionpro_ram1_segment = data; orionpro_bank_switch(machine); break;
		case 0x06 : orionpro_ram2_segment = data; orionpro_bank_switch(machine); break;
		case 0x08 : orionpro_page = data; 		  orionpro_bank_switch(machine); break;
		case 0x09 : orionpro_rom2_segment = data; orionpro_bank_switch(machine); break;
		case 0x0a : orionpro_dispatcher = data;   orionpro_bank_switch(machine); break;
		case 0x10 : wd17xx_command_w(machine,0,data); break;
		case 0x11 : wd17xx_track_w(machine,0,data);break;
		case 0x12 : wd17xx_sector_w(machine,0,data);break;
		case 0x13 : wd17xx_data_w(machine,0,data);break;
		case 0x14 : orion_disk_control_w(machine, 9, data);break;
		case 0x18 : 
		case 0x19 : 
		case 0x1a : 
		case 0x1b : 
					orion128_system_w(machine,(offset & 0xff)-0x18,data); break;
		case 0x28 : orion128_romdisk_w(machine,0,data); break;
		case 0x29 : orion128_romdisk_w(machine,1,data); break;
		case 0x2a : orion128_romdisk_w(machine,2,data); break;
		case 0x2b : orion128_romdisk_w(machine,3,data); break;
		case 0xf8 : orion128_video_mode_w(machine,0,data);break;
		case 0xf9 : orionpro_128_page = data;	  orionpro_bank_switch(machine); break;
		case 0xfa : orion128_video_page_w(machine,0,data);break;		
		case 0xfc : orionpro_pseudo_color = data;break;
		case 0xfe : orionz80_sound_fe_w(machine,0,data);break;
		case 0xff : orionz80_sound_w(machine,0,data);break;
	}
	switch(offset) {
		case 0xfffd : AY8910_control_port_0_w(machine, 0, data);
					  break;
		case 0xbffd :
		case 0xbefd : AY8910_write_port_0_w(machine, 0, data);
		 			  break;		
	}
}
      

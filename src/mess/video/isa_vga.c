/***************************************************************************

  ISA VGA wrapper

***************************************************************************/

#include "emu.h"
#include "isa_vga.h"
#include "video/pc_vga.h"

ROM_START( et4000 )
	ROM_REGION(0x8000,"et4000", 0)
	ROM_LOAD("et4000.bin", 0x00000, 0x8000, CRC(f1e817a8) SHA1(945d405b0fb4b8f26830d495881f8587d90e5ef9) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_VGA = &device_creator<isa8_vga_device>;


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa8_vga_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pcvideo_vga );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa8_vga_device::device_rom_region() const
{
	return ROM_NAME( et4000 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_vga_device - constructor
//-------------------------------------------------

isa8_vga_device::isa8_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, ISA8_VGA, "ISA8_VGA", tag, owner, clock),
		device_isa8_card_interface(mconfig, *this)
{
	m_shortname = "et4000";
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
static READ8_HANDLER( input_port_0_r ) { return 0xff; } //return input_port_read(space->machine(), "IN0"); }

void isa8_vga_device::device_start()
{
	set_isa_device();
	
	pc_vga_init(machine(), input_port_0_r, NULL);

	m_isa->install_rom(this, 0xc0000, 0xc7fff, 0, 0, "et4000", "et4000");
	
	m_isa->install_device(0x3b0, 0x3bf, 0, 0, FUNC(vga_port_03b0_r), FUNC(vga_port_03b0_w));
	m_isa->install_device(0x3c0, 0x3cf, 0, 0, FUNC(vga_port_03c0_r), FUNC(vga_port_03c0_w));
	m_isa->install_device(0x3d0, 0x3df, 0, 0, FUNC(vga_port_03d0_r), FUNC(vga_port_03d0_w));
	
	m_isa->install_memory(0xa0000, 0xbffff, 0, 0, FUNC(vga_mem_r), FUNC(vga_mem_w));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_vga_device::device_reset()
{
	pc_vga_reset(machine());
}
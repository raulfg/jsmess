/**********************************************************************

    IDE64 v4.1 cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - flash write
    - FT245

*/

#include "c64_ide64.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define AT29C010A_TAG		"u3"
#define DS1302_TAG			"u4"
#define FT245R_TAG			"u21"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_IDE64 = &device_creator<c64_ide64_cartridge_device>;


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_ide64 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_ide64 )
	MCFG_ATMEL_29C010_ADD(AT29C010A_TAG)
	MCFG_DS1302_ADD(DS1302_TAG)
	// TODO FT245R
	// TODO CompactFlash

	MCFG_IDE_CONTROLLER_ADD("ide", NULL, ide_image_devices, "hdd", "hdd")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_ide64_cartridge_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_ide64 );
}


//-------------------------------------------------
//  INPUT_PORTS( c64_ide64 )
//-------------------------------------------------

static INPUT_PORTS_START( c64_ide64 )
	PORT_START("JP1")
	PORT_DIPNAME( 0x01, 0x01, "Flash ROM Write Protect" )
	PORT_DIPSETTING(    0x00, "Disabled" )
	PORT_DIPSETTING(    0x01, "Enabled" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_ide64_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_ide64 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_ide64_cartridge_device - constructor
//-------------------------------------------------

c64_ide64_cartridge_device::c64_ide64_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_IDE64, "C64 IDE64 cartridge", tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_flash_rom(*this, AT29C010A_TAG),
	m_rtc(*this, DS1302_TAG),
	m_ide(*this, "ide")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_ide64_cartridge_device::device_start()
{
	// allocate memory
	c64_ram_pointer(machine(), 0x8000);

	// state saving
	save_item(NAME(m_bank));
	save_item(NAME(m_ide_data));
	save_item(NAME(m_enable));
	save_item(NAME(m_rtc_ce));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_ide64_cartridge_device::device_reset()
{
	m_bank = 0;
	m_game = 1;
	m_exrom = 1;
	m_enable = 1;
	m_rtc_ce = 0;

	m_wp = input_port_read(*this, "JP1");
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_ide64_cartridge_device::c64_cd_r(address_space &space, offs_t offset, int roml, int romh, int io1, int io2)
{
	if (!m_enable) return 0;

	UINT8 data = 0;
	int rom_oe = 1, ram_oe = 1;

	if (!m_game && m_exrom)
	{
		if (offset >= 0x1000 && offset < 0x8000)
		{
			ram_oe = 0;
		}
		else if (offset >= 0xa000 && offset < 0xc000)
		{
			rom_oe = 0;
		}
		else if (offset >= 0xc000 && offset < 0xd000)
		{
			ram_oe = 0;
		}
		else if (!romh)
		{
			rom_oe = 0;
		}
	}
	else
	{
		if (!roml || !romh)
		{
			rom_oe = 0;
		}
		else if (!io1)
		{
			// 0x20-0x2f    IDE
			// 0x30-0x37    I/O
			// 0x5d-0x5e    FT245
			// 0x5f-0x5f    DS1302
			// 0x60-0xff    ROM

			UINT8 io1_offset = offset & 0xff;

			if (io1_offset >= 0x20 && io1_offset < 0x30)
			{
				m_ide_data = ide_bus_r(m_ide, BIT(offset, 3), offset & 0x07);

				data = m_ide_data & 0xff;
			}
			else if (io1_offset == 0x31)
			{
				data = m_ide_data >> 8;
			}
			else if (io1_offset == 0x32)
			{
				/*

                    bit     description

                    0       GAME
                    1       EXROM
                    2       A14
                    3       A15
                    4       A16
                    5       v4.x
                    6
                    7

                */

				data = 0x20 | (m_bank << 2) | (m_exrom << 1) | m_game;
			}
			else if ((io1_offset == 0x5f) && m_rtc_ce)
			{
				m_rtc->ds1302_clk_w(0, 0);

				m_rtc->ds1302_dat_w(0, BIT(data, 0));

				m_rtc->ds1302_clk_w(0, 1);
			}
			else if (io1_offset >= 0x60)
			{
				rom_oe = 0;
			}
		}
	}

	if (!rom_oe)
	{
		offs_t addr = (m_bank << 14) | (offset & 0x3fff);
		data = m_flash_rom->read(addr);
	}
	else if (!ram_oe)
	{
		data = m_ram[offset & 0x7fff];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_ide64_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int roml, int romh, int io1, int io2)
{
	if (!m_enable) return;

	if (!m_game && m_exrom)
	{
		if (offset >= 0x1000 && offset < 0x8000)
		{
			m_ram[offset & 0x7fff] = data;
		}
		else if (offset >= 0xc000 && offset < 0xd000)
		{
			m_ram[offset & 0x7fff] = data;
		}
	}
	else
	{
		if ((offset >= 0x8000 && offset < 0xc000) && !m_wp)
		{
			offs_t addr = (m_bank << 14) | (offset & 0x3fff);
			m_flash_rom->write(addr, data);
		}
		else if (!io1)
		{
			// 0x20-0x2f    IDE
			// 0x30-0x37    I/O
			// 0x5d-0x5e    FT245
			// 0x5f-0x5f    DS1302
			// 0x60-0xff    ROM

			UINT8 io1_offset = offset & 0xff;

			if (io1_offset >= 0x20 && io1_offset < 0x30)
			{
				m_ide_data = (m_ide_data & 0xff00) | data;

				ide_bus_w(m_ide, BIT(offset, 3), offset & 0x07, m_ide_data);
			}
			else if (io1_offset == 0x31)
			{
				m_ide_data = (data << 8) | (m_ide_data & 0xff);
			}
			else if ((io1_offset == 0x5f) && m_rtc_ce)
			{
				m_rtc->ds1302_clk_w(0, 0);

				data = m_rtc->ds1302_read(0);

				m_rtc->ds1302_clk_w(0, 1);
			}
			else if (io1_offset >= 0x60 && io1_offset < 0x68)
			{
				m_bank = offset & 0x07;
			}
			else if (io1_offset >= 0xfb)
			{
				/*

                    bit     description

                    0       disable cartridge
                    1       RTC CE
                    2
                    3
                    4
                    5
                    6
                    7

                */

				m_enable = !BIT(data, 0);
				m_rtc_ce = BIT(data, 1);

				m_game = BIT(offset, 0);
				m_exrom = BIT(offset, 1);
			}
			else if (io1_offset >= 0xfc)
			{
				m_game = BIT(offset, 0);
				m_exrom = BIT(offset, 1);
			}
		}
	}
}
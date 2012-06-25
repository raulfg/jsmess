#pragma once

#ifndef __TVC_HBF_H__
#define __TVC_HBF_H__

#include "emu.h"
#include "machine/tvcexp.h"
#include "machine/wd17xx.h"
#include "imagedev/flopdrv.h"
#include "formats/basicdsk.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tvc_hbf_device

class tvc_hbf_device :
		public device_t,
		public device_tvcexp_interface
{
public:
	// construction/destruction
	tvc_hbf_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual const rom_entry *device_rom_region() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "tvc_hbf"; }

	// tvcexp_interface overrides
	virtual UINT8 id_r() { return 0x02; } // ID_A to GND, ID_B to VCC
	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);
	virtual DECLARE_READ8_MEMBER(io_read);
	virtual DECLARE_WRITE8_MEMBER(io_write);

private:
	// internal state
	required_device<device_t>	m_fdc;

	UINT8 *		m_rom;
	UINT8 *		m_ram;
	UINT8		m_rom_bank;		// A12 and A13
};


// device type definition
extern const device_type TVC_HBF;

#endif  /* __TVC_HBF_H__ */
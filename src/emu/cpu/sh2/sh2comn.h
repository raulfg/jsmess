/*****************************************************************************
 *
 *   sh2common.h
 *
 *   SH-2 non-specific components
 *
 *****************************************************************************/

#ifndef _SH2_COMMON_H_
#define _SH2_COMMON_H_

typedef struct
{
	int irq_vector;
	int irq_priority;
} irq_entry;

enum {
	ICF  = 0x00800000,
	OCFA = 0x00080000,
	OCFB = 0x00040000,
	OVF  = 0x00020000
};

#define T	0x00000001
#define S	0x00000002
#define I	0x000000f0
#define Q	0x00000100
#define M	0x00000200

#define AM	0xc7ffffff

#define FLAGS	(M|Q|I|S|T)

#define Rn	((opcode>>8)&15)
#define Rm	((opcode>>4)&15)

#define CHECK_PENDING_IRQ(message)				\
do {											\
	int irq = -1;								\
	if (sh2->pending_irq & (1 <<  0)) irq =	0;	\
	if (sh2->pending_irq & (1 <<  1)) irq =	1;	\
	if (sh2->pending_irq & (1 <<  2)) irq =	2;	\
	if (sh2->pending_irq & (1 <<  3)) irq =	3;	\
	if (sh2->pending_irq & (1 <<  4)) irq =	4;	\
	if (sh2->pending_irq & (1 <<  5)) irq =	5;	\
	if (sh2->pending_irq & (1 <<  6)) irq =	6;	\
	if (sh2->pending_irq & (1 <<  7)) irq =	7;	\
	if (sh2->pending_irq & (1 <<  8)) irq =	8;	\
	if (sh2->pending_irq & (1 <<  9)) irq =	9;	\
	if (sh2->pending_irq & (1 << 10)) irq = 10;	\
	if (sh2->pending_irq & (1 << 11)) irq = 11;	\
	if (sh2->pending_irq & (1 << 12)) irq = 12;	\
	if (sh2->pending_irq & (1 << 13)) irq = 13;	\
	if (sh2->pending_irq & (1 << 14)) irq = 14;	\
	if (sh2->pending_irq & (1 << 15)) irq = 15;	\
	if ((sh2->internal_irq_level != -1) && (sh2->internal_irq_level > irq)) irq = sh2->internal_irq_level; \
	if (irq >= 0)								\
		sh2_exception(message,irq); 			\
} while(0)

typedef struct
{
	UINT32	ppc;
	UINT32	pc;
	UINT32	pr;
	UINT32	sr;
	UINT32	gbr, vbr;
	UINT32	mach, macl;
	UINT32	r[16];
	UINT32	ea;
	UINT32	delay;
	UINT32	cpu_off;
	UINT32	dvsr, dvdnth, dvdntl, dvcr;
	UINT32	pending_irq;
	UINT32    test_irq;
	irq_entry     irq_queue[16];

	INT8	irq_line_state[17];
	int 	(*irq_callback)(int irqline);
	UINT32	*m;
	INT8  nmi_line_state;

	UINT16 	frc;
	UINT16 	ocra, ocrb, icr;
	UINT64 	frc_base;

	int		frt_input;
	int 	internal_irq_level;
	int 	internal_irq_vector;

	emu_timer *timer;
	emu_timer *dma_timer[2];
	int     dma_timer_active[2];

	int     is_slave, cpu_number;

	void	(*ftcsr_read_callback)(UINT32 data);
} SH2;

TIMER_CALLBACK( sh2_timer_callback );
TIMER_CALLBACK( sh2_dmac_callback );

void sh2_common_init(int index, int clock, const void *config, int (*irqcallback)(int));
void sh2_recalc_irq(void);
void sh2_set_irq_line(int irqline, int state);
void sh2_set_frt_input(int cpunum, int state);
void sh2_exception(const char *message, int irqline);

#endif

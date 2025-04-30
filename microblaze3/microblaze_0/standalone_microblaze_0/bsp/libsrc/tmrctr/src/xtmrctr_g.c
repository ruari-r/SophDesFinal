#include "xtmrctr.h"

XTmrCtr_Config XTmrCtr_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,axi-timer-2.0", /* compatible */
		0x40009000, /* reg */
		0x5f5e100, /* clock-frequency */
		0xffff, /* interrupts */
		0xffff /* interrupt-parent */
	},
	{
		"xlnx,axi-timer-2.0", /* compatible */
		0x4000a000, /* reg */
		0x5f5e100, /* clock-frequency */
		0xffff, /* interrupts */
		0xffff /* interrupt-parent */
	},
	{
		"xlnx,axi-timer-2.0", /* compatible */
		0x4000b000, /* reg */
		0x5f5e100, /* clock-frequency */
		0xffff, /* interrupts */
		0xffff /* interrupt-parent */
	},
	{
		"xlnx,axi-timer-2.0", /* compatible */
		0x4000c000, /* reg */
		0x5f5e100, /* clock-frequency */
		0xffff, /* interrupts */
		0xffff /* interrupt-parent */
	},
	 {
		 NULL
	}
};
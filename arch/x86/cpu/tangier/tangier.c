/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/u-boot-x86.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/mmc.h>
#include <asm/msr.h>
#include <asm/arch/intel-mid.h>
#include <intel_scu_ipc.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscellaneous platform dependent initializations
 */
int cpu_init_f(void)
{
	/*
	 *timestamp_init();
	 */
	init_scu_ipc();
	return 0;
}

int board_early_init_f(void)
{
	return 0;
}

int board_early_init_r(void)
{
	return 0;
}

void show_boot_progress(int val)
{
	outb(val, 0x80);
}

int last_stage_init(void)
{
	/*
	 *if (gd->flags & GD_FLG_COLD_BOOT)
	 *        timestamp_add_to_bootstage();
	 */

	return 0;
}

int board_final_cleanup(void)
{

	return 0;
}

void panic_puts(const char *str)
{
}

int board_mmc_init(bd_t * bis)
{
	int index = 0;
	unsigned int base = CONFIG_SYS_EMMC_PORT_BASE + (0x40000 * index);

	return tangier_sdhci_init(base, index, 4);
}


/* ovveride get_tbclk_mhz code see tsc_timer */
/* Get the speed of the TSC timer in MHz */
unsigned __attribute__((no_instrument_function)) long get_tbclk_mhz(void)
{
	u32 ratio , bus_freq;
	u64 platform_info = native_read_msr(MSR_PLATFORM_INFO);
	u64 msr_fsb_freq = native_read_msr(MSR_FSB_FREQ);

	/* compute and correct ratio if necessary */
	ratio = ((platform_info >> 8) & 0xff);
	if(!ratio)
	{
		ratio = 4;
		debug("Read a zero ratio, force tsc ratio to 4 ...\n");
	}

	/* compute fsb */
	bus_freq = (u32) (msr_fsb_freq & 0x7);
	/* lookup real bus freq in kHz according to its index */
	switch(bus_freq)
	{
	case 2:
		bus_freq = FSB_FREQ_133SKU;
		break;
	case 3:
		bus_freq = FSB_FREQ_167SKU;
		break;
	case 4:
		bus_freq = FSB_FREQ_83SKU;
		break;
	case 5:
		bus_freq = FSB_FREQ_400SKU;
		break;
	case 6:
		bus_freq = FSB_FREQ_267SKU;
		break;
	case 7:
		bus_freq = FSB_FREQ_333SKU;
		break;
	default:  /* handle also 0 and 1 */
		bus_freq = FSB_FREQ_100SKU;
		break;
	}
	// return Freq in Mhz
	return ((bus_freq * ratio)/1000);
}

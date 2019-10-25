/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <platform.h>
#include <xlat_tables.h>

/*
 * Table of regions to map using the MMU.
 * This doesn't include TZRAM as the 'mem_layout' argument passed to
 * configure_mmu_elx() will give the available subset of that,
 */
const mmap_region_t plat_arm_mmap[] = {
	{ DEVICE0_BASE, DEVICE0_BASE, DEVICE0_SIZE, MT_DEVICE | MT_RW | MT_SECURE },
	{ DEVICE1_BASE, DEVICE1_BASE, DEVICE1_SIZE, MT_DEVICE | MT_RW | MT_SECURE },
	{0}
};

unsigned int plat_get_syscnt_freq2(void)
{
        /* Frequency of the HPSC Elapsed Timer, which drives the per-cpu
         * Generic Timers. Value not available via any metainfo reg afaik. */
        return 125000000;
}

#if HPSC_WARM_RESTART
#include <gic_common.h>
#include <gicv2.h>

/*
 * This function returns the type of the highest priority pending interrupt
 * at the Interrupt controller. In the case of GICv2, the Highest Priority
 * Pending interrupt register (`GICC_HPPIR`) is read to determine the id of
 * the pending interrupt. The type of interrupt depends upon the id value
 * as follows.
 *   1. id < PENDING_G1_INTID (1022) is reported as a EL3 interrupt
 *   2. id = PENDING_G1_INTID (1022) is reported as a Non-secure interrupt.
 *   3. id = GIC_SPURIOUS_INTERRUPT (1023) is reported as an invalid interrupt
 *           type.
 */
uint32_t plat_ic_get_pending_interrupt_type(void)
{
	unsigned int id;

	id = gicv2_get_pending_interrupt_type();

	/* Assume that all secure interrupts are S-EL1 interrupts */
	if (id < PENDING_G1_INTID)
		return INTR_TYPE_EL3;

	if (id == GIC_SPURIOUS_INTERRUPT)
		return INTR_TYPE_INVAL;

	return INTR_TYPE_NS;
}
#endif

/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <generic_delay_timer.h>
#include <mmio.h>
#include <platform.h>
#include <stdbool.h>
#include <string.h>
#include <xlat_tables.h>
#include "../hpsc_private.h"
#include "pm_api_sys.h"

/*
 * Table of regions to map using the MMU.
 * This doesn't include TZRAM as the 'mem_layout' argument passed to
 * configure_mmu_elx() will give the available subset of that,
 */
const mmap_region_t plat_arm_mmap[] = {
	{ DEVICE0_BASE, DEVICE0_BASE, DEVICE0_SIZE, MT_DEVICE | MT_RW | MT_SECURE },
	{ DEVICE1_BASE, DEVICE1_BASE, DEVICE1_SIZE, MT_DEVICE | MT_RW | MT_SECURE },
	{ CRF_APB_BASE, CRF_APB_BASE, CRF_APB_SIZE, MT_DEVICE | MT_RW | MT_SECURE },
	{0}
};

static unsigned int hpsc_get_silicon_ver(void)
{
	static unsigned int ver;

	if (!ver) {
		ver = mmio_read_32(HPSC_CSU_BASEADDR +
				   HPSC_CSU_VERSION_OFFSET);
		ver &= HPSC_SILICON_VER_MASK;
		ver >>= HPSC_SILICON_VER_SHIFT;
	}

	return ver;
}

unsigned int hpsc_get_uart_clk(void)
{
	unsigned int ver = hpsc_get_silicon_ver();

	switch (ver) {
	case HPSC_CSU_VERSION_VELOCE:
		return 48000;
	case HPSC_CSU_VERSION_EP108:
		return 25000000;
	case HPSC_CSU_VERSION_QEMU:
		return 133000000;
	}

	return 100000000;
}

#if LOG_LEVEL >= LOG_LEVEL_NOTICE
static const struct {
	unsigned int id;
	unsigned int ver;
	char *name;
	bool evexists;
} hpsc_devices[] = {
	{
		.id = 0x00,
		.name = "NONAME",
	},
};

#define HPSC_PL_STATUS_BIT	9
#define HPSC_PL_STATUS_MASK	BIT(HPSC_PL_STATUS_BIT)
#define HPSC_CSU_VERSION_MASK	~(HPSC_PL_STATUS_MASK)

static char *hpsc_get_silicon_idcode_name(void)
{
	uint32_t id, ver, chipid[2];
	size_t i, j, len;
	enum pm_ret_status ret;
	const char *name = "EG/EV";

	ret = pm_get_chipid(chipid);
	if (ret)
		return "UNKN";

	id = chipid[0] & (HPSC_CSU_IDCODE_DEVICE_CODE_MASK |
			  HPSC_CSU_IDCODE_SVD_MASK);
	id >>= HPSC_CSU_IDCODE_SVD_SHIFT;
	ver = chipid[1] >> HPSC_EFUSE_IPDISABLE_SHIFT;

	for (i = 0; i < ARRAY_SIZE(hpsc_devices); i++) {
		if (hpsc_devices[i].id == id &&
		    hpsc_devices[i].ver == (ver & HPSC_CSU_VERSION_MASK))
			break;
	}

	if (i >= ARRAY_SIZE(hpsc_devices))
		return "UNKN";

	if (!hpsc_devices[i].evexists)
		return hpsc_devices[i].name;

	if (ver & HPSC_PL_STATUS_MASK)
		return hpsc_devices[i].name;

	len = strlen(hpsc_devices[i].name) - 2;
	for (j = 0; j < strlen(name); j++) {
		hpsc_devices[i].name[len] = name[j];
		len++;
	}
	hpsc_devices[i].name[len] = '\0';

	return hpsc_devices[i].name;
}

static unsigned int hpsc_get_rtl_ver(void)
{
	uint32_t ver;

	ver = mmio_read_32(HPSC_CSU_BASEADDR + HPSC_CSU_VERSION_OFFSET);
	ver &= HPSC_RTL_VER_MASK;
	ver >>= HPSC_RTL_VER_SHIFT;

	return ver;
}

static char *hpsc_print_silicon_idcode(void)
{
	uint32_t id, maskid, tmp;

	id = mmio_read_32(HPSC_CSU_BASEADDR + HPSC_CSU_IDCODE_OFFSET);

	tmp = id;
	tmp &= HPSC_CSU_IDCODE_HPSC_ID_MASK |
	       HPSC_CSU_IDCODE_FAMILY_MASK;
	maskid = HPSC_CSU_IDCODE_HPSC_ID << HPSC_CSU_IDCODE_HPSC_ID_SHIFT |
		 HPSC_CSU_IDCODE_FAMILY << HPSC_CSU_IDCODE_FAMILY_SHIFT;
	if (tmp != maskid) {
		ERROR("Incorrect HPSC IDCODE 0x%x, maskid 0x%x\n", id, maskid);
		return "UNKN";
	}
	VERBOSE("HPSC IDCODE 0x%x\n", id);
	return hpsc_get_silicon_idcode_name();
}

static unsigned int hpsc_get_ps_ver(void)
{
	uint32_t ver = mmio_read_32(HPSC_CSU_BASEADDR + HPSC_CSU_VERSION_OFFSET);

	ver &= HPSC_PS_VER_MASK;
	ver >>= HPSC_PS_VER_SHIFT;

	return ver + 1;
}

static void hpsc_print_platform_name(void)
{
	unsigned int ver = hpsc_get_silicon_ver();
	unsigned int rtl = hpsc_get_rtl_ver();
	char *label = "Unknown";

	switch (ver) {
	case HPSC_CSU_VERSION_VELOCE:
		label = "VELOCE";
		break;
	case HPSC_CSU_VERSION_EP108:
		label = "EP108";
		break;
	case HPSC_CSU_VERSION_QEMU:
		label = "QEMU";
		break;
	case HPSC_CSU_VERSION_SILICON:
		label = "silicon";
		break;
	}

	NOTICE("ATF running on HPSC %s/%s v%d/RTL%d.%d at 0x%x\n",
	       hpsc_print_silicon_idcode(), label, hpsc_get_ps_ver(),
	       (rtl & 0xf0) >> 4, rtl & 0xf, BL31_BASE);
}
#else
static inline void hpsc_print_platform_name(void) { }
#endif

unsigned int hpsc_get_bootmode(void)
{
	uint32_t r;
	unsigned int ret;

	ret = pm_mmio_read(CRL_APB_BOOT_MODE_USER, &r);

	if (ret != PM_RET_SUCCESS) {
		r = mmio_read_32(CRL_APB_BOOT_MODE_USER);
	}

	return r & CRL_APB_BOOT_MODE_MASK;
}

void hpsc_config_setup(void)
{
	hpsc_print_platform_name();
	generic_delay_timer_init();
}

unsigned int plat_get_syscnt_freq2(void)
{
	unsigned int ver = hpsc_get_silicon_ver();

	switch (ver) {
	case HPSC_CSU_VERSION_VELOCE:
		return 10000;
	case HPSC_CSU_VERSION_EP108:
		return 4000000;
	case HPSC_CSU_VERSION_QEMU:
		return 50000000;
	}

	return mmio_read_32(IOU_SCNTRS_BASEFREQ);
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

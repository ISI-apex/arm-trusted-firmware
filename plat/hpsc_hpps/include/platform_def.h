/*
 * Copyright (c) 2014-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <arch.h>
#include <gic_common.h>
#include <interrupt_props.h>
#include "hpsc_def.h"

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/

/* Size of cacheable stacks */
#define PLATFORM_STACK_SIZE 0x440

#define HPSC_PWR_DOMAINS_AT_MAX_PWR_LVL 1
#define HPSC_CLUSTER_COUNT 2
#define HPSC_CLUSTER0_CORE_COUNT 4
#define HPSC_CLUSTER1_CORE_COUNT 4

#define PLATFORM_CORE_COUNT		(HPSC_CLUSTER0_CORE_COUNT + HPSC_CLUSTER1_CORE_COUNT)
#define PLAT_NUM_PWR_DOMAINS		(HPSC_PWR_DOMAINS_AT_MAX_PWR_LVL + HPSC_CLUSTER_COUNT + PLATFORM_CORE_COUNT )
#define PLAT_MAX_PWR_LVL		2
#define PLAT_MAX_RET_STATE		1
#define PLAT_MAX_OFF_STATE		2

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL31 at the top of the Trusted SRAM (just below the shared memory, if
 * present). BL31_BASE is calculated using the current BL31 debug size plus a
 * little space for growth.
 */
#ifndef HPSC_ATF_MEM_BASE
# define BL31_BASE			0x80000000
# define BL31_LIMIT			0x8001ffff
#else
# define BL31_BASE			(HPSC_ATF_MEM_BASE)
# define BL31_LIMIT			(HPSC_ATF_MEM_BASE + HPSC_ATF_MEM_SIZE - 1)
# ifdef HPSC_ATF_MEM_PROGBITS_SIZE
#  define BL31_PROGBITS_LIMIT		(HPSC_ATF_MEM_BASE + HPSC_ATF_MEM_PROGBITS_SIZE - 1)
# endif
#endif

/* Next image to jump to after ATF */
#ifndef HPSC_NEXT_IMAGE_BASE
#define PLAT_ARM_NS_IMAGE_OFFSET	((BL31_LIMIT) + 1)
#else
#define PLAT_ARM_NS_IMAGE_OFFSET	(HPSC_NEXT_IMAGE_BASE)
#endif

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 32)
#define MAX_MMAP_REGIONS		7
#define MAX_XLAT_TABLES			5

#define CACHE_WRITEBACK_SHIFT   6
#define CACHE_WRITEBACK_GRANULE (1 << CACHE_WRITEBACK_SHIFT)

#define PLAT_ARM_GICD_BASE	BASE_GICD_BASE

/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLAT_ARM_G1S_IRQ_PROPS(grp)	{ARM_IRQ_SEC_PHY_TIMER, 0x0, grp, 0x0},	\
					{ARM_IRQ_SEC_SGI_1,     0x0, grp, 0x0},	\
					{ARM_IRQ_SEC_SGI_2,     0x0, grp, 0x0},	\
					{ARM_IRQ_SEC_SGI_3,     0x0, grp, 0x0},	\
					{ARM_IRQ_SEC_SGI_4,     0x0, grp, 0x0},	\
					{ARM_IRQ_SEC_SGI_5,     0x0, grp, 0x0},	\
					{ARM_IRQ_SEC_SGI_7,     0x0, grp, 0x0}

#define PLAT_ARM_G0_IRQ_PROPS(grp)	{ARM_IRQ_SEC_SGI_0, 0x0, grp, 0x0}, \
					{ARM_IRQ_SEC_SGI_0, 0x0, grp, 0x0}

#endif /* __PLATFORM_DEF_H__ */

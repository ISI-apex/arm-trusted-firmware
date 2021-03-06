/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <bl31.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <errno.h>
#include <plat_arm.h>
#include <platform.h>
#include <generic_delay_timer.h>
#include <uart_16550.h>

#define BL31_END (unsigned long)(&__BL31_END__)

static entry_point_info_t next_image_ep_info;

static void hpsc_print_platform_name(void)
{
	unsigned int ver = 0;

	NOTICE("ATF running on HPSC v%d at 0x%lx\n",
	       ver, (unsigned long)(BL31_BASE));
}

/*
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. On the HPSC platform, we support the next
 * image to be nonsecure only. A NULL pointer is returned if the image does not
 * exist.
 */
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	assert(sec_state_is_valid(type));
	if (type == NON_SECURE)
		return &next_image_ep_info;
	return NULL;
}

/*
 * Perform any BL31 specific platform actions. Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & S-EL3 in BL1) before they
 * are lost (potentially). This needs to be done before the MMU is initialized
 * so that the memory layout can be used while creating page tables.
 */
void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
                                u_register_t arg2, u_register_t arg3)
{
	static console_16550_t console;
#if DEBUG
	void *from_bl2 = (void *)arg0;
	void *plat_params_from_bl2 = (void *)arg1;
#endif

	/* Initialize the console to provide early debug support */
	console_16550_register(HPSC_UART_BASE, HPSC_UART_CLOCK,
			       HPSC_UART_BAUDRATE, &console);

	hpsc_print_platform_name();
	generic_delay_timer_init();

	/* There are no parameters from BL2 if BL31 is a reset vector */
	assert(from_bl2 == NULL);
	assert(plat_params_from_bl2 == NULL);

	SET_PARAM_HEAD(&next_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(next_image_ep_info.h.attr, NON_SECURE);

	next_image_ep_info.pc = plat_get_ns_image_entrypoint();
	next_image_ep_info.spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
					  DISABLE_ALL_EXCEPTIONS);

	NOTICE("BL31: next image entry point: 0x%lx\n", next_image_ep_info.pc);
}

/* Enable the test setup */
#ifndef HPSC_TESTING
static void hpsc_testing_setup(void) { }
#else
static void hpsc_testing_setup(void)
{
	uint32_t actlr_el3, actlr_el2;

	/* Enable CPU ACTLR AND L2ACTLR RW access from non-secure world */
	actlr_el3 = read_actlr_el3();
	actlr_el2 = read_actlr_el2();

	actlr_el3 |= ACTLR_EL3_L2ACTLR_BIT | ACTLR_EL3_CPUACTLR_BIT;
	actlr_el2 |= ACTLR_EL3_L2ACTLR_BIT | ACTLR_EL3_CPUACTLR_BIT;
	write_actlr_el3(actlr_el3);
	write_actlr_el2(actlr_el2);
}
#endif

#if HPSC_WARM_RESTART
static interrupt_type_handler_t type_el3_interrupt_table[MAX_INTR_EL3];

/* Register INTR_TYPE_EL3 interrupt handler to specific GIC entrance */
int request_intr_type_el3(uint32_t id, interrupt_type_handler_t handler)
{
	/* Validate 'handler' and 'id' parameters */
	if (!handler || id >= MAX_INTR_EL3)
		return -EINVAL;

	/* Check if a handler has already been registered */
	if (type_el3_interrupt_table[id])
		return -EALREADY;

	type_el3_interrupt_table[id] = handler;

	return 0;
}

static uint64_t rdo_el3_interrupt_handler(uint32_t id, uint32_t flags,
					  void *handle, void *cookie)
{
	uint32_t intr_id;
	interrupt_type_handler_t handler;

	intr_id = plat_ic_get_pending_interrupt_id();
	handler = type_el3_interrupt_table[intr_id];
	if (handler != NULL)
		handler(intr_id, flags, handle, cookie);

	return 0;
}
#endif

void bl31_platform_setup(void)
{
	/* Initialize the gic cpu and distributor interfaces */
	plat_arm_gic_driver_init();
	plat_arm_gic_init();
	hpsc_testing_setup();
}

void bl31_plat_runtime_setup(void)
{

#if HPSC_WARM_RESTART
	uint64_t flags = 0;
	uint64_t rc;

	set_interrupt_rm_flag(flags, NON_SECURE);
	rc = register_interrupt_type_handler(INTR_TYPE_EL3,
					rdo_el3_interrupt_handler, flags);
	if (rc)
		panic();
#endif

}

/*
 * Perform the very early platform specific architectural setup here.
 */
void bl31_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
	    MAP_REGION_FLAT(BL31_BASE, BL31_END - BL31_BASE,
		    MT_MEMORY | MT_RW | MT_SECURE),
	    MAP_REGION_FLAT(BL_CODE_BASE, BL_CODE_END - BL_CODE_BASE,
		    MT_CODE | MT_SECURE),
	    MAP_REGION_FLAT(BL_RO_DATA_BASE, BL_RO_DATA_END - BL_RO_DATA_BASE,
		    MT_RO_DATA | MT_SECURE),
	    MAP_REGION_FLAT(BL_COHERENT_RAM_BASE,
		    BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE,
		    MT_DEVICE | MT_RW | MT_SECURE),
	    {0}
	};

	setup_page_tables(bl_regions, plat_arm_get_mmap());
	enable_mmu_el3(0);
}

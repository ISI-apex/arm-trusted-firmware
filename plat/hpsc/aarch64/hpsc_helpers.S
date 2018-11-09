/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <gicv2.h>
#include <gicv3.h>
#include <platform_def.h>

	.globl	plat_secondary_cold_boot_setup
	.globl	plat_is_my_cpu_primary
	.globl  plat_arm_calc_core_pos
	.globl  plat_my_core_pos
	.globl	platform_mem_init

	/* -----------------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * This function performs any platform specific actions
	 * needed for a secondary cpu after a cold reset e.g
	 * mark the cpu's presence, mechanism to place it in a
	 * holding pen etc.
	 * TODO: Should we read the PSYS register to make sure
	 * that the request has gone through.
	 * -----------------------------------------------------
	 */
func plat_secondary_cold_boot_setup
#ifndef EL3_PAYLOAD_BASE
	/* ---------------------------------------------
	 * Disable GIC bypass as well
	 * ---------------------------------------------
	 */
	/* Check for GICv3 system register access */
	mrs	x0, id_aa64pfr0_el1
	ubfx	x0, x0, #ID_AA64PFR0_GIC_SHIFT, #ID_AA64PFR0_GIC_WIDTH
	cmp	x0, #1
	b.ne	gicv2_bypass_disable

	/* Check for SRE enable */
	mrs	x1, ICC_SRE_EL3
	tst	x1, #ICC_SRE_SRE_BIT
	b.eq	gicv2_bypass_disable

	mrs	x2, ICC_SRE_EL3
	orr	x2, x2, #(ICC_SRE_DIB_BIT | ICC_SRE_DFB_BIT)
	msr	ICC_SRE_EL3, x2
	b	secondary_cold_boot_wait

gicv2_bypass_disable:
	/* Deactivate the gic cpu interface */
	ldr	x1, =BASE_GICC_BASE
	mov	w0, #(IRQ_BYP_DIS_GRP1 | FIQ_BYP_DIS_GRP1)
	orr	w0, w0, #(IRQ_BYP_DIS_GRP0 | FIQ_BYP_DIS_GRP0)
	str	w0, [x1, #GICC_CTLR]
secondary_cold_boot_wait:
	/*
	 * There is no sane reason to come out of this wfi. This
	 * cpu will be powered on and reset by the cpu_on pm api
	 */
	dsb	sy
	wfi
	bl	plat_panic_handler
#else
	mov_imm	x0, PLAT_ARM_TRUSTED_MAILBOX_BASE

	/* Wait until the entrypoint gets populated */
poll_mailbox:
	ldr	x1, [x0]
	cbz	x1, 1f
	br	x1
1:
	wfe
	b	poll_mailbox
#endif /* EL3_PAYLOAD_BASE */
endfunc plat_secondary_cold_boot_setup

	/* -----------------------------------------------------
	 *  unsigned int plat_arm_calc_core_pos(u_register_t mpidr)
	 *  Helper function to calculate the core position.
	 *  With this function: CorePos = (ClusterId * 4) +
	 *  				  CoreId
	 * -----------------------------------------------------
	 */
func plat_arm_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_arm_calc_core_pos


	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *  This function uses the plat_arm_calc_core_pos()
	 *  definition to get the index of the calling CPU.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_arm_calc_core_pos
endfunc plat_my_core_pos

func plat_is_my_cpu_primary
	mov	x9, x30
	bl	plat_my_core_pos
	cmp	x0, #HPSC_PRIMARY_CPU
	cset	x0, eq
	ret	x9
endfunc plat_is_my_cpu_primary

	/* ---------------------------------------------------------------------
	 * We don't need to carry out any memory initialization on ARM
	 * platforms. The Secure RAM is accessible straight away.
	 * ---------------------------------------------------------------------
	 */
func platform_mem_init
	ret
endfunc platform_mem_init

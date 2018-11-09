/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <errno.h>
#include <gicv3.h>
#include <mmio.h>
#include <plat_arm.h>
#include <platform.h>
#include <psci.h>
#include "pm_api_sys.h"
#include "pm_client.h"
#include "hpsc_private.h"

uintptr_t hpsc_sec_entry;

void hpsc_cpu_standby(plat_local_state_t cpu_state)
{
	VERBOSE("%s: cpu_state: 0x%x\n", __func__, cpu_state);

	dsb();
	wfi();
}

static int hpsc_pwr_domain_on(u_register_t mpidr)
{
	unsigned int cpu_id = plat_core_pos_by_mpidr(mpidr);
	const struct pm_proc *proc;
	VERBOSE("%s: cpu_id(0x%x):  mpidr: 0x%lx\n", __func__, cpu_id, mpidr);
	if (cpu_id >= 0x100) cpu_id = cpu_id - 0x100 + 4;

	if (cpu_id == -1)
		return PSCI_E_INTERN_FAIL;

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
	int kk;
	aff_info_state_t kk2[8];
	for (kk = 0; kk < 8; kk++) {
		kk2[kk] = get_cpu_data_by_index(kk,psci_svc_cpu_data.aff_info_state); 
	}
	VERBOSE("%s: power state = (%d, %d, %d, %d, %d, %d, %d, %d)\n",
		__func__, kk2[0], kk2[1], kk2[2], kk2[3], kk2[4], kk2[5], kk2[6], kk2[7]); 
#endif
	proc = pm_get_proc(cpu_id);
	/* Clear power down request */
#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
	for (kk = 0; kk < 8; kk++) {
		kk2[kk] = get_cpu_data_by_index(kk,psci_svc_cpu_data.aff_info_state); 
	}
	VERBOSE("%s: power state = (%d, %d, %d, %d, %d, %d, %d, %d)\n",
		__func__, kk2[0], kk2[1], kk2[2], kk2[3], kk2[4], kk2[5], kk2[6], kk2[7]); 
#endif
	pm_client_wakeup(proc);
#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
	for (kk = 0; kk < 8; kk++) {
		kk2[kk] = get_cpu_data_by_index(kk,psci_svc_cpu_data.aff_info_state); 
	}
	VERBOSE("%s: power state = (%d, %d, %d, %d, %d, %d, %d, %d)\n",
		__func__, kk2[0], kk2[1], kk2[2], kk2[3], kk2[4], kk2[5], kk2[6], kk2[7]); 
#endif

	/* Send request to PMU to wake up selected APU CPU core */
	pm_req_wakeup(proc->node_id, 1, hpsc_sec_entry, REQ_ACK_BLOCKING);
#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
	for (kk = 0; kk < 8; kk++) {
		kk2[kk] = get_cpu_data_by_index(kk,psci_svc_cpu_data.aff_info_state); 
	}
	VERBOSE("%s: power state = (%d, %d, %d, %d, %d, %d, %d, %d)\n",
		__func__, kk2[0], kk2[1], kk2[2], kk2[3], kk2[4], kk2[5], kk2[6], kk2[7]); 
#endif

	return PSCI_E_SUCCESS;
}

static void hpsc_pwr_domain_off(const psci_power_state_t *target_state)
{
	unsigned int cpu_id = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpu_id);

	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++)
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);

	/* Prevent interrupts from spuriously waking up this cpu */
	gicv3_cpuif_disable(plat_my_core_pos());

	/*
	 * Send request to PMU to power down the appropriate APU CPU
	 * core.
	 * According to PSCI specification, CPU_off function does not
	 * have resume address and CPU core can only be woken up
	 * invoking CPU_on function, during which resume address will
	 * be set.
	 */
	pm_self_suspend(proc->node_id, MAX_LATENCY, PM_STATE_CPU_IDLE, 0);
}

static void hpsc_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	unsigned int state;
	unsigned int cpu_id = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpu_id);

	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++)
		VERBOSE("%s: cpu_id(0x%x): target_state->pwr_domain_state[%lu]=%x\n",
			__func__, cpu_id, i, target_state->pwr_domain_state[i]);

	state = target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE ?
		PM_STATE_SUSPEND_TO_RAM : PM_STATE_CPU_IDLE;

	/* Send request to PMU to suspend this core */
	pm_self_suspend(proc->node_id, MAX_LATENCY, state, hpsc_sec_entry);

#if PLAT_HAS_INTERCONNECT
	/* APU is to be turned off */
	if (target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE) {
		/* disable coherency */
		plat_arm_interconnect_exit_coherency();
	}
#endif // PLAT_HAS_INTERCONNECT
}

static void hpsc_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++)
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);

	gicv3_cpuif_enable(plat_my_core_pos());
	gicv3_rdistif_init(plat_my_core_pos());
}

static void hpsc_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	unsigned int cpu_id = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpu_id);

	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++)
		VERBOSE("%s: cpu(%d): target_state->pwr_domain_state[%lu]=%x\n",
			__func__, cpu_id, i, target_state->pwr_domain_state[i]);

	/* Clear the APU power control register for this cpu */
	pm_client_wakeup(proc);

#if PLAT_HAS_INTERCONNECT
	/* enable coherency */
	plat_arm_interconnect_enter_coherency();
#endif // PLAT_HAS_INTERCONNECT
	/* APU was turned off */
	if (target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE) {	/* > 1 */
		VERBOSE("%s: cpu(%d): plat_arm_gic_init()\n", __func__, cpu_id);
		plat_arm_gic_init();
	} else {
		VERBOSE("%s: cpu(%d): gicv3_cpuif_enable()\n", __func__, cpu_id);
		gicv3_cpuif_enable(plat_my_core_pos());
		gicv3_rdistif_init(plat_my_core_pos());
	}
}

/*******************************************************************************
 * ZynqMP handlers to shutdown/reboot the system
 ******************************************************************************/

static void __dead2 hpsc_system_off(void)
{
#if PLAT_HAS_INTERCONNECT
	/* disable coherency */
	plat_arm_interconnect_exit_coherency();
#endif // PLAT_HAS_INTERCONNECT

	/* Send the power down request to the PMU */
	pm_system_shutdown(PMF_SHUTDOWN_TYPE_SHUTDOWN,
			   pm_get_shutdown_scope());

	while (1)
		wfi();
}

static void __dead2 hpsc_system_reset(void)
{
#if PLAT_HAS_INTERCONNECT
	/* disable coherency */
	plat_arm_interconnect_exit_coherency();
#endif // PLAT_HAS_INTERCONNECT

	/* Send the system reset request to the PMU */
	pm_system_shutdown(PMF_SHUTDOWN_TYPE_RESET,
			   pm_get_shutdown_scope());

	while (1)
		wfi();
}

int hpsc_validate_power_state(unsigned int power_state,
				psci_power_state_t *req_state)
{
	VERBOSE("%s: power_state: 0x%x\n", __func__, power_state);

	int pstate = psci_get_pstate_type(power_state);

	assert(req_state);

	/* Sanity check the requested state */
	if (pstate == PSTATE_TYPE_STANDBY)
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = PLAT_MAX_RET_STATE;
	else
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = PLAT_MAX_OFF_STATE;

	/* We expect the 'state id' to be zero */
	if (psci_get_pstate_id(power_state))
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

int hpsc_validate_ns_entrypoint(unsigned long ns_entrypoint)
{
	VERBOSE("%s: ns_entrypoint: 0x%lx\n", __func__, ns_entrypoint);

	/* FIXME: Actually validate */
	return PSCI_E_SUCCESS;
}

void hpsc_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	req_state->pwr_domain_state[PSCI_CPU_PWR_LVL] = PLAT_MAX_OFF_STATE;
	req_state->pwr_domain_state[1] = PLAT_MAX_OFF_STATE;
}

/*******************************************************************************
 * Export the platform handlers to enable psci to invoke them
 ******************************************************************************/
static const struct plat_psci_ops hpsc_psci_ops = {
	.cpu_standby			= hpsc_cpu_standby,
	.pwr_domain_on			= hpsc_pwr_domain_on,
	.pwr_domain_off			= hpsc_pwr_domain_off,
	.pwr_domain_suspend		= hpsc_pwr_domain_suspend,
	.pwr_domain_on_finish		= hpsc_pwr_domain_on_finish,
	.pwr_domain_suspend_finish	= hpsc_pwr_domain_suspend_finish,
	.system_off			= hpsc_system_off,
	.system_reset			= hpsc_system_reset,
	.validate_power_state		= hpsc_validate_power_state,
	.validate_ns_entrypoint		= hpsc_validate_ns_entrypoint,
	.get_sys_suspend_power_state	= hpsc_get_sys_suspend_power_state,
};

/*******************************************************************************
 * Export the platform specific power ops.
 ******************************************************************************/
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const struct plat_psci_ops **psci_ops)
{
	hpsc_sec_entry = sec_entrypoint;

	*psci_ops = &hpsc_psci_ops;

	return 0;
}

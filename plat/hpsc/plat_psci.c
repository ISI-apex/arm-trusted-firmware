/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <errno.h>
#include <string.h>
#include <gicv3.h>
#include <mmio.h>
#include <utils.h>
#include <plat_arm.h>
#include <platform.h>
#include <psci.h>
#include "pm_ipi.h"
#include "hpsc_def.h"
#include "include/platform_def.h"

#define IRQ_MAX		84
#define NUM_GICD_ISENABLER	((IRQ_MAX >> 5) + 1)
#define UNDEFINED_CPUID		(~0)
#define MAX_LATENCY	(~0U)

/* State arguments of the self suspend */
#define PM_STATE_CPU_IDLE		0x0U
#define PM_STATE_SUSPEND_TO_RAM		0xFU

/**
 * Assigning of argument values into array elements.
 */
#define PM_PACK_PAYLOAD1(pl, arg0) {	\
	pl[0] = (uint32_t)(arg0);	\
}

#define PM_PACK_PAYLOAD2(pl, arg0, arg1) {	\
	pl[1] = (uint32_t)(arg1);		\
	PM_PACK_PAYLOAD1(pl, arg0);		\
}

#define PM_PACK_PAYLOAD3(pl, arg0, arg1, arg2) {	\
	pl[2] = (uint32_t)(arg2);			\
	PM_PACK_PAYLOAD2(pl, arg0, arg1);		\
}

#define PM_PACK_PAYLOAD4(pl, arg0, arg1, arg2, arg3) {	\
	pl[3] = (uint32_t)(arg3);			\
	PM_PACK_PAYLOAD3(pl, arg0, arg1, arg2);		\
}

#define PM_PACK_PAYLOAD5(pl, arg0, arg1, arg2, arg3, arg4) {	\
	pl[4] = (uint32_t)(arg4);				\
	PM_PACK_PAYLOAD4(pl, arg0, arg1, arg2, arg3);		\
}

#define PM_PACK_PAYLOAD6(pl, arg0, arg1, arg2, arg3, arg4, arg5) {	\
	pl[5] = (uint32_t)(arg5);					\
	PM_PACK_PAYLOAD5(pl, arg0, arg1, arg2, arg3, arg4);		\
}

enum pm_api_id {
	/* Miscellaneous API functions: */
	PM_GET_API_VERSION = 1, /* Do not change or move */
	PM_SET_CONFIGURATION,
	PM_GET_NODE_STATUS,
	PM_GET_OP_CHARACTERISTIC,
	PM_REGISTER_NOTIFIER,
	/* API for suspending of PUs: */
	PM_REQ_SUSPEND,
	PM_SELF_SUSPEND,
	PM_FORCE_POWERDOWN,
	PM_ABORT_SUSPEND,
	PM_REQ_WAKEUP,
	PM_SET_WAKEUP_SOURCE,
	PM_SYSTEM_SHUTDOWN,
	PM_API_MAX
};

enum pm_request_ack {
	REQ_ACK_NO = 1,
	REQ_ACK_BLOCKING,
	REQ_ACK_NON_BLOCKING,
};

/**
 * @PMF_SHUTDOWN_TYPE_SHUTDOWN:		shutdown
 * @PMF_SHUTDOWN_TYPE_RESET:		reset/reboot
 * @PMF_SHUTDOWN_TYPE_SETSCOPE_ONLY:	set the shutdown/reboot scope
 */
enum pm_shutdown_type {
	PMF_SHUTDOWN_TYPE_SHUTDOWN,
	PMF_SHUTDOWN_TYPE_RESET,
	PMF_SHUTDOWN_TYPE_SETSCOPE_ONLY,
};

/**
 * @PMF_SHUTDOWN_SUBTYPE_SUBSYSTEM:	shutdown/reboot APU subsystem only
 * @PMF_SHUTDOWN_SUBTYPE_PS_ONLY:	shutdown/reboot entire PS (but not PL)
 * @PMF_SHUTDOWN_SUBTYPE_SYSTEM:	shutdown/reboot entire system
 */
enum pm_shutdown_subtype {
	PMF_SHUTDOWN_SUBTYPE_SUBSYSTEM,
	PMF_SHUTDOWN_SUBTYPE_PS_ONLY,
	PMF_SHUTDOWN_SUBTYPE_SYSTEM,
};

/* Defined by each HPSC subsystem */
const struct pm_proc *pm_get_proc(unsigned int cpuid);
extern const struct pm_proc *primary_proc;

static uintptr_t hpsc_sec_entry;

/* default shutdown/reboot scope is system(2) */
static unsigned int pm_shutdown_scope = PMF_SHUTDOWN_SUBTYPE_SYSTEM;

/**
 * pm_get_shutdown_scope() - Get the currently set shutdown scope
 *
 * @return	Shutdown scope value
 */
static unsigned int pm_get_shutdown_scope()
{
	return pm_shutdown_scope;
}

/**
 * pm_self_suspend() - PM call for processor to suspend itself
 * @nid		Node id of the processor or subsystem
 * @latency	Requested maximum wakeup latency (not supported)
 * @state	Requested state
 * @address	Resume address
 *
 * This is a blocking call, it will return only once PMU has responded.
 * On a wakeup, resume address will be automatically set by PMU.
 *
 * @return	Returns status, either success or error+reason
 */
static enum pm_ret_status pm_self_suspend(enum pm_node_id nid,
				   unsigned int latency,
				   unsigned int state,
				   uintptr_t address)
{
	uint32_t payload[PAYLOAD_ARG_CNT];
	unsigned int cpuid = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpuid);

	/* Send request to the PMU */
	PM_PACK_PAYLOAD6(payload, PM_SELF_SUSPEND, proc->node_id, latency,
			 state, address, (address >> 32));
	return pm_ipi_send_sync(proc, payload, NULL, 0);
}

#if 0 /* unused, but keeping for symmetry with wakeup */
/**
 * pm_req_suspend() - PM call to request for another PU or subsystem to
 *		      be suspended gracefully.
 * @target	Node id of the targeted PU or subsystem
 * @ack		Flag to specify whether acknowledge is requested
 * @latency	Requested wakeup latency (not supported)
 * @state	Requested state (not supported)
 *
 * @return	Returns status, either success or error+reason
 */
static enum pm_ret_status pm_req_suspend(enum pm_node_id target,
				  enum pm_request_ack ack,
				  unsigned int latency, unsigned int state)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD5(payload, PM_REQ_SUSPEND, target, ack, latency, state);
	if (ack == REQ_ACK_BLOCKING)
		return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
	else
		return pm_ipi_send(primary_proc, payload);
}
#endif

/**
 * pm_req_wakeup() - PM call for processor to wake up selected processor
 *		     or subsystem
 * @target	Node id of the processor or subsystem to wake up
 * @ack		Flag to specify whether acknowledge requested
 * @set_address	Resume address presence indicator
 *				1 resume address specified, 0 otherwise
 * @address	Resume address
 *
 * This API function is either used to power up another APU core for SMP
 * (by PSCI) or to power up an entirely different PU or subsystem, such
 * as RPU0, RPU, or PL_CORE_xx. Resume address for the target PU will be
 * automatically set by PMU.
 *
 * @return	Returns status, either success or error+reason
 */
static enum pm_ret_status pm_req_wakeup(enum pm_node_id target,
				 unsigned int set_address,
				 uintptr_t address,
				 enum pm_request_ack ack)
{
	uint32_t payload[PAYLOAD_ARG_CNT];
	uint64_t encoded_address;


	/* encode set Address into 1st bit of address */
	encoded_address = address;
	encoded_address |= !!set_address;

	/* Send request to the PMU to perform the wake of the PU */
	PM_PACK_PAYLOAD5(payload, PM_REQ_WAKEUP, target, encoded_address,
			 encoded_address >> 32, ack);

	if (ack == REQ_ACK_BLOCKING)
		return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
	else
		return pm_ipi_send(primary_proc, payload);
}

/**
 * pm_system_shutdown() - PM call to request a system shutdown or restart
 * @type	Shutdown or restart? 0=shutdown, 1=restart, 2=setscope
 * @subtype	Scope: 0=APU-subsystem, 1=PS, 2=system
 *
 * @return	Returns status, either success or error+reason
 */
static enum pm_ret_status pm_system_shutdown(unsigned int type, unsigned int subtype)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	if (type == PMF_SHUTDOWN_TYPE_SETSCOPE_ONLY) {
		/* Setting scope for subsequent PSCI reboot or shutdown */
		pm_shutdown_scope = subtype;
		return PM_RET_SUCCESS;
	}

	PM_PACK_PAYLOAD3(payload, PM_SYSTEM_SHUTDOWN, type, subtype);
	return pm_ipi_send_non_blocking(primary_proc, payload);
}

static void hpsc_cpu_standby(plat_local_state_t cpu_state)
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

	proc = pm_get_proc(cpu_id);
	/* Clear power down request */

	/* Send request to TRCH to wake up selected APU CPU core */
	pm_req_wakeup(proc->node_id, 1, hpsc_sec_entry, REQ_ACK_BLOCKING);

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

	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++)
		VERBOSE("%s: cpu(%d): target_state->pwr_domain_state[%lu]=%x\n",
			__func__, cpu_id, i, target_state->pwr_domain_state[i]);

#if PLAT_HAS_INTERCONNECT
	/* enable coherency */
	plat_arm_interconnect_enter_coherency();
#endif // PLAT_HAS_INTERCONNECT
	/* APU was turned off */
	if (target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE) {	/* > 1 */
		WARN("%s: cpu(%d): plat_arm_gic_init()\n", __func__, cpu_id);
		plat_arm_gic_init();
	} else {
		WARN("%s: cpu(%d): gicv3_cpuif_enable()\n", __func__, cpu_id);
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

static int hpsc_validate_power_state(unsigned int power_state,
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

static int hpsc_validate_ns_entrypoint(unsigned long ns_entrypoint)
{
	VERBOSE("%s: ns_entrypoint: 0x%lx\n", __func__, ns_entrypoint);

	/* FIXME: Actually validate */
	return PSCI_E_SUCCESS;
}

static void hpsc_get_sys_suspend_power_state(psci_power_state_t *req_state)
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
	int status = pm_ipi_init(primary_proc);
	assert(!status);

	hpsc_sec_entry = sec_entrypoint;

	*psci_ops = &hpsc_psci_ops;

	return status;
}

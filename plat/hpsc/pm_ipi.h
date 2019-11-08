/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PM_IPI_H_
#define _PM_IPI_H_

#define PAYLOAD_ARG_CNT		6U
#define PAYLOAD_ARG_SIZE	4U	/* size in bytes */

enum pm_node_id {
	NODE_UNKNOWN = 0,
	NODE_APU,
	NODE_APU_0,
	NODE_APU_1,
	NODE_APU_2,
	NODE_APU_3,
	NODE_APU_4,
	NODE_APU_5,
	NODE_APU_6,
	NODE_APU_7,
	NODE_RPU,
	NODE_RPU_0,
	NODE_RPU_1,
	NODE_RPU_A53,
	NODE_EXTERN,
	NODE_MAX
};

/**
 * pm_proc - struct for capturing processor related info
 * @node_id	node-ID of the processor
 * @pwrdn_mask	cpu-specific mask to be used for power control register
 * @ipi		pointer to IPI channel structure
 *		(in APU all processors share one IPI channel)
 */
struct pm_proc {
	const enum pm_node_id node_id;
	const unsigned int pwrdn_mask;
	const struct pm_ipi *ipi;
};

/**
 * @PM_RET_SUCCESS:		success
 * @PM_RET_ERROR_ARGS:		illegal arguments provided
 * @PM_RET_ERROR_ACCESS:	access rights violation
 * @PM_RET_ERROR_TIMEOUT:	timeout in communication with PMU
 * @PM_RET_ERROR_NOTSUPPORTED:	feature not supported
 * @PM_RET_ERROR_PROC:		node is not a processor node
 * @PM_RET_ERROR_API_ID:	illegal API ID
 * @PM_RET_ERROR_OTHER:		other error
 */
enum pm_ret_status {
	PM_RET_SUCCESS,
	PM_RET_ERROR_ARGS,
	PM_RET_ERROR_ACCESS,
	PM_RET_ERROR_TIMEOUT,
	PM_RET_ERROR_NOTSUPPORTED,
	PM_RET_ERROR_PROC,
	PM_RET_ERROR_API_ID,
	PM_RET_ERROR_FAILURE,
	PM_RET_ERROR_COMMUNIC,
	PM_RET_ERROR_DOUBLEREQ,
	PM_RET_ERROR_OTHER,
};


int pm_ipi_init(const struct pm_proc *proc);

enum pm_ret_status pm_ipi_send(const struct pm_proc *proc,
			       uint32_t payload[PAYLOAD_ARG_CNT]);
enum pm_ret_status pm_ipi_send_non_blocking(const struct pm_proc *proc,
					    uint32_t payload[PAYLOAD_ARG_CNT]);
enum pm_ret_status pm_ipi_send_sync(const struct pm_proc *proc,
				    uint32_t payload[PAYLOAD_ARG_CNT],
				    unsigned int *value, size_t count);
void pm_ipi_buff_read_callb(unsigned int *value, size_t count);
void pm_ipi_irq_enable(const struct pm_proc *proc);
void pm_ipi_irq_clear(const struct pm_proc *proc);
uint32_t pm_ipi_irq_status(const struct pm_proc *proc);

#endif /* _PM_IPI_H_ */

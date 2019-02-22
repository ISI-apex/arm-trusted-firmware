/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <bakery_lock.h>
#include <mmio.h>
#include <platform.h>
#include <arch_helpers.h>
#include "../hpsc_ipi.h"
#include "../hpsc_private.h"
#include "pm_common.h"
#include "pm_ipi.h"

#if TRCH_SERVER
#include <gicv3.h>
#include "gic.h"
#include "hwinfo.h"
#include "mailbox.h"
#include "mailbox-link.h"
#include "mailbox-map.h"
#include "hpsc-irqs.dtsh"
#include "command.h"
#endif

/* IPI message buffers */
#define IPI_BUFFER_BASEADDR	0xFF990000U

#define IPI_BUFFER_APU_BASE	(IPI_BUFFER_BASEADDR + 0x400U)
#define IPI_BUFFER_PMU_BASE	(IPI_BUFFER_BASEADDR + 0xE00U)

#define IPI_BUFFER_TARGET_APU_OFFSET	0x80U
#define IPI_BUFFER_TARGET_PMU_OFFSET	0x1C0U

#define IPI_BUFFER_MAX_WORDS	8

#define IPI_BUFFER_REQ_OFFSET	0x0U
#define IPI_BUFFER_RESP_OFFSET	0x20U

#define IPI_BLOCKING		1
#define IPI_NON_BLOCKING	0

DEFINE_BAKERY_LOCK(pm_secure_lock);

const struct pm_ipi apu_ipi = {
	.apu_ipi_id = IPI_ID_APU,
	.pmu_ipi_id = IPI_ID_PMU0,
	.buffer_base = IPI_BUFFER_APU_BASE,
};

#if TRCH_SERVER
#define HPPS_RCV_IRQ_IDX  MBOX_HPPS_TRCH__HPPS_RCV_ATF_INT	/* 28 */
#define HPPS_ACK_IRQ_IDX  MBOX_HPPS_TRCH__HPPS_ACK_ATF_INT	/* 29 */

struct link *trch_atf_link;
#endif

/**
 * pm_ipi_init() - Initialize IPI peripheral for communication with PMU
 *
 * @proc	Pointer to the processor who is initiating request
 * @return	On success, the initialization function must return 0.
 *		Any other return value will cause the framework to ignore
 *		the service
 *
 * Called from pm_setup initialization function
 */
int pm_ipi_init(const struct pm_proc *proc)
{
	bakery_lock_init(&pm_secure_lock);
#if TRCH_SERVER
	gic_init((volatile uint32_t *)HPPS_GIC_BASE);
	struct irq *hpps_rcv_irq = 
		gic_request(HPPS_IRQ__HT_MBOX_0 + HPPS_RCV_IRQ_IDX,
		 	GIC_IRQ_TYPE_SPI, GIC_IRQ_CFG_LEVEL);
	struct irq *hpps_ack_irq =
		gic_request(HPPS_IRQ__HT_MBOX_0 + HPPS_ACK_IRQ_IDX,
			GIC_IRQ_TYPE_SPI, GIC_IRQ_CFG_LEVEL);


	trch_atf_link = mbox_link_connect("TRCH_MBOX_ATF_LINK",
			MBOX_HPPS_TRCH__BASE,
			MBOX_HPPS_TRCH__TRCH_ATF_HPPS, MBOX_HPPS_TRCH__HPPS_ATF_TRCH,
			hpps_rcv_irq, HPPS_RCV_IRQ_IDX, /* 28 */
			hpps_ack_irq, HPPS_ACK_IRQ_IDX, /* 29 */
			0,
			MASTER_ID_HPPS_CPU0);
	if (!trch_atf_link)
	   VERBOSE("%s: panic: TRCH_MBOX_ATF_LINK failed to be created\n", __func__);
	else 
   	   VERBOSE("%s: successfully initialized TRCH_MBOX_ATF_LINK \n", __func__);
#endif

	ipi_mb_open(proc->ipi->apu_ipi_id, proc->ipi->pmu_ipi_id);

	return 0;
}

/**
 * pm_ipi_send_common() - Sends IPI request to the PMU
 * @proc	Pointer to the processor who is initiating request
 * @payload	API id and call arguments to be written in IPI buffer
 *
 * Send an IPI request to the power controller. Caller needs to hold
 * the 'pm_secure_lock' lock.
 *
 * @return	Returns status, either success or error+reason
 */
#if TRCH_SERVER
static enum pm_ret_status pm_ipi_send_common(const struct pm_proc *proc,
					     uint32_t payload[PAYLOAD_ARG_CNT],
					     uint32_t is_blocking,
				    	     unsigned int *value, 
					     size_t count)
#else
static enum pm_ret_status pm_ipi_send_common(const struct pm_proc *proc,
					     uint32_t payload[PAYLOAD_ARG_CNT],
					     uint32_t is_blocking)
#endif
{
	unsigned int offset = 0;
	uintptr_t buffer_base = proc->ipi->buffer_base +
					IPI_BUFFER_TARGET_PMU_OFFSET +
					IPI_BUFFER_REQ_OFFSET;
	/* Write payload into IPI buffer */
	for (size_t i = 0; i < PAYLOAD_ARG_CNT; i++) {
		mmio_write_32(buffer_base + offset, payload[i]);
		offset += PAYLOAD_ARG_SIZE;
	}
	/* Generate IPI to PMU */
	ipi_mb_notify(proc->ipi->apu_ipi_id, proc->ipi->pmu_ipi_id,
		      is_blocking);

#if TRCH_SERVER
	/* send PSCI command request to TRCH */
	/* initial test code */
	uint32_t mbox_payload[32];

	// INFO("pm_ipi_send_common: send mail to TRCH\n");
	/* The first two words are added
         * 0: CMD_PSCI
         * 1: node_id of the caller */
	mbox_payload[0] = CMD_PSCI;	/* CMD_PSCI */
	mbox_payload[1] = proc->node_id;/* caller ID */
	for (size_t i = 0; i < PAYLOAD_ARG_CNT ; i++) {
		mbox_payload[i+2] = payload[i];
	}

#if ATF_FIQ	
	gicv3_cpuif_enable(plat_my_core_pos());
        gicv3_rdistif_init(plat_my_core_pos());
#endif
	/* send payload and get ack */
	int rc = trch_atf_link->request(trch_atf_link, 
				CMD_TIMEOUT_MS_SEND, mbox_payload, (PAYLOAD_ARG_CNT + 1) * sizeof(uint32_t)/* sizeof(PAYLOAD_ARG_CNT+1) */,
				CMD_TIMEOUT_MS_RECV, value, count * sizeof(uint32_t));
	if (rc < 0) 
		return rc;
#endif
	return PM_RET_SUCCESS;
}

/**
 * pm_ipi_send_non_blocking() - Sends IPI request to the PMU without blocking
 *			        notification
 * @proc	Pointer to the processor who is initiating request
 * @payload	API id and call arguments to be written in IPI buffer
 *
 * Send an IPI request to the power controller.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_ipi_send_non_blocking(const struct pm_proc *proc,
					    uint32_t payload[PAYLOAD_ARG_CNT])
{
	enum pm_ret_status ret;

	bakery_lock_get(&pm_secure_lock);
#if TRCH_SERVER
	ret = pm_ipi_send_common(proc, payload, IPI_NON_BLOCKING, NULL, 0);
#else
	ret = pm_ipi_send_common(proc, payload, IPI_NON_BLOCKING);
#endif
	bakery_lock_release(&pm_secure_lock);

	return ret;
}

/**
 * pm_ipi_send() - Sends IPI request to the PMU
 * @proc	Pointer to the processor who is initiating request
 * @payload	API id and call arguments to be written in IPI buffer
 *
 * Send an IPI request to the power controller.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_ipi_send(const struct pm_proc *proc,
			       uint32_t payload[PAYLOAD_ARG_CNT])
{
	enum pm_ret_status ret;

	VERBOSE("pm_ipi_send : start \n");
	bakery_lock_get(&pm_secure_lock);

#if TRCH_SERVER
	ret = pm_ipi_send_common(proc, payload, IPI_BLOCKING, NULL, 0);
#else
	ret = pm_ipi_send_common(proc, payload, IPI_BLOCKING);
#endif
	bakery_lock_release(&pm_secure_lock);

	return ret;
}


#if TRCH_SERVER == 0
/**
 * pm_ipi_buff_read() - Reads IPI response after PMU has handled interrupt
 * @proc	Pointer to the processor who is waiting and reading response
 * @value	Used to return value from IPI buffer element (optional)
 * @count	Number of values to return in @value
 *
 * @return	Returns status, either success or error+reason
 */
static enum pm_ret_status pm_ipi_buff_read(const struct pm_proc *proc,
					   unsigned int *value, size_t count)
{
	/* receive ACK packet from TRCH */

	size_t i;
	uintptr_t buffer_base = proc->ipi->buffer_base +
				IPI_BUFFER_TARGET_PMU_OFFSET +
				IPI_BUFFER_RESP_OFFSET;
	VERBOSE("%s: start \n", __func__);

	/*
	 * Read response from IPI buffer
	 * buf-0: success or error+reason
	 * buf-1: value
	 * buf-2: unused
	 * buf-3: unused
	 */
	for (i = 1; i <= count; i++) {
		*value = mmio_read_32(buffer_base + (i * PAYLOAD_ARG_SIZE));
	VERBOSE("%s: value[%ld] = 0x%x\n", __func__, i, *value);
		value++;
	}
	VERBOSE("%s: count = %ld, read %lx \n", __func__, count, buffer_base);

	return mmio_read_32(buffer_base);
}
#endif

/**
 * pm_ipi_buff_read_callb() - Reads IPI response after PMU has handled interrupt
 * @value	Used to return value from IPI buffer element (optional)
 * @count	Number of values to return in @value
 *
 * @return	Returns status, either success or error+reason
 */
void pm_ipi_buff_read_callb(unsigned int *value, size_t count)
{
	size_t i;
	uintptr_t buffer_base = IPI_BUFFER_PMU_BASE +
				IPI_BUFFER_TARGET_APU_OFFSET +
				IPI_BUFFER_REQ_OFFSET;

	if (count > IPI_BUFFER_MAX_WORDS)
		count = IPI_BUFFER_MAX_WORDS;

	for (i = 0; i <= count; i++) {
		*value = mmio_read_32(buffer_base + (i * PAYLOAD_ARG_SIZE));
		value++;
	}
}

/**
 * pm_ipi_send_sync() - Sends IPI request to the PMU
 * @proc	Pointer to the processor who is initiating request
 * @payload	API id and call arguments to be written in IPI buffer
 * @value	Used to return value from IPI buffer element (optional)
 * @count	Number of values to return in @value
 *
 * Send an IPI request to the power controller and wait for it to be handled.
 *
 * @return	Returns status, either success or error+reason and, optionally,
 *		@value
 */
enum pm_ret_status pm_ipi_send_sync(const struct pm_proc *proc,
				    uint32_t payload[PAYLOAD_ARG_CNT],
				    unsigned int *value, size_t count)
{
	enum pm_ret_status ret;
	VERBOSE("pm_ipi_send_sync: start \n");

	bakery_lock_get(&pm_secure_lock);

#if TRCH_SERVER
	ret = pm_ipi_send_common(proc, payload, IPI_BLOCKING, value, count);
	if (ret != PM_RET_SUCCESS)
		goto unlock;
#else
	ret = pm_ipi_send_common(proc, payload, IPI_BLOCKING);
	VERBOSE("pm_ipi_send_sync: after pm_ipi_send_common: return(%d)\n", ret);
	if (ret != PM_RET_SUCCESS)
		goto unlock;

	ret = pm_ipi_buff_read(proc, value, count);

	VERBOSE("pm_ipi_send_sync: after pm_ipi_buff_read: return(%d)\n", ret);
#endif

unlock:
	bakery_lock_release(&pm_secure_lock);

	return ret;
}

void pm_ipi_irq_enable(const struct pm_proc *proc)
{
	ipi_mb_enable_irq(proc->ipi->apu_ipi_id, proc->ipi->pmu_ipi_id);
}

void pm_ipi_irq_clear(const struct pm_proc *proc)
{
	ipi_mb_ack(proc->ipi->apu_ipi_id, proc->ipi->pmu_ipi_id);
}

uint32_t pm_ipi_irq_status(const struct pm_proc *proc)
{
	int ret;

	ret = ipi_mb_enquire_status(proc->ipi->apu_ipi_id,
				    proc->ipi->pmu_ipi_id);
	if (ret & IPI_MB_STATUS_RECV_PENDING)
		return 1;
	else
		return 0;
}

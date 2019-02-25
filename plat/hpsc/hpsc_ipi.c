/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Zynq UltraScale+ MPSoC IPI agent registers access management
 */

#include <bakery_lock.h>
#include <debug.h>
#include <errno.h>
#include <mmio.h>
#include <runtime_svc.h>
#include <string.h>
#include "hpsc_ipi.h"
#include "../hpsc_private.h"

/*********************************************************************
 * Macros definitions
 ********************************************************************/

/* IPI registers base address */
#define IPI_REGS_BASE   0xFF300000U

/* IPI registers offsets macros */
#define IPI_TRIG_OFFSET 0x00U
#define IPI_OBR_OFFSET  0x04U
#define IPI_ISR_OFFSET  0x10U
#define IPI_IMR_OFFSET  0x14U
#define IPI_IER_OFFSET  0x18U
#define IPI_IDR_OFFSET  0x1CU

/* IPI register start offset */
#define IPI_REG_BASE(I) (hpsc_ipi_table[(I)].ipi_reg_base)

/* IPI register bit mask */
#define IPI_BIT_MASK(I) (hpsc_ipi_table[(I)].ipi_bit_mask)

/* IPI secure check */
#define IPI_SECURE_MASK  0x1U
#define IPI_IS_SECURE(I) ((hpsc_ipi_table[(I)].secure_only & \
			   IPI_SECURE_MASK) ? 1 : 0)

/*********************************************************************
 * Struct definitions
 ********************************************************************/

/* structure to maintain IPI configuration information */
struct hpsc_ipi_config {
	unsigned int ipi_bit_mask;
	unsigned int ipi_reg_base;
	unsigned char secure_only;
};

/**
 * ipi_mb_validate() - validate IPI mailbox access
 *
 * @local  - local IPI ID
 * @remote - remote IPI ID
 * @is_secure - indicate if the requester is from secure software
 *
 * return - 0 success, negative value for errors
 */
int ipi_mb_validate(uint32_t local, uint32_t remote, unsigned int is_secure)
{
	int ret = 0;
	return ret;
}

/**
 * ipi_mb_open() - Open IPI mailbox.
 *
 * @local  - local IPI ID
 * @remote - remote IPI ID
 *
 */
void ipi_mb_open(uint32_t local, uint32_t remote)
{
}

/**
 * ipi_mb_release() - Open IPI mailbox.
 *
 * @local  - local IPI ID
 * @remote - remote IPI ID
 *
 */
void ipi_mb_release(uint32_t local, uint32_t remote)
{
}

/**
 * ipi_mb_enquire_status() - Enquire IPI mailbox status
 *
 * @local  - local IPI ID
 * @remote - remote IPI ID
 *
 * return - 0 idle, positive value for pending sending or receiving,
 *          negative value for errors
 */
int ipi_mb_enquire_status(uint32_t local, uint32_t remote)
{
	int ret = 0;
	return ret;
}

/* ipi_mb_notify() - Trigger IPI mailbox notification
 *
 * @local - local IPI ID
 * @remote - remote IPI ID
 * @is_blocking - if to trigger the notification in blocking mode or not.
 *
 * It sets the remote bit in the IPI agent trigger register.
 *
 */
void ipi_mb_notify(uint32_t local, uint32_t remote, uint32_t is_blocking)
{
}

/* ipi_mb_ack() - Ack IPI mailbox notification from the other end
 *
 * @local - local IPI ID
 * @remote - remote IPI ID
 *
 * It will clear the remote bit in the isr register.
 *
 */
void ipi_mb_ack(uint32_t local, uint32_t remote)
{
}

/* ipi_mb_disable_irq() - Disable IPI mailbox notification interrupt
 *
 * @local - local IPI ID
 * @remote - remote IPI ID
 *
 * It will mask the remote bit in the idr register.
 *
 */
void ipi_mb_disable_irq(uint32_t local, uint32_t remote)
{
}

/* ipi_mb_enable_irq() - Enable IPI mailbox notification interrupt
 *
 * @local - local IPI ID
 * @remote - remote IPI ID
 *
 * It will mask the remote bit in the idr register.
 *
 */
void ipi_mb_enable_irq(uint32_t local, uint32_t remote)
{
}

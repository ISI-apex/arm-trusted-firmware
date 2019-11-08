/*
 * Copyright (c) 2014-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __HPSC_DEF_H__
#define __HPSC_DEF_H__

#include <common_def.h>

#define HPSC_CONSOLE_ID_16550           1
/* #define HPSC_CONSOLE_ID_dcc		2 */

#define HPSC_CONSOLE_IS(con)	(HPSC_CONSOLE_ID_ ## con == HPSC_CONSOLE)

/* Firmware Image Package */
#define HPSC_PRIMARY_CPU		0

/*******************************************************************************
 * HPSC memory map related constants
 ******************************************************************************/
/* Aggregate of all devices in the first GB */
#define DEVICE0_BASE		0xFF000000
#define DEVICE0_SIZE		0x01000000	/* extended from 0x00E00000 for hpsc-mailbox */
#define DEVICE1_BASE		0xF9000000
#define DEVICE1_SIZE		0x00800000

/*******************************************************************************
 * CCI-400 related constants
 ******************************************************************************/
#define PLAT_ARM_CCI_BASE		0xFD6E0000
#define PLAT_ARM_CCI_CLUSTER0_SL_IFACE_IX	3
#define PLAT_ARM_CCI_CLUSTER1_SL_IFACE_IX	4

/*******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/
#define BASE_GICD_BASE		0xF9000000
#define PLAT_ARM_GICR_BASE 	0xF9100000

#define ARM_IRQ_SEC_PHY_TIMER		29

#define ARM_IRQ_SEC_SGI_0		8
#define ARM_IRQ_SEC_SGI_1		9
#define ARM_IRQ_SEC_SGI_2		10
#define ARM_IRQ_SEC_SGI_3		11
#define ARM_IRQ_SEC_SGI_4		12
#define ARM_IRQ_SEC_SGI_5		13
#define ARM_IRQ_SEC_SGI_6		14
#define ARM_IRQ_SEC_SGI_7		15

#define ARM_IRQ_SEC_SPI_0		32
#define ARM_IRQ_SEC_SPI_1		33
#define ARM_IRQ_SEC_SPI_2		34
#define ARM_IRQ_SEC_SPI_3		35
#define ARM_IRQ_SEC_SPI_4		36
#define ARM_IRQ_SEC_SPI_5		37
#define ARM_IRQ_SEC_SPI_6		38
#define ARM_IRQ_SEC_SPI_7		39

#define MAX_INTR_EL3			128

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define HPSC_UART0_BASE		0xF92C0000

#if HPSC_CONSOLE_IS(16550)
# define HPSC_UART_BASE	HPSC_UART0_BASE
#else
# error "invalid HPSC_CONSOLE"
#endif

#define HPSC_UART_CLOCK         16000000

/* Must be non zero */
#define HPSC_UART_BAUDRATE	500000
#define ARM_CONSOLE_BAUDRATE	HPSC_UART_BAUDRATE

#if TRCH_SERVER
#define HPPS_RCV_IRQ_IDX  MBOX_HPPS_TRCH__HPPS_RCV_ATF_INT	/* 4 */
#define HPPS_ACK_IRQ_IDX  MBOX_HPPS_TRCH__HPPS_ACK_ATF_INT	/* 5 */
#endif /* TRCH_SERVER */

#endif /* __HPSC_DEF_H__ */

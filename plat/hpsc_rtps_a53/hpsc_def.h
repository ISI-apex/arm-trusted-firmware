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

/* Memory location options for Shared data and TSP in HPSC */
#define HPSC_IN_TRUSTED_SRAM		0
#define HPSC_IN_TRUSTED_DRAM		1

/*******************************************************************************
 * HPSC memory map related constants
 ******************************************************************************/
/* Aggregate of all devices in the first GB */
#define DEVICE0_BASE		0x30000000
#define DEVICE0_SIZE		0x01000000
#define DEVICE1_BASE		0x31000000
#define DEVICE1_SIZE		0x0f000000

/* For cpu reset APU space here too 0xFE5F1000 CRF_APB*/
#define CRF_APB_BASE		0xFD1A0000
#define CRF_APB_SIZE		0x00600000
#define CRF_APB_CLK_BASE	0xFD1A0020

/* CRF registers and bitfields */
#define CRF_APB_RST_FPD_APU	(CRF_APB_BASE + 0X00000104)

#define CRF_APB_RST_FPD_APU_ACPU_RESET		(1 << 0)
#define CRF_APB_RST_FPD_APU_ACPU_PWRON_RESET	(1 << 14)

/* CRL registers and bitfields */
#define CRL_APB_BASE			0xFF5E0000
#define CRL_APB_BOOT_MODE_USER		(CRL_APB_BASE + 0x200)
#define CRL_APB_RESET_CTRL		(CRL_APB_BASE + 0x218)
#define CRL_APB_RST_LPD_TOP		(CRL_APB_BASE + 0x23C)
#define CRL_APB_BOOT_PIN_CTRL		(CRL_APB_BASE + 0x250)
#define CRL_APB_CLK_BASE		0xFF5E0020

#define CRL_APB_RPU_AMBA_RESET		(1 << 2)
#define CRL_APB_RPLL_CTRL_BYPASS	(1 << 3)

#define CRL_APB_RESET_CTRL_SOFT_RESET	(1 << 4)

#define CRL_APB_BOOT_MODE_MASK		(0xf << 0)
#define CRL_APB_BOOT_PIN_MASK		(0xf0f << 0)
#define CRL_APB_BOOT_DRIVE_PIN_1_SHIFT	9
#define CRL_APB_BOOT_ENABLE_PIN_1_SHIFT	1
#define CRL_APB_BOOT_ENABLE_PIN_1	(0x1 << CRL_APB_BOOT_ENABLE_PIN_1_SHIFT)
#define CRL_APB_BOOT_DRIVE_PIN_1	(0x1 << CRL_APB_BOOT_DRIVE_PIN_1_SHIFT)
#define HPSC_BOOTMODE_JTAG		0
#define HPSC_ULPI_RESET_VAL_HIGH	(CRL_APB_BOOT_ENABLE_PIN_1 | \
					 CRL_APB_BOOT_DRIVE_PIN_1)
#define HPSC_ULPI_RESET_VAL_LOW	CRL_APB_BOOT_ENABLE_PIN_1

/* system counter registers and bitfields */
#define IOU_SCNTRS_BASE			0xFF260000
#define IOU_SCNTRS_BASEFREQ		(IOU_SCNTRS_BASE + 0x20)

/* APU registers and bitfields */
#define APU_BASE		0xFD5C0000
#define APU_CONFIG_0		(APU_BASE + 0x20)
#define APU_RVBAR_L_0		(APU_BASE + 0x40)
#define APU_RVBAR_H_0		(APU_BASE + 0x44)
#define APU_PWRCTL		(APU_BASE + 0x90)

#define APU_CONFIG_0_VINITHI_SHIFT	8
#define APU_0_PWRCTL_CPUPWRDWNREQ_MASK		1
#define APU_1_PWRCTL_CPUPWRDWNREQ_MASK		2
#define APU_2_PWRCTL_CPUPWRDWNREQ_MASK		4
#define APU_3_PWRCTL_CPUPWRDWNREQ_MASK		8

#define APU1_BASE		0xFD5C1000
#define APU1_CONFIG_0		(APU1_BASE + 0x20)
#define APU1_RVBAR_L_0		(APU1_BASE + 0x40)
#define APU1_RVBAR_H_0		(APU1_BASE + 0x44)
#define APU1_PWRCTL		(APU1_BASE + 0x90)

#define APU1_CONFIG_0_VINITHI_SHIFT	8
#define APU_4_PWRCTL_CPUPWRDWNREQ_MASK		1
#define APU_5_PWRCTL_CPUPWRDWNREQ_MASK		2
#define APU_6_PWRCTL_CPUPWRDWNREQ_MASK		4
#define APU_7_PWRCTL_CPUPWRDWNREQ_MASK		8

/* PMU registers and bitfields */
#define PMU_GLOBAL_BASE			0xFFD80000
#define PMU_GLOBAL_CNTRL		(PMU_GLOBAL_BASE + 0)
#define PMU_GLOBAL_GEN_STORAGE6		(PMU_GLOBAL_BASE + 0x48)
#define PMU_GLOBAL_REQ_PWRUP_STATUS	(PMU_GLOBAL_BASE + 0x110)
#define PMU_GLOBAL_REQ_PWRUP_EN		(PMU_GLOBAL_BASE + 0x118)
#define PMU_GLOBAL_REQ_PWRUP_DIS	(PMU_GLOBAL_BASE + 0x11c)
#define PMU_GLOBAL_REQ_PWRUP_TRIG	(PMU_GLOBAL_BASE + 0x120)

#define PMU_GLOBAL_CNTRL_FW_IS_PRESENT	(1 << 4)

/*******************************************************************************
 * CCI-400 related constants
 ******************************************************************************/
#define PLAT_ARM_CCI_BASE		0xFD6E0000
#define PLAT_ARM_CCI_CLUSTER0_SL_IFACE_IX	3
#define PLAT_ARM_CCI_CLUSTER1_SL_IFACE_IX	4


/*******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/
#define BASE_GICD_BASE		0x30c00000
#define PLAT_ARM_GICR_BASE 	0x30c40000
/* DK: Probably for gicv2 backward compatibility */
#define BASE_GICC_BASE		0xF9020000
#define VE_GICC_BASE		0xF9020000

#if HPSC_WARM_RESTART
#define IRQ_SEC_IPI_APU				67
#define IRQ_TTC3_1				77

#define TTC3_BASE_ADDR				0xFF140000
#define TTC3_INTR_REGISTER_1			(TTC3_BASE_ADDR + 0x54)
#define TTC3_INTR_ENABLE_1			(TTC3_BASE_ADDR + 0x60)
#endif

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
#define HPSC_UART0_BASE		0x30001000

#if HPSC_CONSOLE_IS(16550)
# define HPSC_UART_BASE	HPSC_UART0_BASE
#else
# error "invalid HPSC_CONSOLE"
#endif

#define HPSC_UART_CLOCK         100000000

/* Must be non zero */
#define HPSC_UART_BAUDRATE	115200
#define ARM_CONSOLE_BAUDRATE	HPSC_UART_BAUDRATE

/* Silicon version detection */
#define HPSC_SILICON_VER_MASK		0xF000
#define HPSC_SILICON_VER_SHIFT	12
#define HPSC_CSU_VERSION_SILICON	0
#define HPSC_CSU_VERSION_EP108	1
#define HPSC_CSU_VERSION_VELOCE	2
#define HPSC_CSU_VERSION_QEMU		3

#define HPSC_RTL_VER_MASK		0xFF0
#define HPSC_RTL_VER_SHIFT		4

#define HPSC_PS_VER_MASK		0xF
#define HPSC_PS_VER_SHIFT		0

#define HPSC_CSU_BASEADDR		0xFFCA0000
#define HPSC_CSU_IDCODE_OFFSET	0x40

#define HPSC_CSU_IDCODE_HPSC_ID_SHIFT	0
#define HPSC_CSU_IDCODE_HPSC_ID_MASK	(0xFFF << HPSC_CSU_IDCODE_HPSC_ID_SHIFT)
#define HPSC_CSU_IDCODE_HPSC_ID		0x093

#define HPSC_CSU_IDCODE_SVD_SHIFT		12
#define HPSC_CSU_IDCODE_SVD_MASK		(0x7 << HPSC_CSU_IDCODE_SVD_SHIFT)
#define HPSC_CSU_IDCODE_DEVICE_CODE_SHIFT	15
#define HPSC_CSU_IDCODE_DEVICE_CODE_MASK	(0xF << HPSC_CSU_IDCODE_DEVICE_CODE_SHIFT)
#define HPSC_CSU_IDCODE_SUB_FAMILY_SHIFT	19
#define HPSC_CSU_IDCODE_SUB_FAMILY_MASK	(0x3 << HPSC_CSU_IDCODE_SUB_FAMILY_SHIFT)
#define HPSC_CSU_IDCODE_FAMILY_SHIFT		21
#define HPSC_CSU_IDCODE_FAMILY_MASK		(0x7F << HPSC_CSU_IDCODE_FAMILY_SHIFT)
#define HPSC_CSU_IDCODE_FAMILY		0x23

#define HPSC_CSU_IDCODE_REVISION_SHIFT	28
#define HPSC_CSU_IDCODE_REVISION_MASK		(0xF << HPSC_CSU_IDCODE_REVISION_SHIFT)
#define HPSC_CSU_IDCODE_REVISION		0

#define HPSC_CSU_VERSION_OFFSET	0x44

/* Efuse */
#define EFUSE_BASEADDR		0xFFCC0000
#define EFUSE_IPDISABLE_OFFSET	0x1018
#define EFUSE_IPDISABLE_VERSION	0x1FFU
#define HPSC_EFUSE_IPDISABLE_SHIFT	20

/* Access control register defines */
#define ACTLR_EL3_L2ACTLR_BIT	(1 << 6)
#define ACTLR_EL3_CPUACTLR_BIT	(1 << 0)

#define IOU_SLCR_BASEADDR		0xFF180000

#define HPSC_RPU_GLBL_CNTL			0xFF9A0000
#define HPSC_RPU0_CFG				0xFF9A0100
#define HPSC_RPU1_CFG				0xFF9A0200
#define HPSC_SLSPLIT_MASK			0x08
#define HPSC_TCM_COMB_MASK			0x40
#define HPSC_SLCLAMP_MASK			0x10
#define HPSC_VINITHI_MASK			0x04

/* Tap delay bypass */
#define IOU_TAPDLY_BYPASS			0XFF180390
#define TAP_DELAY_MASK				0x7

/* SGMII mode */
#define IOU_GEM_CTRL				0xFF180360
#define IOU_GEM_CLK_CTRL			0xFF180308
#define SGMII_SD_MASK				0x3
#define SGMII_SD_OFFSET				2
#define SGMII_PCS_SD_0				0x0
#define SGMII_PCS_SD_1				0x1
#define SGMII_PCS_SD_PHY			0x2
#define GEM_SGMII_MASK				0x4
#define GEM_CLK_CTRL_MASK			0xF
#define GEM_CLK_CTRL_OFFSET			5
#define GEM_RX_SRC_SEL_GTR			0x1
#define GEM_SGMII_MODE				0x4

/* SD DLL reset */
#define HPSC_SD_DLL_CTRL			0xFF180358
#define HPSC_SD0_DLL_RST_MASK			0x00000004
#define HPSC_SD0_DLL_RST			0x00000004
#define HPSC_SD1_DLL_RST_MASK			0x00040000
#define HPSC_SD1_DLL_RST			0x00040000

/* SD tap delay */
#define HPSC_SD_DLL_CTRL			0xFF180358
#define HPSC_SD_ITAP_DLY			0xFF180314
#define HPSC_SD_OTAP_DLY			0xFF180318
#define HPSC_SD_TAP_OFFSET			16
#define HPSC_SD_ITAPCHGWIN_MASK		0x200
#define HPSC_SD_ITAPCHGWIN			0x200
#define HPSC_SD_ITAPDLYENA_MASK		0x100
#define HPSC_SD_ITAPDLYENA			0x100
#define HPSC_SD_ITAPDLYSEL_MASK		0xFF
#define HPSC_SD_OTAPDLYSEL_MASK		0x3F
#define HPSC_SD_OTAPDLYENA_MASK		0x40
#define HPSC_SD_OTAPDLYENA			0x40

/* Clock control registers */
/* Full power domain clocks */
#define CRF_APB_APLL_CTRL		(CRF_APB_CLK_BASE + 0x00)
#define CRF_APB_DPLL_CTRL		(CRF_APB_CLK_BASE + 0x0c)
#define CRF_APB_VPLL_CTRL		(CRF_APB_CLK_BASE + 0x18)
#define CRF_APB_PLL_STATUS		(CRF_APB_CLK_BASE + 0x24)
#define CRF_APB_APLL_TO_LPD_CTRL	(CRF_APB_CLK_BASE + 0x28)
#define CRF_APB_DPLL_TO_LPD_CTRL	(CRF_APB_CLK_BASE + 0x2c)
#define CRF_APB_VPLL_TO_LPD_CTRL	(CRF_APB_CLK_BASE + 0x30)
/* Peripheral clocks */
#define CRF_APB_ACPU_CTRL		(CRF_APB_CLK_BASE + 0x40)
#define CRF_APB_DBG_TRACE_CTRL		(CRF_APB_CLK_BASE + 0x44)
#define CRF_APB_DBG_FPD_CTRL		(CRF_APB_CLK_BASE + 0x48)
#define CRF_APB_DP_VIDEO_REF_CTRL	(CRF_APB_CLK_BASE + 0x50)
#define CRF_APB_DP_AUDIO_REF_CTRL	(CRF_APB_CLK_BASE + 0x54)
#define CRF_APB_DP_STC_REF_CTRL		(CRF_APB_CLK_BASE + 0x5c)
#define CRF_APB_DDR_CTRL		(CRF_APB_CLK_BASE + 0x60)
#define CRF_APB_GPU_REF_CTRL		(CRF_APB_CLK_BASE + 0x64)
#define CRF_APB_SATA_REF_CTRL		(CRF_APB_CLK_BASE + 0x80)
#define CRF_APB_PCIE_REF_CTRL		(CRF_APB_CLK_BASE + 0x94)
#define CRF_APB_GDMA_REF_CTRL		(CRF_APB_CLK_BASE + 0x98)
#define CRF_APB_DPDMA_REF_CTRL		(CRF_APB_CLK_BASE + 0x9c)
#define CRF_APB_TOPSW_MAIN_CTRL		(CRF_APB_CLK_BASE + 0xa0)
#define CRF_APB_TOPSW_LSBUS_CTRL	(CRF_APB_CLK_BASE + 0xa4)
#define CRF_APB_GTGREF0_REF_CTRL	(CRF_APB_CLK_BASE + 0xa8)
#define CRF_APB_DBG_TSTMP_CTRL		(CRF_APB_CLK_BASE + 0xd8)

/* Low power domain clocks */
#define CRL_APB_IOPLL_CTRL		(CRL_APB_CLK_BASE + 0x00)
#define CRL_APB_RPLL_CTRL		(CRL_APB_CLK_BASE + 0x10)
#define CRL_APB_PLL_STATUS		(CRL_APB_CLK_BASE + 0x20)
#define CRL_APB_IOPLL_TO_FPD_CTRL	(CRL_APB_CLK_BASE + 0x24)
#define CRL_APB_RPLL_TO_FPD_CTRL	(CRL_APB_CLK_BASE + 0x28)
/* Peripheral clocks */
#define CRL_APB_USB3_DUAL_REF_CTRL	(CRL_APB_CLK_BASE + 0x2c)
#define CRL_APB_GEM0_REF_CTRL		(CRL_APB_CLK_BASE + 0x30)
#define CRL_APB_GEM1_REF_CTRL		(CRL_APB_CLK_BASE + 0x34)
#define CRL_APB_GEM2_REF_CTRL		(CRL_APB_CLK_BASE + 0x38)
#define CRL_APB_GEM3_REF_CTRL		(CRL_APB_CLK_BASE + 0x3c)
#define CRL_APB_USB0_BUS_REF_CTRL	(CRL_APB_CLK_BASE + 0x40)
#define CRL_APB_USB1_BUS_REF_CTRL	(CRL_APB_CLK_BASE + 0x44)
#define CRL_APB_QSPI_REF_CTRL		(CRL_APB_CLK_BASE + 0x48)
#define CRL_APB_SDIO0_REF_CTRL		(CRL_APB_CLK_BASE + 0x4c)
#define CRL_APB_SDIO1_REF_CTRL		(CRL_APB_CLK_BASE + 0x50)
#define CRL_APB_UART0_REF_CTRL		(CRL_APB_CLK_BASE + 0x54)
#define CRL_APB_UART1_REF_CTRL		(CRL_APB_CLK_BASE + 0x58)
#define CRL_APB_SPI0_REF_CTRL		(CRL_APB_CLK_BASE + 0x5c)
#define CRL_APB_SPI1_REF_CTRL		(CRL_APB_CLK_BASE + 0x60)
#define CRL_APB_CAN0_REF_CTRL		(CRL_APB_CLK_BASE + 0x64)
#define CRL_APB_CAN1_REF_CTRL		(CRL_APB_CLK_BASE + 0x68)
#define CRL_APB_CPU_R5_CTRL		(CRL_APB_CLK_BASE + 0x70)
#define CRL_APB_IOU_SWITCH_CTRL		(CRL_APB_CLK_BASE + 0x7c)
#define CRL_APB_CSU_PLL_CTRL		(CRL_APB_CLK_BASE + 0x80)
#define CRL_APB_PCAP_CTRL		(CRL_APB_CLK_BASE + 0x84)
#define CRL_APB_LPD_SWITCH_CTRL		(CRL_APB_CLK_BASE + 0x88)
#define CRL_APB_LPD_LSBUS_CTRL		(CRL_APB_CLK_BASE + 0x8c)
#define CRL_APB_DBG_LPD_CTRL		(CRL_APB_CLK_BASE + 0x90)
#define CRL_APB_NAND_REF_CTRL		(CRL_APB_CLK_BASE + 0x94)
#define CRL_APB_ADMA_REF_CTRL		(CRL_APB_CLK_BASE + 0x98)
#define CRL_APB_PL0_REF_CTRL		(CRL_APB_CLK_BASE + 0xa0)
#define CRL_APB_PL1_REF_CTRL		(CRL_APB_CLK_BASE + 0xa4)
#define CRL_APB_PL2_REF_CTRL		(CRL_APB_CLK_BASE + 0xa8)
#define CRL_APB_PL3_REF_CTRL		(CRL_APB_CLK_BASE + 0xac)
#define CRL_APB_PL0_THR_CNT		(CRL_APB_CLK_BASE + 0xb4)
#define CRL_APB_PL1_THR_CNT		(CRL_APB_CLK_BASE + 0xbc)
#define CRL_APB_PL2_THR_CNT		(CRL_APB_CLK_BASE + 0xc4)
#define CRL_APB_PL3_THR_CNT		(CRL_APB_CLK_BASE + 0xdc)
#define CRL_APB_GEM_TSU_REF_CTRL	(CRL_APB_CLK_BASE + 0xe0)
#define CRL_APB_DLL_REF_CTRL		(CRL_APB_CLK_BASE + 0xe4)
#define CRL_APB_AMS_REF_CTRL		(CRL_APB_CLK_BASE + 0xe8)
#define CRL_APB_I2C0_REF_CTRL		(CRL_APB_CLK_BASE + 0x100)
#define CRL_APB_I2C1_REF_CTRL		(CRL_APB_CLK_BASE + 0x104)
#define CRL_APB_TIMESTAMP_REF_CTRL	(CRL_APB_CLK_BASE + 0x108)
#define IOU_SLCR_GEM_CLK_CTRL		(IOU_SLCR_BASEADDR + 0x308)
#define IOU_SLCR_CAN_MIO_CTRL		(IOU_SLCR_BASEADDR + 0x304)
#define IOU_SLCR_WDT_CLK_SEL		(IOU_SLCR_BASEADDR + 0x300)

/* Global general storage register base address */
#define GGS_BASEADDR		(0xFFD80030U)
#define GGS_NUM_REGS		(4)

/* Persistent global general storage register base address */
#define PGGS_BASEADDR		(0xFFD80050U)
#define PGGS_NUM_REGS		(4)

/* Warm restart boot health status register and mask */
#define PM_BOOT_HEALTH_STATUS_REG		(GGS_BASEADDR + 0x10)
#define PM_BOOT_HEALTH_STATUS_MASK		0x00000001

/*AFI registers */
#define  AFIFM6_WRCTRL		(13)
#define  FABRIC_WIDTH		(3)
#endif /* __HPSC_DEF_H__ */

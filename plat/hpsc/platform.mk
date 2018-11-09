# Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

override ERRATA_A53_855873 := 1
override ENABLE_PLAT_COMPAT := 0
override PROGRAMMABLE_RESET_ADDRESS := 1
MULTI_CONSOLE_API := 1
PSCI_EXTENDED_STATE_ID := 1
A53_DISABLE_NON_TEMPORAL_HINT := 0
SEPARATE_CODE_AND_RODATA := 1
HPSC_WARM_RESTART := 0
override RESET_TO_BL31 := 1

# Do not enable SVE
ENABLE_SVE_FOR_NS	:= 0

WORKAROUND_CVE_2017_5715	:=	0

ifdef HPSC_ATF_MEM_BASE
    $(eval $(call add_define,HPSC_ATF_MEM_BASE))

    ifndef HPSC_ATF_MEM_SIZE
        $(error "HPSC_ATF_BASE defined without HPSC_ATF_SIZE")
    endif
    $(eval $(call add_define,HPSC_ATF_MEM_SIZE))

    ifdef HPSC_ATF_MEM_PROGBITS_SIZE
        $(eval $(call add_define,HPSC_ATF_MEM_PROGBITS_SIZE))
    endif
endif

ifdef HPSC_BL32_MEM_BASE
    $(eval $(call add_define,HPSC_BL32_MEM_BASE))

    ifndef HPSC_BL32_MEM_SIZE
        $(error "HPSC_BL32_BASE defined without HPSC_BL32_SIZE")
    endif
    $(eval $(call add_define,HPSC_BL32_MEM_SIZE))
endif

ifdef HPSC_WARM_RESTART
  $(eval $(call add_define,HPSC_WARM_RESTART))
endif

PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/			\
				-Iinclude/plat/arm/common/aarch64/		\
				-Iplat/hpsc/include/				\
				-Iplat/hpsc/pm_service/				\
				-Iplat/hpsc/ipi_mailbox_service/

PLAT_BL_COMMON_SOURCES	:=	lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/aarch64/xlat_tables.c		\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/arm/gic/common/gic_common.c		\
				drivers/arm/gic/v2/gicv2_main.c			\
				drivers/arm/gic/v2/gicv2_helpers.c		\
				drivers/console/aarch64/multi_console.S		\
				plat/arm/common/arm_cci.c			\
				plat/arm/common/arm_common.c			\
				plat/arm/common/arm_gicv2.c			\
				plat/common/plat_gicv2.c			\
				plat/hpsc/aarch64/hpsc_helpers.S		\
				plat/hpsc/aarch64/hpsc_common.c

HPSC_CONSOLE	?=	cadence
ifeq (${HPSC_CONSOLE}, $(filter ${HPSC_CONSOLE},cadence cadence0 cadence1))
  PLAT_BL_COMMON_SOURCES += drivers/cadence/uart/aarch64/cdns_console.S
else ifeq (${HPSC_CONSOLE}, dcc)
  PLAT_BL_COMMON_SOURCES += \
			    drivers/arm/dcc/dcc_console.c
else
  $(error "Please define HPSC_CONSOLE")
endif
$(eval $(call add_define_val,HPSC_CONSOLE,HPSC_CONSOLE_ID_${HPSC_CONSOLE}))

BL31_SOURCES		+=	drivers/arm/cci/cci.c			\
				lib/cpus/aarch64/aem_generic.S		\
				lib/cpus/aarch64/cortex_a53.S		\
				plat/common/plat_psci_common.c		\
				plat/hpsc/bl31_hpsc_setup.c		\
				plat/hpsc/plat_psci.c			\
				plat/hpsc/plat_hpsc.c			\
				plat/hpsc/plat_startup.c		\
				plat/hpsc/plat_topology.c		\
				plat/hpsc/sip_svc_setup.c		\
				plat/hpsc/hpsc_ipi.c			\
				plat/hpsc/pm_service/pm_svc_main.c	\
				plat/hpsc/pm_service/pm_api_sys.c	\
				plat/hpsc/pm_service/pm_api_pinctrl.c	\
				plat/hpsc/pm_service/pm_api_ioctl.c	\
				plat/hpsc/pm_service/pm_api_clock.c	\
				plat/hpsc/pm_service/pm_ipi.c		\
				plat/hpsc/pm_service/pm_client.c	\
				plat/hpsc/ipi_mailbox_service/ipi_mailbox_svc.c

ifneq (${RESET_TO_BL31},1)
  $(error "Using BL31 as the reset vector is only one option supported on ZynqMP. Please set RESET_TO_BL31 to 1.")
endif

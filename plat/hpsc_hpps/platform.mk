# Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

override ERRATA_A53_855873 := 1
override ENABLE_PLAT_COMPAT := 0
override PROGRAMMABLE_RESET_ADDRESS := 1
override RESET_TO_BL31 := 1

MULTI_CONSOLE_API := 1
PSCI_EXTENDED_STATE_ID := 1
A53_DISABLE_NON_TEMPORAL_HINT := 0
SEPARATE_CODE_AND_RODATA := 1
HPSC_WARM_RESTART := 0
TRCH_SERVER := 1
CONFIG_AARCH64 := 1

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

ifdef HPSC_NEXT_IMAGE_BASE
    $(eval $(call add_define,HPSC_NEXT_IMAGE_BASE))
endif

ifdef HPSC_WARM_RESTART
  $(eval $(call add_define,HPSC_WARM_RESTART))
endif

ifdef CONFIG_AARCH64
  $(eval $(call add_define,CONFIG_ARCH64))
endif

ifdef TRCH_SERVER 
  $(eval $(call add_define,TRCH_SERVER))
endif

PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/			\
				-Iinclude/plat/arm/common/aarch64/		\
				-Iplat/hpsc/					\
				-Iplat/hpsc/include/				\
				-Iplat/hpsc/hpsc_mailbox/			\
				-Iplat/hpsc_hpps/				\
				-Iplat/hpsc_hpps/include			\

PLAT_BL_COMMON_SOURCES	:=	lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/aarch64/xlat_tables.c		\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/arm/gic/common/gic_common.c		\
				drivers/arm/gic/v3/gicv3_main.c			\
				drivers/arm/gic/v3/gicv3_helpers.c		\
				drivers/console/aarch64/multi_console.S		\
				plat/arm/common/arm_common.c			\
				plat/arm/common/arm_gicv3.c			\
				plat/common/plat_gicv3.c			\
				plat/hpsc/aarch64/hpsc_helpers.S		\
				plat/hpsc/aarch64/hpsc_common.c

HPSC_CONSOLE	?=	16550
ifeq (${HPSC_CONSOLE}, $(filter ${HPSC_CONSOLE},16550))
  PLAT_BL_COMMON_SOURCES += drivers/ti/uart/aarch64/16550_console.S
else ifeq (${HPSC_CONSOLE}, dcc)
  PLAT_BL_COMMON_SOURCES += drivers/arm/dcc/dcc_console.c
else
  $(error "Please define HPSC_CONSOLE")
endif
$(eval $(call add_define_val,HPSC_CONSOLE,HPSC_CONSOLE_ID_${HPSC_CONSOLE}))

BL31_SOURCES		+=	lib/cpus/aarch64/aem_generic.S		\
				lib/cpus/aarch64/cortex_a53.S		\
				plat/common/plat_psci_common.c		\
				plat/hpsc/bl31_hpsc_setup.c		\
				plat/hpsc/plat_psci.c			\
				plat/hpsc/plat_hpsc.c			\
				plat/hpsc/pm_ipi.c			\
				plat/hpsc/hpsc_mailbox/command.c  \
				plat/hpsc/hpsc_mailbox/gic.c  \
				plat/hpsc/hpsc_mailbox/intc.c \
				plat/hpsc/hpsc_mailbox/mailbox.c  \
				plat/hpsc/hpsc_mailbox/mailbox-link.c \
				plat/hpsc/hpsc_mailbox/mem.c  \
				plat/hpsc/hpsc_mailbox/object.c \

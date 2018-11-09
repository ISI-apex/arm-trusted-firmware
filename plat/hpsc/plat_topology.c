/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "include/platform_def.h"
static const unsigned char plat_power_domain_tree_desc[] = {
		HPSC_PWR_DOMAINS_AT_MAX_PWR_LVL,
		HPSC_CLUSTER_COUNT,
		HPSC_CLUSTER1_CORE_COUNT,
		HPSC_CLUSTER0_CORE_COUNT};

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return plat_power_domain_tree_desc;
}

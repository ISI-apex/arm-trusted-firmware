/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "include/platform_def.h"
/* this is used to power up/down nodes */
// static const unsigned char plat_power_domain_tree_desc[] = {1, 8};
// static const unsigned char plat_power_domain_tree_desc[] = {1, 2, 4, 4};

static const unsigned char plat_power_domain_tree_desc[] = {
		HPSC_PWR_DOMAINS_AT_MAX_PWR_LVL,
		HPSC_CLUSTER_COUNT,
		HPSC_CLUSTER1_CORE_COUNT,
		HPSC_CLUSTER0_CORE_COUNT};

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return plat_power_domain_tree_desc;
}

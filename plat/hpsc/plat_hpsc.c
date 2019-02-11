/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat_arm.h>
#include <debug.h>

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	VERBOSE("plat_core_pos_by_mpidr: start with mpidr(0x%lx)\n", mpidr);
	if (((mpidr & MPIDR_CLUSTER_MASK) != 0) && ((mpidr & MPIDR_CLUSTER_MASK) != 0x100)) 
	{
		VERBOSE("plat_core_pos_by_mpidr: mpidr - 0x%lx, MPIDR_CUSTER_MASK - 0x%llx, mpidr & MPIDR_CLUSTER_MASK = 0x%llx\n",
                     mpidr, MPIDR_CLUSTER_MASK, mpidr & MPIDR_CLUSTER_MASK);
		return -1;
	}

	if ((mpidr & MPIDR_CPU_MASK) >= PLATFORM_CORE_COUNT)
	{
		VERBOSE("plat_core_pos_by_mpidr: >= PLATFORM_CORE_COUNT : mpidr - 0x%lx, MPIDR_CPU_MASK - 0x%llx, mpidr & MPIDR_CPU_MASK = 0x%llx\n",
                     mpidr, MPIDR_CPU_MASK, mpidr & MPIDR_CPU_MASK);
		return -1;
	}

	VERBOSE("plat_core_pos_by_mpidr: return cpu # %d \n", plat_arm_calc_core_pos(mpidr));
	return plat_arm_calc_core_pos(mpidr);	// this is an assembly code
}

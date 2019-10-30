#include "hpsc_def.h"
#include "pm_ipi.h"

/* Order in pm_procs_all array must match cpu ids */
static const struct pm_proc pm_procs_all[] = {
	{ .node_id = NODE_APU_0, },
	{ .node_id = NODE_APU_1, },
	{ .node_id = NODE_APU_2, },
	{ .node_id = NODE_APU_3, },
	{ .node_id = NODE_APU_4, },
	{ .node_id = NODE_APU_5, },
	{ .node_id = NODE_APU_6, },
	{ .node_id = NODE_APU_7, },
};
const struct pm_proc *primary_proc = &pm_procs_all[0];

static const unsigned char plat_power_domain_tree_desc[] = {
		HPSC_PWR_DOMAINS_AT_MAX_PWR_LVL,
		HPSC_CLUSTER_COUNT,
		HPSC_CLUSTER0_CORE_COUNT,
		HPSC_CLUSTER1_CORE_COUNT
};

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return plat_power_domain_tree_desc;
}

/**
 * pm_get_proc() - returns pointer to the proc structure
 * @cpuid:	id of the cpu whose proc struct pointer should be returned
 *
 * Return: pointer to a proc structure if proc is found, otherwise NULL
 */
const struct pm_proc *pm_get_proc(unsigned int cpuid)
{
	if (cpuid < ARRAY_SIZE(pm_procs_all))
		return &pm_procs_all[cpuid];

	return NULL;
}

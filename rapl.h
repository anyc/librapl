
#ifndef RAPL_H
#define RAPL_H

/*
 *
 * librapl
 * -------
 *
 * librapl is a library that simplifies access to the RAPL values in
 * MSR registers of modern Intel CPUs.
 *
 *  Author: Mario Kicherer (http://kicherer.org)
 *  License: GPL v2 (http://www.gnu.org/licenses/gpl-2.0.txt)
 *
 */

#define MSR_RAPL_POWER_UNIT 0x606

#define MSR_ENERGY_STATUS_MASK 0xffffffff

#define MSR_PKG_RAPL_POWER_LIMIT 0x610
#define MSR_PKG_ENERGY_STATUS 0x611
#define MSR_PKG_PERF_STATUS 0x613
#define MSR_PKG_POWER_INFO 0x614

#define MSR_PP0_POWER_LIMIT 0x638
#define MSR_PP0_ENERGY_STATUS 0x639
#define MSR_PP0_POLICY 0x63A
#define MSR_PP0_PERF_STATUS 0x63B

/* MSRs that are supported on 062A */
#define MSR_PP1_POWER_LIMIT 0x640
#define MSR_PP1_ENERGY_STATUS 0x641
#define MSR_PP1_POLICY 0x642

/* MSRs that are supported on 062D */
#define MSR_DRAM_POWER_LIMIT 0x618
#define MSR_DRAM_ENERGY_STATUS 0x619
#define MSR_DRAM_PERF_STATUS 0x61B
#define MSR_DRAM_POWER_INFO 0x61C

/* Defined for use with MSR_RAPL_POWER_UNIT Register */
#define RAPL_POWER_UNIT_OFFSET 0x00
#define RAPL_POWER_UNIT_MASK 0x0F
#define RAPL_ENERGY_UNIT_OFFSET 0x08
#define RAPL_ENERGY_UNIT_MASK 0x1F
#define RAPL_TIME_UNIT_OFFSET 0x10
#define RAPL_TIME_UNIT_MASK 0xF

/* Defined for use with MSR_PKG_POWER_INFO Register */
#define RAPL_THERMAL_SPEC_POWER_OFFSET 0x0
#define RAPL_THERMAL_SPEC_POWER_MASK 0x7FFF
#define RAPL_MINIMUM_POWER_OFFSET 0x10
#define RAPL_MINIMUM_POWER_MASK 0x7FFF
#define RAPL_MAXIMUM_POWER_OFFSET 0x20
#define RAPL_MAXIMUM_POWER_MASK 0x7FFF
#define RAPL_MAXIMUM_TIME_WINDOW_OFFSET 0x30
#define RAPL_MAXIMUM_TIME_WINDOW_MASK 0x3F

struct rapl_units {
	double power_units;
	double energy_units;
	double time_units;
};

struct rapl_pkg_power_info {
	double thermal_spec_power;
	double minimum_power;
	double maximum_power;
	double time_window;
};

struct rapl_raw_power_counters {
	double pkg;
	double pp0;
	double pp1;
	double dram;
};

struct rapl_power_diff {
	double pkg;
	double cpu;
	double gpu;
	double dram;
	double uncore;
};

#define RAPL_GET_ENERGY_STATUS(fd_msr, runits, ID) \
	((double)rapl_get_msr((fd_msr), ID) * (runits)->energy_units)

#define RAPL_PRINT_ENERGY_STATUS(fd_msr, runits, ID) \
	{printf(#ID ": %f\n", RAPL_GET_ENERGY_STATUS(fd_msr, runits, ID));}

int rapl_open_msr(char core);
unsigned long long rapl_get_msr(int fd_msr, int offset);
char rapl_read_msr(int fd_msr, int offset, unsigned long long * buf);

int rapl_get_cpu_id();
int rapl_get_cpu_model();
char rapl_available();

char rapl_get_units(int fd_msr, struct rapl_units * ru);
void rapl_print_units(struct rapl_units * ru);

char rapl_get_pkg_power_info(int fd_msr, struct rapl_units * ru, struct rapl_pkg_power_info * pinfo);
void rapl_print_pkg_power_info(struct rapl_pkg_power_info * pinfo);

void rapl_get_raw_power_counters(int fd_msr, struct rapl_units * runits, struct rapl_raw_power_counters * pc);
void rapl_print_raw_power_counters (int fd_msr, struct rapl_units * runits);

void rapl_get_power_diff(struct rapl_raw_power_counters * start, struct rapl_raw_power_counters * stop,
		struct rapl_power_diff * pd);

void rapl_print_power_diff(struct rapl_power_diff * pd);

char rapl_pkg_available();
char rapl_pp0_available();
char rapl_pp1_available();
char rapl_dram_available();
char rapl_uncore_available();

#endif
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
 * Based on work from:
 * 	http://web.eece.maine.edu/~vweaver/projects/rapl/
 * 	https://github.com/razvanlupusoru/Intel-RAPL-via-Sysfs/
 *
 * See also:
 * 	Intel 64 and IA-32 Architectures Software Developer's Manual,
 * 	Volume 3B: System Programming Guide, Part 2, chapter 14.7
 *
 * Other approaches:
 * 	https://github.com/tpatki/librapl/tree/master/libmsr
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>

#include "rapl.h"

int rapl_open_msr(char core) {
	int fd_msr;
	char filename[BUFSIZ];
	
	snprintf(filename, BUFSIZ, "/dev/cpu/%d/msr", core);
	fd_msr = open(filename, O_RDONLY);
	if (fd_msr < 0) {
		if (errno == ENXIO) {
			fprintf(stderr, "no cpu core %d\n", core);
		} else
		if (errno == EIO) {
			fprintf(stderr, "core %d doesn't support MSRs\n", core);
		} else
		if (errno == EACCES) {
			fprintf(stderr, "Permission denied. On most systems you'll need root privileges for this application.\n");
		}
		perror("open msr failed");
	}
	return fd_msr;
}

// get value at offset. Returns 0 on error.
unsigned long long rapl_get_msr(int fd_msr, int offset) {
	unsigned long long buf;
	
	if (pread(fd_msr, &buf, sizeof(buf), offset) != sizeof(buf)) {
		perror("msr read failed");
		buf = 0;
	}
	return buf;
}

// get value at offset. Returns 1 on success, 0 on error.
char rapl_read_msr(int fd_msr, int offset, unsigned long long * buf) {
	if (pread(fd_msr, buf, sizeof(unsigned long long), offset) != sizeof(unsigned long long))
		return 0;
	
	return 1;
}

char rapl_get_units(int fd_msr, struct rapl_units * ru) {
	unsigned long long data;
	
	if (!rapl_read_msr(fd_msr, MSR_RAPL_POWER_UNIT, &data))
		return 0;
	
	ru->power_units= pow(0.5,(double)( (data>>RAPL_POWER_UNIT_OFFSET)  & RAPL_POWER_UNIT_MASK ) );
	ru->energy_units=pow(0.5,(double)( (data>>RAPL_ENERGY_UNIT_OFFSET) & RAPL_ENERGY_UNIT_MASK ) );
	ru->time_units=  pow(0.5,(double)( (data>>RAPL_TIME_UNIT_OFFSET)   & RAPL_TIME_UNIT_MASK ) );
	
	return 1;
}

void rapl_print_units(struct rapl_units * ru) {
	printf("Power units = %.3f W\n",ru->power_units);
	printf("Energy units = %.8f J\n",ru->energy_units);
	printf("Time units = %.8f s\n",ru->time_units);
	printf("\n");
}

char rapl_get_pkg_power_info(int fd_msr, struct rapl_units * ru, struct rapl_pkg_power_info * pinfo) {
	unsigned long long data;
	
	if (!rapl_read_msr(fd_msr, MSR_PKG_POWER_INFO, &data))
		return 0;
	
	pinfo->thermal_spec_power=ru->power_units*(double)((data>>RAPL_THERMAL_SPEC_POWER_OFFSET) & RAPL_THERMAL_SPEC_POWER_MASK);
	pinfo->minimum_power=     ru->power_units*(double)((data>>RAPL_MINIMUM_POWER_OFFSET)      & RAPL_MINIMUM_POWER_MASK);
	pinfo->maximum_power=     ru->power_units*(double)((data>>RAPL_MAXIMUM_POWER_OFFSET)      & RAPL_MAXIMUM_POWER_MASK);
	pinfo->time_window=       ru->time_units* (double)((data>>RAPL_MAXIMUM_TIME_WINDOW_OFFSET)& RAPL_MAXIMUM_TIME_WINDOW_MASK);
	
	return 1;
}

void rapl_print_pkg_power_info(struct rapl_pkg_power_info * pinfo) {
	printf("Package thermal spec: %.3f W\n",  pinfo->thermal_spec_power);
	printf("Package minimum power: %.3f W\n", pinfo->minimum_power);
	printf("Package maximum power: %.3f W\n", pinfo->maximum_power);
	printf("Package maximum time window: %.3f s\n", pinfo->time_window);
}

// see http://stackoverflow.com/questions/6491566/getting-machine-serial-number-and-cpu-id-using-c-c-in-linux
int rapl_get_cpu_id() {
	unsigned int eax, ebx, ecx, edx;
	
	eax = 1; ecx = 0;
	/* ecx is often an input as well as an output. */
	asm volatile("cpuid"
		: "=a" (eax),
		"=b" (ebx),
		"=c" (ecx),
		"=d" (edx)
		: "0" (eax), "2" (ecx));
	
// 	printf("stepping %d\n", eax & 0xF);
// 	printf("model %d\n", (eax >> 4) & 0xF);
// 	printf("family %d\n", (eax >> 8) & 0xF);
// 	printf("processor type %d\n", (eax >> 12) & 0x3);
// 	printf("extended model %d\n", (eax >> 16) & 0xF);
// 	printf("extended family %d\n", (eax >> 20) & 0xFF);
	
	return eax;
}

int rapl_get_cpu_model() {
	unsigned int eax;
	
	eax = rapl_get_cpu_id();
	return (((eax >> 16) & 0xF)<<4) + ((eax >> 4) & 0xF);
}

char rapl_available() {
	unsigned int eax;
	
	eax = rapl_get_cpu_id();
	
	int model = (((eax >> 16) & 0xF)<<4) + ((eax >> 4) & 0xF);
	int family = ((eax >> 8) & 0xF);
	
	/*
	 * 06_2A : Intel Core Sandy Bridge
	 * 06_2D : Intel Xeon Sandy Bridge
	 * 06_3A : Intel Core Ivy Bridge
	 */
	
	
	if (family == 6) {
		switch (model) {
			case 0x2a:
			case 0x2d:
			case 0x3a:
			case 0x3e:
				return 1;
				break;
		}
	}
	return 0;
}

void rapl_get_raw_power_counters(int fd_msr, struct rapl_units * runits, struct rapl_raw_power_counters * pc) {
	unsigned long long data;
	
	if (rapl_read_msr(fd_msr, MSR_PKG_ENERGY_STATUS, &data))
		pc->pkg = (double)(data & MSR_ENERGY_STATUS_MASK) * runits->energy_units;
	else
		pc->pkg = -1;
	
	if (rapl_read_msr(fd_msr, MSR_PP0_ENERGY_STATUS, &data))
		pc->pp0 = (double)(data & MSR_ENERGY_STATUS_MASK) * runits->energy_units;
	else
		pc->pp0 = -1;
	
	if (rapl_read_msr(fd_msr, MSR_PP1_ENERGY_STATUS, &data))
		pc->pp1 = (double)(data & MSR_ENERGY_STATUS_MASK) * runits->energy_units;
	else
		pc->pp1 = -1;
	
	if (rapl_read_msr(fd_msr, MSR_DRAM_ENERGY_STATUS, &data))
		pc->dram = (double)(data & MSR_ENERGY_STATUS_MASK) * runits->energy_units;
	else
		pc->dram = -1;
}

void rapl_get_power_diff(struct rapl_raw_power_counters * start, struct rapl_raw_power_counters * stop,
		struct rapl_power_diff * pd)
{
	if (rapl_pkg_available())
		pd->pkg = stop->pkg - start->pkg;
	else
		pd->pkg = -1;
	
	if (rapl_pp0_available())
		pd->cpu = stop->pp0 - start->pp0;
	else
		pd->cpu = -1;
	
	if (rapl_pp1_available())
		pd->gpu = stop->pp1 - start->pp1;
	else
		pd->gpu = -1;
	
	if (rapl_dram_available())
		pd->dram = stop->dram - start->dram;
	else
		pd->dram = -1;
	
	if (rapl_uncore_available())
		pd->uncore = pd->pkg - ( pd->cpu + pd->gpu );
	else
		pd->uncore = -1;
}

void rapl_print_power_diff(struct rapl_power_diff * pd) {
	if (pd->pkg > -1) printf("Package: %f J\n", pd->pkg);
	if (pd->cpu > -1) printf("CPU: %f J\n", pd->cpu);
	if (pd->gpu > -1) printf("GPU: %f J\n", pd->gpu);
	if (pd->dram > -1) printf("DRAM: %f J\n", pd->dram);
	if (pd->uncore > -1) printf("Uncore: %f J\n", pd->uncore);
}

void rapl_print_raw_power_counters (int fd_msr, struct rapl_units * runits) {
	RAPL_PRINT_ENERGY_STATUS(fd_msr, runits, MSR_PKG_ENERGY_STATUS);
	RAPL_PRINT_ENERGY_STATUS(fd_msr, runits, MSR_PP0_ENERGY_STATUS);
	RAPL_PRINT_ENERGY_STATUS(fd_msr, runits, MSR_PP1_ENERGY_STATUS);
	
	if (rapl_dram_available()) {
		RAPL_PRINT_ENERGY_STATUS(fd_msr, runits, MSR_DRAM_ENERGY_STATUS);
	}
}

char rapl_pkg_available() {
	return 1;
}

char rapl_pp0_available() {
	return 1;
}

char rapl_pp1_available() {
	int cpu_id = rapl_get_cpu_model();
	
	if (cpu_id == 0x2d)
		return 0;
	
	return 1;
}

char rapl_dram_available() {
	int cpu_id = rapl_get_cpu_model();
	
	if (cpu_id == 0x2a || cpu_id == 0x3a)
		return 0;
	
	return 1;
}

char rapl_uncore_available() {
	return rapl_pkg_available() && rapl_pp0_available() && rapl_pp1_available();
}

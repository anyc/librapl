
#include <stdio.h>
#include <time.h>

#include <rapl.h>

int main(int argc, char ** argv) {
	char core;
	int fd_msr;
	struct rapl_power_counters pc;
	time_t timer;
	struct tm* tm_info;
	char buf[32];
	
	
	if (argc > 1)
		core = atoi(argv[0]);
	
	fd_msr = rapl_open_msr(core);
	
	struct rapl_units runits;
	if (!rapl_get_units(fd_msr, &runits)) {
		printf("error reading units\n");
		return 1;
	}
	rapl_print_units(&runits);
	
	struct rapl_pkg_power_info pinfo;
	if (!rapl_get_pkg_power_info(fd_msr, &runits, &pinfo)) {
		printf("error reading power info\n");
		return 1;
	}
	rapl_print_pkg_power_info(&pinfo);
	
	printf("\n");
	
	while(1) {
		time(&timer);
		tm_info = localtime(&timer);
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
		printf("Time: %s\n", buf);
		
		rapl_get_power_counters(fd_msr, &runits, &pc);
		
		if (rapl_pkg_available())
			printf("Package: %f J\n", pc.pkg);
		if (rapl_pp0_available())
			printf("Powerplane 0: %f J\n", pc.pp0);
		if (rapl_pp1_available())
			printf("Powerplane 1: %f J\n", pc.pp1);
		if (rapl_dram_available())
			printf("DRAM: %f J\n", pc.dram);
		printf("\n");
		
		sleep(1);
	}
	close(fd_msr);
	
	return 0;
}
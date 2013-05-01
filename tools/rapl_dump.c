
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <rapl.h>

FILE *f;

int main(int argc, char ** argv) {
	char core;
	char *mode;
	int fd_msr;
	struct rapl_power_counters last, curr;
	time_t timer;
	struct tm* tm_info;
	char buf[32];
	unsigned int seconds;
	
	
	if (argc > 1)
		core = atoi(argv[1]);
	
	// if seconds argument is set, create gnuplot output
	mode = 0;
	if (argc > 2)
		mode = argv[2];
	
	fd_msr = rapl_open_msr(core);
	
	struct rapl_units runits;
	if (!rapl_get_units(fd_msr, &runits)) {
		printf("error reading units\n");
		return 1;
	}
	if (!mode)
		rapl_print_units(&runits);
	
	struct rapl_pkg_power_info pinfo;
	if (!rapl_get_pkg_power_info(fd_msr, &runits, &pinfo)) {
		printf("error reading power info\n");
		return 1;
	}
	if (!mode) {
		rapl_print_pkg_power_info(&pinfo);
		printf("\n");
	}
	
	if (mode) {
		char buf[64];
		char dataname[64];
		
		snprintf(dataname, sizeof(dataname), "%s.data", mode);
		snprintf(buf, sizeof(buf), "%s.gnuplot", mode);
		f = fopen(buf, "w+");
		if (!f) {
			perror("output failed");
			return 1;
		}
		
		fprintf(f, "set terminal pdf\n");
		fprintf(f, "set output \"%s.pdf\n", mode);
		fprintf(f, "set title \"RAPL power consumption\"\n");
		fprintf(f, "plot ");
		
		int count = 2;
		if (rapl_pkg_available()) {
			fprintf(f, "\"%s\" using 1:%d title \"PKG\" with lines, ", dataname, count);
			count++;
		}
		if (rapl_pp0_available()) {
			fprintf(f, "\"%s\" using 1:%d title \"PP0\" with lines, ", dataname, count);
			count++;
		}
		if (rapl_pp1_available()) {
			fprintf(f, "\"%s\" using 1:%d title \"PP1\" with lines, ", dataname, count);
			count++;
		}
		if (rapl_dram_available()) {
			fprintf(f, "\"%s\" using 1:%d title \"DRAM\" with lines, ", dataname, count);
			count++;
		}
		fseek(f, -2, SEEK_CUR);
		fprintf(f, "\n");
		fclose(f);
		
		
		f = fopen(dataname, "w+");
	}
	
	rapl_get_power_counters(fd_msr, &runits, &last);
	
	seconds = 0;
	while(1) {
		if (!mode) {
			time(&timer);
			tm_info = localtime(&timer);
			strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
			printf("Time: %s\n", buf);
			
			rapl_get_power_counters(fd_msr, &runits, &curr);
			
			if (rapl_pkg_available())
				printf("Package: %f J\n", curr.pkg - last.pkg);
			if (rapl_pp0_available())
				printf("Powerplane 0: %f J\n", curr.pp0 - last.pp0);
			if (rapl_pp1_available())
				printf("Powerplane 1: %f J\n", curr.pp1 - last.pp1);
			if (rapl_dram_available())
				printf("DRAM: %f J\n", curr.dram - last.dram);
			printf("\n");
			
			last = curr;
			sleep(1);
		} else {
			fprintf(f, "%d ", seconds);
			
			rapl_get_power_counters(fd_msr, &runits, &curr);
			
			if (rapl_pkg_available())
				fprintf(f, "%f ", curr.pkg - last.pkg);
			if (rapl_pp0_available())
				fprintf(f, "%f ", curr.pp0 - last.pp0);
			if (rapl_pp1_available())
				fprintf(f, "%f ", curr.pp1 - last.pp1);
			if (rapl_dram_available())
				fprintf(f, "%f ", curr.dram - last.dram);
			fprintf(f, "\n");
			fflush(f);
			seconds++;
			
			last = curr;
			usleep(100000);
		}
		
	}
	close(fd_msr);
	
	return 0;
}
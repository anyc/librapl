
/*
 * 
 * rapl_dump
 * ---------
 *
 * rapl_dump queries current RAPL values and either dumps them into terminal
 * or a data file for Gnuplot (http://www.gnuplot.info/).
 *
 *  Author: Mario Kicherer (http://kicherer.org)
 *  License: GPL v2 (http://www.gnu.org/licenses/gpl-2.0.txt)
 *
 * Usage with gnuplot:
 *
 *	LD_LIBRARY_PATH=../ ./rapl_dump <core_number> rapldata
 *	gnuplot rapldata.gnuplot
 *
 * Usage without gnuplot:
 *	LD_LIBRARY_PATH=../ ./rapl_dump [core_number]
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <rapl.h>

int main(int argc, char ** argv) {
	char core;
	char *mode;
	int fd_msr;
	struct rapl_raw_power_counters start, stop;
	struct rapl_power_diff pd;
	
	time_t timer;
	struct tm* tm_info;
	char buf[32];
	unsigned int seconds;
	FILE *f;
	
	core = 0;
	if (argc > 1)
		core = atoi(argv[1]);
	
	// if seconds argument is set, create gnuplot output
	mode = 0;
	if (argc > 2)
		mode = argv[2];
	
	if (!rapl_available()) {
		unsigned int eax;
		
		eax = rapl_get_cpu_id();
		int family = ((eax >> 8) & 0xF);
		
		int model = rapl_get_cpu_model();
		
		printf("Warning: your CPU (%d %x) is not in the list of known models.\n", family, model);
		printf("         The output might be incorrect!\n");
		printf("If your CPU supports RAPL please open an issue at https://github.com/anyc/librapl\n");
		printf("with the output of this tool.\n\n");
		sleep(5);
	}
	
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
	
	f=0;
	if (mode) {
		// we are in Gnuplot mode
		
		char buf[64];
		char dataname[64];
		
		snprintf(dataname, sizeof(dataname), "%s.data", mode);
		snprintf(buf, sizeof(buf), "%s.gnuplot", mode);
		
		// write gnuplot control script
		f = fopen(buf, "w+");
		if (!f) {
			perror("output failed");
			return 1;
		}
		
		fprintf(f, "set terminal pdf\n");
		fprintf(f, "set output \"%s.pdf\"\n", mode);
		fprintf(f, "set ylabel \"J\"\n");
		fprintf(f, "set xlabel \"Time\"\n");
		fprintf(f, "set title \"RAPL power consumption\"\n");
		fprintf(f, "plot ");
		
		int count = 2;
		if (rapl_pkg_available()) {
			fprintf(f, "\"%s\" using 1:%d title \"PKG\" with lines, ", dataname, count);
			count++;
		}
		if (rapl_pp0_available()) {
			fprintf(f, "\"%s\" using 1:%d title \"CPU\" with lines, ", dataname, count);
			count++;
		}
		if (rapl_pp1_available()) {
			fprintf(f, "\"%s\" using 1:%d title \"GPU\" with lines, ", dataname, count);
			count++;
		}
		if (rapl_dram_available()) {
			fprintf(f, "\"%s\" using 1:%d title \"DRAM\" with lines, ", dataname, count);
			count++;
		}
		
		if (rapl_uncore_available()) {
			fprintf(f, "\"%s\" using 1:%d title \"Uncore\" with lines, ", dataname, count);
			count++;
		}
		fseek(f, -2, SEEK_CUR);
		fprintf(f, "\n");
		fclose(f);
		
		// open data file
		f = fopen(dataname, "w+");
	}
	
	rapl_get_raw_power_counters(fd_msr, &runits, &start);
	
	seconds = 0;
	while(1) {
		if (!mode) {
			// normal output on console
			
			sleep(1);
			
			time(&timer);
			tm_info = localtime(&timer);
			strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
			printf("Time: %s\n", buf);
			
			rapl_get_raw_power_counters(fd_msr, &runits, &stop);
			rapl_get_power_diff(&start, &stop, &pd);
			start = stop;
			
			if (pd.pkg > -1)
				printf("Package: %f J\n", pd.pkg);
			if (pd.cpu > -1)
				printf("CPU: %f J\n", pd.cpu);
			if (pd.gpu > -1)
				printf("GPU: %f J\n", pd.gpu);
			if (pd.dram > -1)
				printf("DRAM: %f J\n", pd.dram);
			if (pd.uncore > -1)
				printf("Uncore: %f J\n", pd.uncore);
			
			printf("\n");
		} else {
			// we are in Gnuplot mode
			
			usleep(100000);
			seconds++;
			fprintf(f, "%d ", seconds);
			
			rapl_get_raw_power_counters(fd_msr, &runits, &stop);
			rapl_get_power_diff(&start, &stop, &pd);
			start = stop;
			
			if (pd.pkg > -1)
				fprintf(f, "%f ", pd.pkg);
			if (pd.cpu > -1)
				fprintf(f, "%f ", pd.cpu);
			if (pd.gpu > -1)
				fprintf(f, "%f ", pd.gpu);
			if (pd.dram > -1)
				fprintf(f, "%f ", pd.dram);
			if (pd.uncore > -1)
				fprintf(f, "%f", pd.uncore);
			
			fprintf(f, "\n");
			fflush(f);
		}
		
	}
	close(fd_msr);
	
	return 0;
}
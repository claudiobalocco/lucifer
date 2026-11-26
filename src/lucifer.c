/*
Lucifer - a 3D FDTD simulator
Copyright (C) 2015-2018 Claudio Balocco

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/* ---------------------------------------- */
/* -------------- LUCIFER ----------------- */
/* ---------------------------------------- */

#include "lucifer_global_def.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define access _access
#define F_OK 0
#else
#include <unistd.h>
#endif
#ifdef _OPENMP
#include <omp.h>
#endif

#define LUC_OUTPUT_BUFFER (8*1024*1024) // stdio buffer for the data file

// start the embedded Python 3 interpreter
static int start_python(void) {
	PyStatus status;
	PyConfig config;

	PyConfig_InitPythonConfig(&config); // honours PYTHONHOME, PYTHONPATH, ...
#ifdef LUC_PYTHONHOME
	// Windows builds (build_windows.bat) record the Python installation used at build
	// time, so lucifer.exe finds the Python standard library without PYTHONHOME
	if(getenv("PYTHONHOME") == NULL) {
		status = PyConfig_SetBytesString(&config, &config.home, LUC_PYTHONHOME);
		if(PyStatus_Exception(status)) {
			PyConfig_Clear(&config);
			return 0;
		}
	}
#endif
	status = Py_InitializeFromConfig(&config);
	PyConfig_Clear(&config);
	if(PyStatus_Exception(status)) {
		printf("[ERROR] failed to start Python: %s\n", status.err_msg ? status.err_msg : "unknown error");
		return 0;
	}
	return 1;
}

#if defined(_WIN32) && defined(LUC_PYTHONHOME)
/*
 build_windows.bat links the Python DLL with /DELAYLOAD, so it is loaded at the
 first Python call, from PATH. Put the Python installation (PYTHONHOME or the one
 used at build time) in front of PATH first: then the Python DLL and the DLLs of
 numpy & co. are found without activating the Python environment. For conda
 installations Library\bin holds those DLLs.
*/
static void add_python_to_path(void) {
	const char *home = getenv("PYTHONHOME");
	char *old_path, *new_path;
	DWORD n;
	size_t len;
	if(home == NULL || home[0] == '\0')
		home = LUC_PYTHONHOME;
	n = GetEnvironmentVariableA("PATH", NULL, 0);
	old_path = malloc(n + 1);
	if(!old_path)
		return;
	old_path[0] = '\0';
	if(n > 0)
		GetEnvironmentVariableA("PATH", old_path, n);
	len = 2*strlen(home) + strlen(old_path) + 32;
	new_path = malloc(len);
	if(new_path) {
		snprintf(new_path, len, "%s;%s\\Library\\bin;%s", home, home, old_path);
		SetEnvironmentVariableA("PATH", new_path);
		free(new_path);
	}
	free(old_path);
}
#endif

#ifdef _WIN32
// let the Windows console interpret the ANSI color codes used in the messages
static void enable_console_colors(void) {
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD mode;
	if(h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode))
		SetConsoleMode(h, mode | 0x0004); // ENABLE_VIRTUAL_TERMINAL_PROCESSING
}
#endif


int main(int argc, char *argv[]) {

	FILE * fp;
	time_t current_time;
	char* c_time_string;
	int cnt;
	int QUIET;
	int OPENMP_THREADS;
	char CONF_FILENAME[MAX_LEN_FILENAME];
	char usr_str[1024];

	LUC_sim_pars *simp;
	LUC_fields *fields;
	LUC_diel_mat *diel;
	LUC_abc_fields *abc_fields;
	LUC_source **src;

#ifdef _WIN32
	enable_console_colors();
#ifdef LUC_PYTHONHOME
	add_python_to_path(); // before the first Python call (delay loaded DLL)
#endif
#endif
	if(!start_python()) // start Python interpreter
		return EXIT_FAILURE;
	PyObject* sysPath = PySys_GetObject("path");
	// sysPath has a borrowed reference, no need to decref it
	PyObject* curDir = PyUnicode_FromString(".");
	PyList_Append(sysPath, curDir);
	Py_DECREF(curDir);

	printf("------------------------------ %s ------------------------------\n\n",LUC_VER);

	QUIET = 0;
	OPENMP_THREADS = 1;
	strcpy(CONF_FILENAME, "luc"); // default conf filename is luc.py

	for(cnt=1;cnt<argc;cnt++) {
		if (!strcmp(argv[cnt], "-q"))
			QUIET = 1;
		else if (!strcmp(argv[cnt], "-c")) {
			if(cnt+1 >= argc) {
				printf(C_RED "[ERROR]" C_RES " -c detected, but no configuration filename provided\n");
				return EXIT_FAILURE;
			}
			if(strlen(argv[cnt+1]) >= sizeof(CONF_FILENAME)) {
				printf(C_RED "[ERROR]" C_RES " configuration filename too long\n");
				return EXIT_FAILURE;
			}
			strcpy(CONF_FILENAME, argv[++cnt]);
		}
		else if (!strcmp(argv[cnt], "-t")) {
			if(cnt+1 >= argc) {
				printf(C_RED "[ERROR]" C_RES " -t detected, but no # of threads provided\n");
				return EXIT_FAILURE;
			}
			OPENMP_THREADS = atoi(argv[++cnt]);
			if(OPENMP_THREADS < 1) {
				printf(C_RED "[ERROR]" C_RES " # of threads must be >= 1\n");
				return EXIT_FAILURE;
			}
		}
		else if (!strcmp(argv[cnt], "-v")){
			printf("%s\n",LUC_VER);
			return EXIT_SUCCESS;
		}
		else if (!strcmp(argv[cnt], "-h")){
			printf("USE: lucifer <options> -c <conf_file>\noptions:\n-q: quiet mode, no output is displayed\n-t: #of threads\n-v: print version number\n-h: help\n\n");
			return EXIT_SUCCESS;
		}
		else
			printf(C_YELLOW "[WARNING]" C_RES " unknown option %s ignored\n", argv[cnt]);
	}

	simp = par_config(CONF_FILENAME);
	if(!simp){
		printf(C_RED "[ERROR]" C_RES " failed to read configuration file %s\n",CONF_FILENAME);
		return EXIT_FAILURE;
	}
	printf(C_GREEN "[MSG]" C_RES " configuration file read successfully\n");

	// set the # of threads before allocating: the arrays are zeroed in parallel so
	// that memory pages are placed close to the threads that will update them
#ifdef _OPENMP
	omp_set_num_threads(OPENMP_THREADS); //set the # of threads for openmp
	printf(C_GREEN "[MSG]" C_RES " using %d OpenMP thread(s)\n", OPENMP_THREADS);
#else
	if(OPENMP_THREADS > 1)
		printf(C_YELLOW "[WARNING]" C_RES " compiled without OpenMP, -t is ignored\n");
#endif

	fields = allocate_fields(simp);
	if(!fields){
		printf(C_RED "[ERROR]" C_RES " failed to allocate fields (out of memory?)\n");
		return EXIT_FAILURE;
	}
	printf(C_GREEN "[MSG]" C_RES " e.m. fields are allocated\n");

	diel = allocate_diel(simp);
	if(!diel){
		printf(C_RED "[ERROR]" C_RES " failed to allocate materials (out of memory?)\n");
		return EXIT_FAILURE;
	}
	printf(C_GREEN "[MSG]" C_RES " materials and geometry are allocated\n");

	abc_fields = allocate_abc(simp);
	if(!abc_fields){
		printf(C_RED "[ERROR]" C_RES " failed to allocate abc fields (out of memory?)\n");
		return EXIT_FAILURE;
	}
	printf(C_GREEN "[MSG]" C_RES " fields for ABC are allocated\n");

	src = malloc(((*simp).n_sources > 0 ? (*simp).n_sources : 1)*sizeof(LUC_source*));
	if(!src){
		printf(C_RED "[ERROR]" C_RES " failed to allocate sources (out of memory?)\n");
		return EXIT_FAILURE;
	}

	for(cnt = 0; cnt < (*simp).n_sources; cnt++)
		if(!source_init((*simp).src_files[cnt], &src[cnt], simp)) {
			printf(C_RED "[ERROR]" C_RES " source_init: failed to allocate source %d (%s)\n",cnt,(*simp).src_files[cnt]);
			return EXIT_FAILURE;
		}

	printf(C_GREEN "[MSG]" C_RES " sources are allocated\n");

	printf(C_GREEN "[MSG]" C_RES " building geometry...\n");
	printf(C_GREEN "[MSG]" C_RES " msgs from " C_BLUE "%s.py" C_RES " begin\n",(*simp).geom_file);
	fflush(stdout); // keep C and Python output in order
	if(!build_geometry((*simp).geom_file, simp, diel)) {
		printf(C_RED "[ERROR]" C_RES " failed to build geometry from %s.py\n",(*simp).geom_file);
		return EXIT_FAILURE;
	}
	PyRun_SimpleString("import sys; sys.stdout.flush()");
	printf(C_GREEN "[MSG]" C_RES " msgs from " C_BLUE "%s.py" C_RES " end\n",(*simp).geom_file);
	if(!check_materials(simp, diel))
		return EXIT_FAILURE;
	printf(C_GREEN "[MSG]" C_RES " geometry done\n\n");


	printf(C_GREEN "[MSG]" C_RES " simulated domain: x = %g m; y = %g m; z = %g m;\n",(*simp).size_x*(*simp).cell_size,(*simp).size_y*(*simp).cell_size,(*simp).size_z*(*simp).cell_size);
	printf(C_GREEN "[MSG]" C_RES " simulated time: %g s\n", (*simp).sim_time);
	printf(C_GREEN "[MSG]" C_RES " total number of time steps: %d\n", (*simp).max_time);
	printf(C_GREEN "[MSG]" C_RES " data to be saved in: %s\n", (*simp).data_file);

	// ask if OK

	//--------------------------------
	//---------- START SIM -----------
	//--------------------------------

	if( (access( (*simp).data_file, F_OK ) != -1) && !QUIET ) {
		printf(C_RED "[WARNING]" C_RES " file %s exists, do you want to overwrite it? [N/y]\n", (*simp).data_file);
		if (scanf("%1023s",usr_str) != 1 || (strcmp(usr_str, "y") && strcmp(usr_str, "Y") && strcmp(usr_str, "yes") && strcmp(usr_str, "Yes") && strcmp(usr_str, "YES"))){
			return EXIT_SUCCESS;
		}
	} else {
		// file doesn't exist
	}

	fp = fopen ((*simp).data_file, "wb"); // binary: no newline translation on Windows
	if(fp == NULL){
		printf(C_RED "[ERROR]" C_RES " failed to create data file %s\n", (*simp).data_file);
		return EXIT_FAILURE;
	}
	setvbuf(fp, NULL, _IOFBF, LUC_OUTPUT_BUFFER); // fewer, larger writes to disk

	write_header(fp, simp);

	current_time = time(NULL);
    	c_time_string = ctime(&current_time);
	printf(C_GREEN "[MSG]" C_RES " simulation started on: %s", c_time_string);

	for ((*simp).t_step = 0; (*simp).t_step < (*simp).max_time; (*simp).t_step++) {

		updateH(fields, diel, simp);
		updateE(fields, diel, simp);

		// add source
		for(cnt = 0; cnt < (*simp).n_sources; cnt++)
		   	add_source(src[cnt], fields, simp);

	    	// apply abc
    		abc(fields, abc_fields, simp);

    		// apply pbc (after sources and abc)
		pbc(fields, simp);

		write_field_views(fp, fields, simp);

		if(!QUIET)
    		printf(C_GREEN "[MSG]" C_RES " completed: %.2f%%\r", ((float)((*simp).t_step+1)/(float)(*simp).max_time)*100);

	}

	current_time = time(NULL);
    c_time_string = ctime(&current_time);
	printf("\n" C_GREEN "[MSG]" C_RES " simulation finished on: %s\n", c_time_string);
	printf("----------------------------------- END ----------------------------------\n");

	if(fclose(fp) != 0) {
		printf(C_RED "[ERROR]" C_RES " failed to write data file %s (disk full?)\n", (*simp).data_file);
		return EXIT_FAILURE;
	}
	Py_Finalize();

	return EXIT_SUCCESS;
}

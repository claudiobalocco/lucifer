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

#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <omp.h>
#include "lucifer_global_def.h"


int main(int argc, char *argv[]) {

	Py_Initialize(); // start Python interpreter
	PyObject* sysPath = PySys_GetObject((char*)"path"); 
	// sysPath has a borrowed reference, no need to decref it
	PyObject* curDir = PyString_FromString(".");
	PyList_Append(sysPath, curDir);
	Py_DECREF(curDir);

	FILE * fp;
	time_t current_time;
	char* c_time_string;
	int i, j, k, cnt;
	int QUIET;
	int OPENMP_THREADS;
	char CONF_FILENAME[1024];
	char usr_str[1024];
	
	LUC_sim_pars *simp;
	LUC_fields *fields;
	LUC_diel_mat *diel;
	LUC_abc_fields *abc_fields;
	LUC_source **src;

	printf("------------------------------ %s ------------------------------\n\n",LUC_VER);	
	
	QUIET = 0;
	OPENMP_THREADS = 1;
	strcpy(CONF_FILENAME, "luc"); // default conf filename is luc.py

	for(cnt=1;cnt<argc;cnt++) {
		if (!strcmp(argv[cnt], "-q"))
			QUIET = 1;
		if (!strcmp(argv[cnt], "-c")) {
			if(argv[cnt+1]==NULL) {
				printf(C_RED "[ERROR]" C_RES " -c detected, but no configuration filename provided\n");
				return 0;
			}
			strcpy(CONF_FILENAME, argv[cnt+1]);
		}
		if (!strcmp(argv[cnt], "-t")) {
			if(argv[cnt+1]==NULL) {
				printf(C_RED "[ERROR]" C_RES " -t detected, but no # of threads provided\n");
				return 0;
			}
			OPENMP_THREADS = atoi(argv[cnt+1]);
		}
		if (!strcmp(argv[cnt], "-v")){
			printf("%s\n",LUC_VER);
			return 0;
		}
		if (!strcmp(argv[cnt], "-h")){
			printf("USE: lucifer <options> -c <conf_file>\noptions:\n-q: quiet mode, no output is displayed\n-t: #of threads\n-v: print version number\n-h: help\n\n");
			return 0;
		}
	}

	simp = par_config(CONF_FILENAME);
	if(!simp){
		printf(C_RED "[ERROR]" C_RES " failed to read configuration file %s\n",CONF_FILENAME);
		return 0;
	}
	printf(C_GREEN "[MSG]" C_RES " configuration file read successfully\n");

	fields = allocate_fields(simp);
	if(!fields){
		printf(C_RED "[ERROR]" C_RES " failed to allocate fields (out of memory?)\n");
		return 0;
	}
	printf(C_GREEN "[MSG]" C_RES " e.m. fields are allocated\n");

	diel = allocate_diel(simp);
	if(!diel){
		printf(C_RED "[ERROR]" C_RES " failed to allocate materials (out of memory?)\n");
		return 0;
	}
	printf(C_GREEN "[MSG]" C_RES " materials and geometry are allocated\n");

	abc_fields = allocate_abc(simp);
	if(!abc_fields){
		printf(C_RED "[ERROR]" C_RES " failed to allocate abc fields (out of memory?)\n");
		return 0;
	}
	printf(C_GREEN "[MSG]" C_RES " fields for ABC are allocated\n");

	src = malloc((*simp).n_sources*sizeof(LUC_source**));
	if(!src){
		printf(C_RED "[ERROR]" C_RES " failed to allocate sources (out of memory?)\n");
		return 0;
	}

	for(cnt = 0; cnt < (*simp).n_sources; cnt++)
		if(!source_init((*simp).src_files[cnt], &src[cnt], simp)) {			
			printf(C_RED "[ERROR]" C_RES "source_init: failed to allocated source %d\n",cnt);
			return 0;
		}

	printf(C_GREEN "[MSG]" C_RES " sources are allocated\n");

	printf(C_GREEN "[MSG]" C_RES " building geometry...\n");
	printf(C_GREEN "[MSG]" C_RES " msgs from " C_BLUE "%s.py" C_RES " begin\n",(*simp).geom_file);
	build_geometry((*simp).geom_file, simp, diel);
	printf(C_GREEN "[MSG]" C_RES " msgs from " C_BLUE "%s.py" C_RES " end\n",(*simp).geom_file);
	printf(C_GREEN "[MSG]" C_RES " geometry done\n\n");
	
	
	printf(C_GREEN "[MSG]" C_RES " simulated domain: x = %g m; y = %g m; z = %g m;\n",(*simp).size_x*(*simp).cell_size,(*simp).size_y*(*simp).cell_size,(*simp).size_z*(*simp).cell_size);
	printf(C_GREEN "[MSG]" C_RES " simulated time: %g s\n", (*simp).sim_time);
	printf(C_GREEN "[MSG]" C_RES " total number of time steps: %d\n", (*simp).max_time);
	printf(C_GREEN "[MSG]" C_RES " data to be saved in: %s\n", (*simp).data_file);
	
	// ask if OK	
	
	//--------------------------------
	//---------- START SIM -----------
	//--------------------------------
	
	omp_set_num_threads(OPENMP_THREADS); //set the # of threads for openmp

	if( (access( (*simp).data_file, F_OK ) != -1) && !QUIET ) {
		printf(C_RED "[WARNING]" C_RES " file %s exists, do you want to overwrite it? [N/y]\n", (*simp).data_file);
		scanf("%s",usr_str);
		if (strcmp(usr_str, "y") && strcmp(usr_str, "Y") && strcmp(usr_str, "yes") && strcmp(usr_str, "Yes") && strcmp(usr_str, "YES")){
			return 0;
		}
	} else {
		// file doesn't exist
	}

	fp = fopen ((*simp).data_file, "w");
	if(fp == NULL){
		printf(C_RED "[ERROR]" C_RES " failed to create data file %s\n", (*simp).data_file);
		return 0;
	}
	
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
		    	
    		// apply pbc
	    	//pbc(pbc_k,pbc_f,pbc_l,pbc_r,pbc_b,pbc_t);
    	
		write_field_views(fp, fields, simp);
		    
		if(!QUIET)	
    		printf(C_GREEN "[MSG]" C_RES " completed: %.2f%%\r", ((float)(*simp).t_step/(float)(*simp).max_time)*100);
    	
	}

	current_time = time(NULL);
    c_time_string = ctime(&current_time);
	printf(C_GREEN "[MSG]" C_RES " simulation finished on: %s\n", c_time_string);
	printf("----------------------------------- END ----------------------------------\n");

	fclose(fp);
	Py_Finalize();

	return 1;
}

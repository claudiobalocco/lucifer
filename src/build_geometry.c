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


#include "lucifer_global_def.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int build_geometry(const char *geom_file, LUC_sim_pars *simp, LUC_diel_mat *diel) {
	
	PyObject *pluginModule, *filterFunc, *args, *result, *fname; 
		
	fname = PyUnicode_DecodeFSDefault(geom_file);
	pluginModule = PyImport_Import(fname);
	Py_DECREF(fname); // we used it, now decrement the reference counter	
	if(!pluginModule) {
		PyErr_Print();
		return 0;
	}	
	
	filterFunc = PyObject_GetAttrString(pluginModule, "build_geometry");
	Py_DECREF(pluginModule);
	if(!filterFunc) {
		PyErr_Print();
		return 0;
	}

	args = Py_BuildValue("OOOOOOOO",(*diel).py_material,(*diel).py_Cexe,(*diel).py_Cexh,(*diel).py_Ceye,(*diel).py_Ceyh,(*diel).py_Ceze,(*diel).py_Cezh,(*simp).py_simp_dict);
	if(!args) {
		Py_DECREF(filterFunc);
		PyErr_Print();
		return 0;
	}

	result = PyObject_CallObject(filterFunc, args);
	Py_DECREF(filterFunc);
	Py_DECREF(args);

	if(!result) {
		PyErr_Print();
		return 0;
	}

	Py_DECREF(result);

	return 1;
}



/*
 Material indices index the coefficient tables (MAX_NUM_OF_MATERIALS entries):
 a larger index would read outside them. Parallel read-only scan, no copy.
*/
int check_materials(LUC_sim_pars *simp, LUC_diel_mat *diel) {

	const unsigned short *m = (*diel).material;
	const long long n = (long long)(*simp).n_cells;
	long long i;
	int max_idx = 0;

	#pragma omp parallel for reduction(max:max_idx)
	for(i = 0; i < n; i++)
		if(m[i] > max_idx)
			max_idx = m[i];

	if(max_idx >= MAX_NUM_OF_MATERIALS) {
		printf(C_RED "[ERROR]" C_RES " material index %d found in the geometry, the maximum is %d\n", max_idx, MAX_NUM_OF_MATERIALS-1);
		return 0;
	}
	printf(C_GREEN "[MSG]" C_RES " highest material index used: %d\n", max_idx);
	return 1;
}

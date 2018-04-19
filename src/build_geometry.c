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

#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lucifer_global_def.h"


int build_geometry(const char *geom_file, LUC_sim_pars *simp, LUC_diel_mat *diel) {
	
	PyObject *pluginModule, *filterFunc, *args, *result, *fname; 
		
	fname = PyString_FromString(geom_file);
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



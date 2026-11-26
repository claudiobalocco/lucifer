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
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/*
 Soft sources: the value of the time profile at the current time step is added
 to the selected field component, at one point or on a whole plane.
 Planes are added in parallel; offsets are ptrdiff_t.
*/

int add_source(LUC_source *src, LUC_fields *fields, LUC_sim_pars *simp){
	
	const int sx = (*simp).size_x;
	const int sy = (*simp).size_y;
	const int sz = (*simp).size_z;
	const ptrdiff_t stride_x = (ptrdiff_t)sy*sz;
	float *f[6] = {(*fields).Ex, (*fields).Ey, (*fields).Ez, (*fields).Hx, (*fields).Hy, (*fields).Hz};
	float *LUC_RESTRICT a;
	float v;
	int x, y, z;

	if((*src).field < 0 || (*src).field > 5)
		return 0;
	a = f[(*src).field];
	v = (*src).t_profile[(*simp).t_step];

	if((*src).type == 0) { 
		// add a point source
		x = (*src).location[0];
		y = (*src).location[1];
		z = (*src).location[2];
		a[x*stride_x + (ptrdiff_t)y*sz + z] += v;
		return 1;
	}

	if((*src).type == 1) { 		
		// add a plane wave
		switch((*src).direction) {
			case 0: // x direction: plane x = location[0]
				x = (*src).location[0];
				#pragma omp parallel for private(z)
				for(y = 0; y < sy; y++)
					for(z = 0; z < sz; z++)
						a[x*stride_x + (ptrdiff_t)y*sz + z] += v;
				return 1;
			case 1: // y direction: plane y = location[1]
				y = (*src).location[1];
				#pragma omp parallel for private(z)
				for(x = 0; x < sx; x++)
					for(z = 0; z < sz; z++)
						a[x*stride_x + (ptrdiff_t)y*sz + z] += v;
				return 1;
			case 2: // z direction: plane z = location[2]
				z = (*src).location[2];
				#pragma omp parallel for private(y)
				for(x = 0; x < sx; x++)
					for(y = 0; y < sy; y++)
						a[x*stride_x + (ptrdiff_t)y*sz + z] += v;
				return 1;
		}
	}
	return 0;
}

// borrowed reference to dict[key] converted to int
static int src_get_int(PyObject *dict, const char *key, int *out, const char *source_file) {
	PyObject *item = PyDict_GetItemString(dict, key); // borrowed, do not DECREF
	long v;
	if(!item) {
		printf(C_RED "[ERROR]" C_RES " source %s: %s not in dictionary\n", source_file, key);
		return 0;
	}
	v = PyLong_AsLong(item);
	if(v == -1 && PyErr_Occurred()) {
		PyErr_Print();
		printf(C_RED "[ERROR]" C_RES " source %s: %s must be an integer\n", source_file, key);
		return 0;
	}
	*out = (int)v;
	return 1;
}

int source_init(const char *source_file, LUC_source **src, LUC_sim_pars *simp){
	PyObject *pluginModule, *filterFunc, *args, *result, *fname, *py_src_dict, *py_item, *py_v;
	int i, ok;
	size_t time_size;

	*src = (LUC_source *)calloc(1, sizeof(LUC_source));
	if(!*src)
		return 0;

	fname = PyUnicode_DecodeFSDefault(source_file);
	if(!fname) {
		PyErr_Print();
		return 0;
	}

	pluginModule = PyImport_Import(fname);
	Py_DECREF(fname);
	if(!pluginModule) {
		PyErr_Print();
		return 0;
	}

	filterFunc = PyObject_GetAttrString(pluginModule, "get_src_par");
	if(!filterFunc) {
		PyErr_Print();
		Py_DECREF(pluginModule);
		return 0;
	}

	py_src_dict = PyObject_CallObject(filterFunc, NULL);
	Py_DECREF(filterFunc);
	if(!py_src_dict) {
		PyErr_Print();
		Py_DECREF(pluginModule);
		return 0;
	}

	if(!PyDict_Check(py_src_dict)) {
		Py_DECREF(py_src_dict);
		Py_DECREF(pluginModule);
		printf(C_RED "[ERROR]" C_RES " get_src_par() in %s must return a dict\n",source_file);
		return 0;
	}

	ok = src_get_int(py_src_dict, "field", &(**src).field, source_file) &&
	     src_get_int(py_src_dict, "type", &(**src).type, source_file);

	if(ok && ((**src).field < 0 || (**src).field > 5)) {
		printf(C_RED "[ERROR]" C_RES " source %s: field must be 0..5 (Ex, Ey, Ez, Hx, Hy, Hz)\n", source_file);
		ok = 0;
	}
	if(ok && (**src).type != 0 && (**src).type != 1) {
		printf(C_RED "[ERROR]" C_RES " source %s: type must be 0 (point) or 1 (plane wave)\n", source_file);
		ok = 0;
	}

	if(ok) {
	// it's a point source or a plane wave
		py_item = PyDict_GetItemString(py_src_dict, "location"); // borrowed
		if(!py_item || !PySequence_Check(py_item) || PySequence_Size(py_item) != 3) {
			PyErr_Clear();
			printf(C_RED "[ERROR]" C_RES " source %s: location must be a tuple (x, y, z)\n", source_file);
			ok = 0;
		}
		for(i = 0; ok && i < 3; i++) {
			py_v = PySequence_GetItem(py_item, i); // new reference
			(**src).location[i] = py_v ? (int)PyLong_AsLong(py_v) : -1;
			Py_XDECREF(py_v);
			if(PyErr_Occurred()) {
				PyErr_Print();
				ok = 0;
			}
		}
	}

	if(ok && (**src).type == 1) {
	// it's a plane wave: get propagation direction direction
		ok = src_get_int(py_src_dict, "direction", &(**src).direction, source_file);
		if(ok && ((**src).direction < 0 || (**src).direction > 2)) {
			printf(C_RED "[ERROR]" C_RES " source %s: direction must be 0, 1 or 2\n", source_file);
			ok = 0;
		}
	}

	// bounds check, an out of range location would write outside the field arrays
	if(ok) {
		int sizes[3] = {(*simp).size_x, (*simp).size_y, (*simp).size_z};
		for(i = 0; i < 3; i++) {
			// for a plane wave only the coordinate along direction is used
			if((**src).type == 1 && i != (**src).direction)
				continue;
			if((**src).location[i] < 0 || (**src).location[i] >= sizes[i]) {
				printf(C_RED "[ERROR]" C_RES " source %s: location is outside the simulation domain\n", source_file);
				ok = 0;
				break;
			}
		}
	}

	Py_DECREF(py_src_dict);
	if(!ok) {
		Py_DECREF(pluginModule);
		return 0;
	}

	// time profile: C array exposed to Python as a writable memoryview (no copy)
	time_size = (size_t)(*simp).max_time*sizeof(float);
	(**src).t_profile = (float *)calloc((size_t)(*simp).max_time, sizeof(float));
	if(!(**src).t_profile) {
		Py_DECREF(pluginModule);
		return 0;
	}
	(**src).py_t_profile = PyMemoryView_FromMemory((char *)(**src).t_profile, (Py_ssize_t)time_size, PyBUF_WRITE);
	if(!(**src).py_t_profile) {
		PyErr_Print();
		Py_DECREF(pluginModule);
		return 0;
	}

	filterFunc = PyObject_GetAttrString(pluginModule, "get_time_profile");
	Py_DECREF(pluginModule);
	if(!filterFunc) {
		PyErr_Print();
		return 0;
	}

	args = Py_BuildValue("(OO)",(**src).py_t_profile,(*simp).py_simp_dict);
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

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

#define XYZ x*(*simp).size_y*(*simp).size_z+y*(*simp).size_z+z

int add_source(LUC_source *src, LUC_fields *fields, LUC_sim_pars *simp){
	
	int x, y, z;

	if((*src).type == 1) { 		
		// add a plane wave
		if((*src).direction == 0) { // x direction
			x = (*src).location[0];

			for(z=0;z<((*simp).size_z);z++) {
				if((*src).field == 0) {
					for(y=0;y<((*simp).size_y);y++) {
						(*fields).Ex[XYZ] = (*fields).Ex[XYZ] + (*src).t_profile[(*simp).t_step];
					}
				}
				if((*src).field == 1) {
					for(y=0;y<((*simp).size_y);y++) {
						(*fields).Ey[XYZ] = (*fields).Ey[XYZ] + (*src).t_profile[(*simp).t_step];
					}
				}
				if((*src).field == 2) {
					for(y=0;y<((*simp).size_y);y++) {
						(*fields).Ez[XYZ] = (*fields).Ez[XYZ] + (*src).t_profile[(*simp).t_step];
					}
				}
				if((*src).field == 3) {
					for(y=0;y<((*simp).size_y);y++) {
						(*fields).Hx[XYZ] = (*fields).Hx[XYZ] + (*src).t_profile[(*simp).t_step];
					}
				}
				if((*src).field == 4) {
					for(y=0;y<((*simp).size_y);y++) {
						(*fields).Hy[XYZ] = (*fields).Hy[XYZ] + (*src).t_profile[(*simp).t_step];
					}
				}
				if((*src).field == 5) {
					for(y=0;y<((*simp).size_y);y++) {
						(*fields).Hz[XYZ] = (*fields).Hz[XYZ] + (*src).t_profile[(*simp).t_step];
					}
				}
			}
			return 1;
		}
		if((*src).direction == 1) { // y direction
			y = (*src).location[1];

			for(x=0;x<((*simp).size_x);x++) {
				if((*src).field == 0) {
					for(z=0;z<((*simp).size_z);z++) {
						(*fields).Ex[XYZ] = (*fields).Ex[XYZ] + (*src).t_profile[(*simp).t_step];
					}
				}
				if((*src).field == 1) {
					for(z=0;z<((*simp).size_z);z++) {
						(*fields).Ey[XYZ] = (*fields).Ey[XYZ] + (*src).t_profile[(*simp).t_step];
					}
				}
				if((*src).field == 2) {
					for(z=0;z<((*simp).size_z);z++) {
						(*fields).Ez[XYZ] = (*fields).Ez[XYZ] + (*src).t_profile[(*simp).t_step];
					}
				}
				if((*src).field == 3) {
					for(z=0;z<((*simp).size_z);z++) {
						(*fields).Hx[XYZ] = (*fields).Hx[XYZ] + (*src).t_profile[(*simp).t_step];
					}
				}
				if((*src).field == 4) {
					for(z=0;z<((*simp).size_z);z++) {
						(*fields).Hy[XYZ] = (*fields).Hy[XYZ] + (*src).t_profile[(*simp).t_step];
					}
				}
				if((*src).field == 5) {
					for(z=0;z<((*simp).size_z);z++) {
						(*fields).Hz[XYZ] = (*fields).Hz[XYZ] + (*src).t_profile[(*simp).t_step];
					}
				}
			}
			return 1;
		}
		if((*src).direction == 2) { // z direction
			z = (*src).location[2];

			for(x=0;x<((*simp).size_x);x++) {
				if((*src).field == 0) {
					for(y=0;y<((*simp).size_y);y++) {
						(*fields).Ex[XYZ] = (*fields).Ex[XYZ] + (*src).t_profile[(*simp).t_step];
					}
				}
				if((*src).field == 1) {
					for(y=0;y<((*simp).size_y);y++) {
						(*fields).Ey[XYZ] = (*fields).Ey[XYZ] + (*src).t_profile[(*simp).t_step];
					}
				}
				if((*src).field == 2) {
					for(y=0;y<((*simp).size_y);y++) {
						(*fields).Ez[XYZ] = (*fields).Ez[XYZ] + (*src).t_profile[(*simp).t_step];
					}
				}
				if((*src).field == 3) {
					for(y=0;y<((*simp).size_y);y++) {
						(*fields).Hx[XYZ] = (*fields).Hx[XYZ] + (*src).t_profile[(*simp).t_step];
					}
				}
				if((*src).field == 4) {
					for(y=0;y<((*simp).size_y);y++) {
						(*fields).Hy[XYZ] = (*fields).Hy[XYZ] + (*src).t_profile[(*simp).t_step];
					}
				}
				if((*src).field == 5) {
					for(y=0;y<((*simp).size_y);y++) {
						(*fields).Hz[XYZ] = (*fields).Hz[XYZ] + (*src).t_profile[(*simp).t_step];
					}
				}
			}
			return 1;
		}
	}

	if((*src).type == 0) { 
		// add a point source source
		x = (*src).location[0];
		y = (*src).location[1];
		z = (*src).location[2];

		if((*src).field == 0) {
			(*fields).Ex[XYZ] = (*fields).Ex[XYZ] + (*src).t_profile[(*simp).t_step];
			return 1;
		}
		if((*src).field == 1) {
			(*fields).Ey[XYZ] = (*fields).Ey[XYZ] + (*src).t_profile[(*simp).t_step];
			return 1;
		}
		if((*src).field == 2) {
			(*fields).Ez[XYZ] = (*fields).Ez[XYZ] + (*src).t_profile[(*simp).t_step];
			return 1;
		}
		if((*src).field == 3) {
			(*fields).Hx[XYZ] = (*fields).Hx[XYZ] + (*src).t_profile[(*simp).t_step];
			return 1;
		}
		if((*src).field == 4) {
			(*fields).Hy[XYZ] = (*fields).Hy[XYZ] + (*src).t_profile[(*simp).t_step];
			return 1;
		}
		if((*src).field == 5) {
			(*fields).Hz[XYZ] = (*fields).Hz[XYZ] + (*src).t_profile[(*simp).t_step];
			return 1;
		}
		return 0;
	}
	return 0;
}

int source_init(const char *source_file, LUC_source **src, LUC_sim_pars *simp){
	PyObject *pluginModule, *filterFunc, *args, *result, *fname, *py_src_dict, *py_item; 
	
	int plane_size, time_size;
	Py_ssize_t s;

	*src = (LUC_source *)malloc(sizeof(LUC_source));
	if(!*src)
		return 0;

	fname = PyString_FromString(source_file);
	if(!fname) {
		PyErr_Print();
		return 0;
	}

	pluginModule = PyImport_Import(fname);
	if(!pluginModule) {
		PyErr_Print();
		Py_DECREF(fname);
		return 0;
	}

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
		printf("Error reading %s\n",source_file);
		return 0;
	}

	
	py_item = PyDict_GetItemString(py_src_dict, "field");
	if(!py_item) {
		Py_DECREF(py_src_dict);
		Py_DECREF(py_item);
		printf("source field not in dictionary\n");
		return 0;
	}
	(**src).field = (int)PyInt_AsLong(py_item);

	py_item = PyDict_GetItemString(py_src_dict, "type");
	if(!py_item) {
		Py_DECREF(py_src_dict);
		Py_DECREF(py_item);
		printf("source type not in dictionary\n");
		return 0;
	}
	(**src).type = (int)PyInt_AsLong(py_item);

	if((**src).type == 0 || (**src).type == 1){
	// it's a point source or a plane wave
		py_item = PyDict_GetItemString(py_src_dict, "location");
		if(!py_item) {
			Py_DECREF(py_src_dict);
			Py_DECREF(py_item);
			printf("source type not in dictionary\n");
			return 0;
		}
		// return a tuple (x, y, z)
		(**src).location[0] = (int)PyInt_AsLong(PyTuple_GetItem(py_item, 0));
		(**src).location[1] = (int)PyInt_AsLong(PyTuple_GetItem(py_item, 1));
		(**src).location[2] = (int)PyInt_AsLong(PyTuple_GetItem(py_item, 2));
	}


	if((**src).type == 1){
	// it's a plane wave: get propagation direction direction
		py_item = PyDict_GetItemString(py_src_dict, "direction");
		if(!py_item) {
			Py_DECREF(py_src_dict);
			Py_DECREF(py_item);
			printf("source direction not in dictionary\n");
			return 0;
		}
		// return an int
		(**src).direction = (int)PyInt_AsLong(py_item);
	}

	/*
	(**src).py_sp_profile = PyBuffer_New(plane_size);
	(**src).sp_profile = (float *)malloc(plane_size);
	PyObject_AsWriteBuffer((**src).py_sp_profile, (void **)&((**src).sp_profile), &s);
	memset((**src).sp_profile, 0, plane_size);
	*/

	time_size = (*simp).max_time*sizeof(float);
	(**src).py_t_profile = PyBuffer_New(time_size);
	(**src).t_profile = (float *)malloc(time_size);
	PyObject_AsWriteBuffer((**src).py_t_profile, (void **)&((**src).t_profile), &s);
	memset((**src).t_profile, 0, time_size);

	filterFunc = PyObject_GetAttrString(pluginModule, "get_time_profile");
	//Py_DECREF(pluginModule);
	if(!filterFunc) {
		PyErr_Print();
		Py_DECREF(pluginModule);
		return 0;
	}

	args = Py_BuildValue("OO",(**src).py_t_profile,(*simp).py_simp_dict);
	if(!args) {
		Py_DECREF(filterFunc);
		Py_DECREF(pluginModule);
		PyErr_Print();
		return 0;
	}

	result = PyObject_CallObject(filterFunc, args);
	Py_DECREF(filterFunc);
	Py_DECREF(args);

	if(!result) {
		PyErr_Print();
		Py_DECREF(pluginModule);
		return 0;
	}
	
	Py_DECREF(pluginModule);
	Py_DECREF(result);

	return 1;
}









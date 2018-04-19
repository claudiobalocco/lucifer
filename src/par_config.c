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
#include <string.h>

#include "lucifer_global_def.h"

LUC_sim_pars * par_config(const char *luc_name) {
	
	LUC_sim_pars *simp;
	PyObject *pluginModule, *filterFunc, *fname, *py_item, *py_item2;
	int i, j;
	char *fbase, *fpath, *fdotpy, pycmd[2048];
	
	simp = malloc(sizeof(LUC_sim_pars));
	
	// we need to set the the path to the directory containg the conf. file - and only that folder
	// it's dirty code - it'll need improvement
	
	fbase = strrchr(luc_name, '/');
	if(fbase == NULL)
		fbase = luc_name;
	else
		fbase++;
	
	fdotpy = strrchr(fbase,'.');
	if(fdotpy!=NULL)
		if(!strcmp(fdotpy,".py")) //strip off the .py extension if exist
			*fdotpy='\0';

	fpath = malloc((strlen(luc_name)+1)*sizeof(char));
	strcpy(fpath,luc_name);
	fpath[strlen(luc_name)-strlen(fbase)]='\0';
	
	sprintf(pycmd,"import sys\nsys.path.append('%s')",fpath);

	PyRun_SimpleString(pycmd);

	// dirty code ends... hopefully	

	fname = PyString_FromString(fbase);

	pluginModule = PyImport_Import(fname);
	Py_DECREF(fname); // we used it, now decrement the reference counter	
	if(!pluginModule) {
		PyErr_Print();
		return NULL;
	}	
	
	filterFunc = PyObject_GetAttrString(pluginModule, "get_pars");
	Py_DECREF(pluginModule);	
	if(!filterFunc) {
		PyErr_Print();
		return NULL;
	}
	
	(*simp).py_simp_dict = PyObject_CallObject(filterFunc, NULL);
	Py_DECREF(filterFunc);
	if(!(*simp).py_simp_dict) {
		PyErr_Print();
		return NULL;
	}
	
	if(!PyDict_Check((*simp).py_simp_dict)) {
		Py_DECREF((*simp).py_simp_dict);
		printf("[ERROR] Error reading %s\n",luc_name);
		return NULL;
	}

	/* --------------------------------------------------------- */

	py_item = PyDict_GetItemString((*simp).py_simp_dict, "size_x");
	if(!py_item) {
		Py_DECREF((*simp).py_simp_dict);
		Py_DECREF(py_item);
		printf("[ERROR] size_x not in dictionary\n");
		return NULL;
	}
	(*simp).size_x = (int)PyInt_AsLong(py_item);
	
	py_item = PyDict_GetItemString((*simp).py_simp_dict, "size_y");
	if(!py_item) {
		Py_DECREF((*simp).py_simp_dict);
		Py_DECREF(py_item);
		printf("[ERROR] size_y not in dictionary\n");
		return NULL;
	}
	(*simp).size_y = (int)PyInt_AsLong(py_item);

	py_item = PyDict_GetItemString((*simp).py_simp_dict, "size_z");
	if(!py_item) {
		Py_DECREF((*simp).py_simp_dict);
		Py_DECREF(py_item);
		printf("[ERROR] size_z not in dictionary\n");
		return NULL;
	}
	(*simp).size_z = (int)PyInt_AsLong(py_item);

	py_item = PyDict_GetItemString((*simp).py_simp_dict, "courant");
	if(!py_item) {
		Py_DECREF((*simp).py_simp_dict);
		Py_DECREF(py_item);
		printf("[ERROR] courant not in dictionary\n");
		return NULL;
	}
	(*simp).courant = (float)PyFloat_AsDouble(py_item);

	py_item = PyDict_GetItemString((*simp).py_simp_dict, "cell_size");
	if(!py_item) {
		Py_DECREF((*simp).py_simp_dict);
		Py_DECREF(py_item);
		printf("[ERROR] cell_size not in dictionary\n");
		return NULL;
	}
	(*simp).cell_size = (float)PyFloat_AsDouble(py_item);

	py_item = PyDict_GetItemString((*simp).py_simp_dict, "sim_time");
	if(!py_item) {
		Py_DECREF((*simp).py_simp_dict);
		Py_DECREF(py_item);
		printf("[ERROR] sim_time not in dictionary\n");
		return NULL;
	}
	(*simp).sim_time = (float)PyFloat_AsDouble(py_item);

	/* --------------------------------------------------------- */
	
	// boundary conditions
	py_item = PyDict_GetItemString((*simp).py_simp_dict, "front");
	if(!py_item) {
		Py_DECREF((*simp).py_simp_dict);
		Py_DECREF(py_item);
		printf("[ERROR] front not in dictionary\n");
		return NULL;
	}
	if (!strcmp(PyString_AsString(py_item),"pbc")){
		(*simp).abc_f = 0;
		(*simp).pbc_f = 1;
	} 
	else if (!strcmp(PyString_AsString(py_item),"abc")){
		(*simp).abc_f = 1;
		(*simp).pbc_f = 0;
	}
	else { //default option
		(*simp).abc_f = 1;
		(*simp).pbc_f = 0;			
	}

	py_item = PyDict_GetItemString((*simp).py_simp_dict, "back");
	if(!py_item) {
		Py_DECREF((*simp).py_simp_dict);
		Py_DECREF(py_item);
		printf("[ERROR] back not in dictionary\n");
		return NULL;
	}
	if (!strcmp(PyString_AsString(py_item),"pbc")){
		(*simp).abc_k = 0;
		(*simp).pbc_k = 1;
	} 
	else if (!strcmp(PyString_AsString(py_item),"abc")){
		(*simp).abc_k = 1;
		(*simp).pbc_k = 0;
	}
	else { //default option
		(*simp).abc_k = 1;
		(*simp).pbc_k = 0;			
	}

	py_item = PyDict_GetItemString((*simp).py_simp_dict, "top");
	if(!py_item) {
		Py_DECREF((*simp).py_simp_dict);
		Py_DECREF(py_item);
		printf("[ERROR] top not in dictionary\n");
		return NULL;
	}
	if (!strcmp(PyString_AsString(py_item),"pbc")){
		(*simp).abc_t = 0;
		(*simp).pbc_t = 1;
	} 
	else if (!strcmp(PyString_AsString(py_item),"abc")){
		(*simp).abc_t = 1;
		(*simp).pbc_t = 0;
	}
	else { //default option
		(*simp).abc_t = 1;
		(*simp).pbc_t = 0;			
	}

	py_item = PyDict_GetItemString((*simp).py_simp_dict, "bottom");
	if(!py_item) {
		Py_DECREF((*simp).py_simp_dict);
		Py_DECREF(py_item);
		printf("[ERROR] bottom not in dictionary\n");
		return NULL;
	}
	if (!strcmp(PyString_AsString(py_item),"pbc")){
		(*simp).abc_b = 0;
		(*simp).pbc_b = 1;
	} 
	else if (!strcmp(PyString_AsString(py_item),"abc")){
		(*simp).abc_b = 1;
		(*simp).pbc_b = 0;
	}
	else { //default option
		(*simp).abc_b = 1;
		(*simp).pbc_b = 0;			
	}

	py_item = PyDict_GetItemString((*simp).py_simp_dict, "left");
	if(!py_item) {
		Py_DECREF((*simp).py_simp_dict);
		Py_DECREF(py_item);
		printf("[ERROR] left not in dictionary\n");
		return NULL;
	}
	if (!strcmp(PyString_AsString(py_item),"pbc")){
		(*simp).abc_l = 0;
		(*simp).pbc_l = 1;
	} 
	else if (!strcmp(PyString_AsString(py_item),"abc")){
		(*simp).abc_l = 1;
		(*simp).pbc_l = 0;
	}
	else { //default option
		(*simp).abc_l = 1;
		(*simp).pbc_l = 0;			
	}

	py_item = PyDict_GetItemString((*simp).py_simp_dict, "right");
	if(!py_item) {
		Py_DECREF((*simp).py_simp_dict);
		Py_DECREF(py_item);
		printf("[ERROR] right not in dictionary\n");
		return NULL;
	}
	if (!strcmp(PyString_AsString(py_item),"pbc")){
		(*simp).abc_r = 0;
		(*simp).pbc_r = 1;
	} 
	else if (!strcmp(PyString_AsString(py_item),"abc")){
		(*simp).abc_r = 1;
		(*simp).pbc_r = 0;
	}
	else { //default option
		(*simp).abc_r = 1;
		(*simp).pbc_r = 0;			
	}
	Py_DECREF(py_item);

	/* --------------------------------------------------------- */

	py_item = PyDict_GetItemString((*simp).py_simp_dict, "data_file");
	if(!py_item) {
		Py_DECREF((*simp).py_simp_dict);
		Py_DECREF(py_item);
		printf("[ERROR] data_file not in dictionary\n");
		return NULL;
	}
	(*simp).data_file = PyString_AsString(py_item);
	Py_DECREF(py_item);

	py_item = PyDict_GetItemString((*simp).py_simp_dict, "geom_file");
	if(!py_item) {
		Py_DECREF((*simp).py_simp_dict);
		Py_DECREF(py_item);
		printf("[ERROR] geom_file not in dictionary\n");
		return NULL;
	}
	(*simp).geom_file = PyString_AsString(py_item);
	Py_DECREF(py_item);

	/* --------------------------------------------------------- */

	py_item = PyDict_GetItemString((*simp).py_simp_dict, "sources");
	if(!py_item) {
		Py_DECREF((*simp).py_simp_dict);
		Py_DECREF(py_item);
		printf("[ERROR] sources not in dictionary\n");
		return NULL;
	}
	i = PyTuple_Size(py_item);
	(*simp).n_sources = i;
	(*simp).src_files = malloc(i*sizeof(const char*));
	for(j = 0; j < i; j++)
		(*simp).src_files[j] = PyString_AsString(PyTuple_GetItem(py_item, j));
	Py_DECREF(py_item);

	/* --------------------------------------------------------- */

	py_item = PyDict_GetItemString((*simp).py_simp_dict, "field_views");
	if(!py_item) {
		Py_DECREF((*simp).py_simp_dict);
		Py_DECREF(py_item);
		printf("[ERROR] field_views not in dictionary\n");
		return NULL;
	}
	i = PyTuple_Size(py_item);
	(*simp).n_field_views = i;
	printf("there are %d field views\n",i);
	(*simp).field_views = malloc(i*sizeof(LUC_field_view));
	
	for(j = 0; j < i; j++) {
		py_item2 = PyDict_GetItemString(PyTuple_GetItem(py_item, j),"field");
		
		if(!strcmp(PyString_AsString(py_item2),"Ex"))
			((*simp).field_views[j]).field = 0;
		else if(!strcmp(PyString_AsString(py_item2),"Ey"))
			((*simp).field_views[j]).field = 1;
		else if(!strcmp(PyString_AsString(py_item2),"Ez"))
			((*simp).field_views[j]).field = 2;
		else if(!strcmp(PyString_AsString(py_item2),"Hx"))
			((*simp).field_views[j]).field = 3;
		else if(!strcmp(PyString_AsString(py_item2),"Hy"))
			((*simp).field_views[j]).field = 4;
		else if(!strcmp(PyString_AsString(py_item2),"Hz"))
			((*simp).field_views[j]).field = 5;
		else // we have a problem here...
			return NULL;
		Py_DECREF(py_item2);
		
		py_item2 = PyDict_GetItemString(PyTuple_GetItem(py_item, j),"type");
		if(!strcmp(PyString_AsString(py_item2),"point"))
			((*simp).field_views[j]).type = 0;
		else if(!strcmp(PyString_AsString(py_item2),"xz"))
			((*simp).field_views[j]).type = 1;
		else if(!strcmp(PyString_AsString(py_item2),"yz"))
			((*simp).field_views[j]).type = 2;
		else if(!strcmp(PyString_AsString(py_item2),"xy"))
			((*simp).field_views[j]).type = 3;
		else // we have a problem here...
			return NULL;
		Py_DECREF(py_item2);

		py_item2 = PyDict_GetItemString(PyTuple_GetItem(py_item, j),"pos");
		if(((*simp).field_views[j]).type == 0){
			((*simp).field_views[j]).pos[0] = (int)PyInt_AsLong(PyTuple_GetItem(py_item2, 0));
			((*simp).field_views[j]).pos[1] = (int)PyInt_AsLong(PyTuple_GetItem(py_item2, 1));
			((*simp).field_views[j]).pos[2] = (int)PyInt_AsLong(PyTuple_GetItem(py_item2, 2));
		} else {
			((*simp).field_views[j]).pos[0] = (int)PyInt_AsLong(PyTuple_GetItem(py_item2, 0));
		}
		Py_DECREF(py_item2);
		
	}
	Py_DECREF(py_item);
	
	/* --------------------------------------------------------- */
	/* add some extra useful parameters to the python dictionary */

	(*simp).dt = ((*simp).courant)*((*simp).cell_size)/C0;
	(*simp).max_time = ((*simp).sim_time)/((*simp).dt) + 1;
	
	py_item = PyInt_FromLong((long)(*simp).max_time);
	PyDict_SetItemString((*simp).py_simp_dict, "max_time",py_item);
	Py_DECREF(py_item);

	py_item = PyFloat_FromDouble((double)(*simp).dt);
	PyDict_SetItemString((*simp).py_simp_dict, "dt",py_item);
	Py_DECREF(py_item);

	py_item = PyFloat_FromDouble((double)C0);
	PyDict_SetItemString((*simp).py_simp_dict, "C0",py_item);
	Py_DECREF(py_item);

	py_item = PyFloat_FromDouble((double)ETA0);
	PyDict_SetItemString((*simp).py_simp_dict, "ETA0",py_item);
	Py_DECREF(py_item);

	return simp;
}








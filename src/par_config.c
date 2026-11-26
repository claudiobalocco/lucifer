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
#include <string.h>
#include <stdlib.h>

/*
 Note on reference counting: PyDict_GetItemString and PyTuple_GetItem return
 BORROWED references, which must never be Py_DECREF'd. The strings returned by
 PyUnicode_AsUTF8 are owned by their str objects; py_simp_dict is kept alive for
 the whole run, so data_file, geom_file and src_files stay valid.
*/

// borrowed reference to dict[key], or NULL with an error message
static PyObject * get_item(PyObject *dict, const char *key) {
	PyObject *item = PyDict_GetItemString(dict, key);
	if(!item)
		printf(C_RED "[ERROR]" C_RES " %s not in dictionary\n", key);
	return item;
}

static int get_int(PyObject *dict, const char *key, int *out) {
	PyObject *item = get_item(dict, key);
	long v;
	if(!item)
		return 0;
	v = PyLong_AsLong(item);
	if(v == -1 && PyErr_Occurred()) {
		PyErr_Print();
		printf(C_RED "[ERROR]" C_RES " %s must be an integer\n", key);
		return 0;
	}
	*out = (int)v;
	return 1;
}

static int get_float(PyObject *dict, const char *key, float *out) {
	PyObject *item = get_item(dict, key);
	double v;
	if(!item)
		return 0;
	v = PyFloat_AsDouble(item);
	if(v == -1.0 && PyErr_Occurred()) {
		PyErr_Print();
		printf(C_RED "[ERROR]" C_RES " %s must be a number\n", key);
		return 0;
	}
	*out = (float)v;
	return 1;
}

// UTF-8 view of a str object, or NULL with an error message
static const char * as_str(PyObject *item, const char *what) {
	const char *s = item ? PyUnicode_AsUTF8(item) : NULL;
	if(!s) {
		if(PyErr_Occurred())
			PyErr_Print();
		printf(C_RED "[ERROR]" C_RES " %s must be a string\n", what);
	}
	return s;
}

static int get_str(PyObject *dict, const char *key, const char **out) {
	PyObject *item = get_item(dict, key);
	if(!item)
		return 0;
	*out = as_str(item, key);
	return *out != NULL;
}

// boundary condition: "pbc" or "abc" (anything else defaults to abc)
static int get_bc(PyObject *dict, const char *key, int *abc_v, int *pbc_v) {
	const char *s;
	if(!get_str(dict, key, &s))
		return 0;
	if(!strcmp(s, "pbc")) {
		*abc_v = 0;
		*pbc_v = 1;
	} else {
		if(strcmp(s, "abc"))
			printf(C_YELLOW "[WARNING]" C_RES " unknown boundary '%s' for %s, using abc\n", s, key);
		*abc_v = 1;
		*pbc_v = 0;
	}
	return 1;
}

// integer element idx of a tuple/list
static int seq_int(PyObject *seq, Py_ssize_t idx, int *out) {
	PyObject *item;
	long v;
	if(!PySequence_Check(seq) || PySequence_Size(seq) <= idx) {
		PyErr_Clear();
		return 0;
	}
	item = PySequence_GetItem(seq, idx); // new reference
	if(!item) {
		PyErr_Print();
		return 0;
	}
	v = PyLong_AsLong(item);
	Py_DECREF(item);
	if(v == -1 && PyErr_Occurred()) {
		PyErr_Print();
		return 0;
	}
	*out = (int)v;
	return 1;
}

static int field_index(const char *s) {
	static const char *names[6] = {"Ex", "Ey", "Ez", "Hx", "Hy", "Hz"};
	int i;
	for(i = 0; i < 6; i++)
		if(!strcmp(s, names[i]))
			return i;
	return -1;
}

static int view_type_index(const char *s) {
	static const char *names[4] = {"point", "xz", "yz", "xy"};
	int i;
	for(i = 0; i < 4; i++)
		if(!strcmp(s, names[i]))
			return i;
	return -1;
}

static int add_to_sys_path(const char *dir) {
	PyObject *sys_path = PySys_GetObject("path"); // borrowed
	PyObject *py_dir;
	int ret;
	if(!sys_path)
		return 0;
	py_dir = PyUnicode_DecodeFSDefault(dir);
	if(!py_dir) {
		PyErr_Print();
		return 0;
	}
	ret = PyList_Append(sys_path, py_dir);
	Py_DECREF(py_dir);
	return ret == 0;
}

LUC_sim_pars * par_config(const char *luc_name) {

	LUC_sim_pars *simp;
	PyObject *pluginModule, *filterFunc, *fname, *py_item, *py_view, *py_pos;
	int i, j, n_pos;
	const char *s;
	char *fpath, *fbase, *fslash, *fdotpy;

	simp = calloc(1, sizeof(LUC_sim_pars));
	if(!simp)
		return NULL;

	// split luc_name into directory (added to sys.path) and module name
	fpath = malloc(strlen(luc_name) + 1);
	if(!fpath)
		return NULL;
	strcpy(fpath, luc_name);

	fslash = strrchr(fpath, '/');
#ifdef _WIN32
	if(strrchr(fpath, '\\') > fslash)
		fslash = strrchr(fpath, '\\');
#endif
	if(fslash == NULL) {
		fbase = fpath;
		add_to_sys_path(".");
	} else {
		*fslash = '\0';
		fbase = fslash + 1;
		add_to_sys_path(fslash == fpath ? "/" : fpath);
	}

	fdotpy = strrchr(fbase, '.');
	if(fdotpy != NULL && !strcmp(fdotpy, ".py")) //strip off the .py extension if exist
		*fdotpy = '\0';

	fname = PyUnicode_DecodeFSDefault(fbase);
	free(fpath);
	if(!fname) {
		PyErr_Print();
		return NULL;
	}

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
		printf(C_RED "[ERROR]" C_RES " get_pars() in %s must return a dict\n",luc_name);
		return NULL;
	}

	/* --------------------------------------------------------- */

	if(!get_int((*simp).py_simp_dict, "size_x", &(*simp).size_x) ||
	   !get_int((*simp).py_simp_dict, "size_y", &(*simp).size_y) ||
	   !get_int((*simp).py_simp_dict, "size_z", &(*simp).size_z) ||
	   !get_float((*simp).py_simp_dict, "courant", &(*simp).courant) ||
	   !get_float((*simp).py_simp_dict, "cell_size", &(*simp).cell_size) ||
	   !get_float((*simp).py_simp_dict, "sim_time", &(*simp).sim_time))
		return NULL;

	if((*simp).size_x < 3 || (*simp).size_y < 3 || (*simp).size_z < 3) {
		printf(C_RED "[ERROR]" C_RES " size_x, size_y and size_z must be >= 3\n");
		return NULL;
	}
	(*simp).n_cells = (size_t)(*simp).size_x*(size_t)(*simp).size_y*(size_t)(*simp).size_z;

	if((*simp).courant <= 0 || (*simp).cell_size <= 0 || (*simp).sim_time <= 0) {
		printf(C_RED "[ERROR]" C_RES " courant, cell_size and sim_time must be > 0\n");
		return NULL;
	}
	if((*simp).courant > 0.57735f) // 1/sqrt(3)
		printf(C_YELLOW "[WARNING]" C_RES " courant = %g is above the 3D stability limit 1/sqrt(3)\n", (*simp).courant);

	/* --------------------------------------------------------- */

	// boundary conditions
	if(!get_bc((*simp).py_simp_dict, "front",  &(*simp).abc_f, &(*simp).pbc_f) ||
	   !get_bc((*simp).py_simp_dict, "back",   &(*simp).abc_k, &(*simp).pbc_k) ||
	   !get_bc((*simp).py_simp_dict, "top",    &(*simp).abc_t, &(*simp).pbc_t) ||
	   !get_bc((*simp).py_simp_dict, "bottom", &(*simp).abc_b, &(*simp).pbc_b) ||
	   !get_bc((*simp).py_simp_dict, "left",   &(*simp).abc_l, &(*simp).pbc_l) ||
	   !get_bc((*simp).py_simp_dict, "right",  &(*simp).abc_r, &(*simp).pbc_r))
		return NULL;

	// a periodic axis needs pbc on both of its faces
	if((*simp).pbc_f != (*simp).pbc_k || (*simp).pbc_l != (*simp).pbc_r || (*simp).pbc_t != (*simp).pbc_b) {
		printf(C_RED "[ERROR]" C_RES " pbc must be set on both faces of an axis (front/back, left/right, top/bottom)\n");
		return NULL;
	}
	if((*simp).pbc_f)
		printf(C_GREEN "[MSG]" C_RES " x is periodic: period is size_x-1 = %d cells, plane x=%d is the image of x=0\n", (*simp).size_x-1, (*simp).size_x-1);
	if((*simp).pbc_l)
		printf(C_GREEN "[MSG]" C_RES " y is periodic: period is size_y-1 = %d cells, plane y=%d is the image of y=0\n", (*simp).size_y-1, (*simp).size_y-1);
	if((*simp).pbc_t)
		printf(C_GREEN "[MSG]" C_RES " z is periodic: period is size_z-1 = %d cells, plane z=%d is the image of z=0\n", (*simp).size_z-1, (*simp).size_z-1);

	/* --------------------------------------------------------- */

	if(!get_str((*simp).py_simp_dict, "data_file", &(*simp).data_file) ||
	   !get_str((*simp).py_simp_dict, "geom_file", &(*simp).geom_file))
		return NULL;

	/* --------------------------------------------------------- */

	py_item = get_item((*simp).py_simp_dict, "sources");
	if(!py_item)
		return NULL;
	if(!PyTuple_Check(py_item)) {
		printf(C_RED "[ERROR]" C_RES " sources must be a tuple, e.g. ('src1', )\n");
		return NULL;
	}
	i = (int)PyTuple_Size(py_item);
	(*simp).n_sources = i;
	(*simp).src_files = malloc((i > 0 ? i : 1)*sizeof(const char*));
	if(!(*simp).src_files)
		return NULL;
	for(j = 0; j < i; j++) {
		(*simp).src_files[j] = as_str(PyTuple_GetItem(py_item, j), "sources item");
		if(!(*simp).src_files[j])
			return NULL;
	}

	/* --------------------------------------------------------- */

	py_item = get_item((*simp).py_simp_dict, "field_views");
	if(!py_item)
		return NULL;
	if(!PyTuple_Check(py_item)) {
		printf(C_RED "[ERROR]" C_RES " field_views must be a tuple, e.g. ({...}, )\n");
		return NULL;
	}
	i = (int)PyTuple_Size(py_item);
	(*simp).n_field_views = i;
	printf(C_GREEN "[MSG]" C_RES " there are %d field views\n",i);
	(*simp).field_views = malloc((i > 0 ? i : 1)*sizeof(LUC_field_view));
	if(!(*simp).field_views)
		return NULL;

	for(j = 0; j < i; j++) {
		LUC_field_view *fv = &((*simp).field_views[j]);
		py_view = PyTuple_GetItem(py_item, j); // borrowed
		if(!PyDict_Check(py_view)) {
			printf(C_RED "[ERROR]" C_RES " field view %d must be a dict\n", j);
			return NULL;
		}

		if(!get_str(py_view, "field", &s))
			return NULL;
		(*fv).field = field_index(s);
		if((*fv).field < 0) {
			printf(C_RED "[ERROR]" C_RES " field view %d: unknown field '%s'\n", j, s);
			return NULL;
		}

		if(!get_str(py_view, "type", &s))
			return NULL;
		(*fv).type = view_type_index(s);
		if((*fv).type < 0) {
			printf(C_RED "[ERROR]" C_RES " field view %d: unknown type '%s'\n", j, s);
			return NULL;
		}

		py_pos = get_item(py_view, "pos");
		if(!py_pos)
			return NULL;
		n_pos = ((*fv).type == 0) ? 3 : 1;
		(*fv).pos[0] = (*fv).pos[1] = (*fv).pos[2] = 0;
		for(int p = 0; p < n_pos; p++)
			if(!seq_int(py_pos, p, &(*fv).pos[p])) {
				printf(C_RED "[ERROR]" C_RES " field view %d: pos must be a tuple of %d int\n", j, n_pos);
				return NULL;
			}

		// bounds check, an out of range position would read outside the field arrays
		{
			int ok = 1;
			switch((*fv).type) {
				case 0: ok = (*fv).pos[0] >= 0 && (*fv).pos[0] < (*simp).size_x &&
				             (*fv).pos[1] >= 0 && (*fv).pos[1] < (*simp).size_y &&
				             (*fv).pos[2] >= 0 && (*fv).pos[2] < (*simp).size_z; break;
				case 1: ok = (*fv).pos[0] >= 0 && (*fv).pos[0] < (*simp).size_y; break; // xz plane at y
				case 2: ok = (*fv).pos[0] >= 0 && (*fv).pos[0] < (*simp).size_x; break; // yz plane at x
				case 3: ok = (*fv).pos[0] >= 0 && (*fv).pos[0] < (*simp).size_z; break; // xy plane at z
			}
			if(!ok) {
				printf(C_RED "[ERROR]" C_RES " field view %d: pos is outside the simulation domain\n", j);
				return NULL;
			}
		}
	}

	/* --------------------------------------------------------- */
	/* add some extra useful parameters to the python dictionary */

	(*simp).dt = ((*simp).courant)*((*simp).cell_size)/C0;
	(*simp).max_time = (int)(((*simp).sim_time)/((*simp).dt)) + 1;

	py_item = PyLong_FromLong((long)(*simp).max_time);
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

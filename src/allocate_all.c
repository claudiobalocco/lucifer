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
#include <stdlib.h>
#include <string.h>

/*
 Python 3 has no PyBuffer_New. The arrays shared with the Python scripts are
 allocated here in C and exposed to Python as writable memoryviews
 (PyMemoryView_FromMemory). numpy.frombuffer() on a memoryview gives a numpy
 array that points to the same memory: no copy is made in either direction.
 The C memory is never freed while the memoryviews exist (they live until exit).
*/
static PyObject * wrap_memory(void *ptr, size_t size) {
	PyObject *mv = PyMemoryView_FromMemory((char *)ptr, (Py_ssize_t)size, PyBUF_WRITE);
	if(!mv)
		PyErr_Print();
	return mv;
}

LUC_fields * allocate_fields(LUC_sim_pars *simp) {

	LUC_fields *fields;
	size_t n = (*simp).n_cells;
	size_t size = n*sizeof(float);
	const int sx = (*simp).size_x;
	const int sy = (*simp).size_y;
	const int sz = (*simp).size_z;
	const ptrdiff_t stride_x = (ptrdiff_t)sy*sz;
	int x, y;

	// buffer to gather one xy plane in write_field_views.c (small: size_x*size_y)
	(*simp).view_buf = (float *)malloc((size_t)sx*sy*sizeof(float));
	if(!(*simp).view_buf)
		return NULL;

	fields = malloc(sizeof(LUC_fields));
	if(!fields)
		return NULL;

	(*fields).Hx = (float *)malloc(size);
	(*fields).Hy = (float *)malloc(size);
	(*fields).Hz = (float *)malloc(size);
	(*fields).Ex = (float *)malloc(size);
	(*fields).Ey = (float *)malloc(size);
	(*fields).Ez = (float *)malloc(size);
	if(!(*fields).Hx || !(*fields).Hy || !(*fields).Hz || !(*fields).Ex || !(*fields).Ey || !(*fields).Ez)
		return NULL;

	// set initial fields to 0
	// done in parallel with the same (x,y) row split as updateE/updateH, so that on
	// multi-socket machines each page is placed on the node of the thread that updates it
	// (first touch). Call omp_set_num_threads before this.
	#pragma omp parallel for collapse(2) schedule(static)
	for(x = 0; x < sx; x++)
		for(y = 0; y < sy; y++) {
			const ptrdiff_t r = x*stride_x + (ptrdiff_t)y*sz;
			memset((*fields).Ex + r, 0, (size_t)sz*sizeof(float));
			memset((*fields).Ey + r, 0, (size_t)sz*sizeof(float));
			memset((*fields).Ez + r, 0, (size_t)sz*sizeof(float));
			memset((*fields).Hx + r, 0, (size_t)sz*sizeof(float));
			memset((*fields).Hy + r, 0, (size_t)sz*sizeof(float));
			memset((*fields).Hz + r, 0, (size_t)sz*sizeof(float));
		}
	(void)n;

	return fields;
}

LUC_diel_mat * allocate_diel(LUC_sim_pars *simp) {

	LUC_diel_mat *diel;
	size_t xyzi_size = (*simp).n_cells*sizeof(unsigned short);
	size_t mat_size = MAX_NUM_OF_MATERIALS*sizeof(float);

	const int sx = (*simp).size_x;
	const int sy = (*simp).size_y;
	const int sz = (*simp).size_z;
	const ptrdiff_t stride_x = (ptrdiff_t)sy*sz;
	int x, y;

	diel = malloc(sizeof(LUC_diel_mat));
	if(!diel)
		return NULL;

	(*diel).material = (unsigned short *)malloc(xyzi_size);
	if(!(*diel).material)
		return NULL;
	// all is vacuum; zeroed in parallel for first touch placement (see allocate_fields)
	#pragma omp parallel for collapse(2) schedule(static)
	for(x = 0; x < sx; x++)
		for(y = 0; y < sy; y++)
			memset((*diel).material + x*stride_x + (ptrdiff_t)y*sz, 0, (size_t)sz*sizeof(unsigned short));
	(*diel).py_material = wrap_memory((*diel).material, xyzi_size);
	if(!(*diel).py_material)
		return NULL;

	// update coefficients
	(*diel).Cexe = (float *)calloc(MAX_NUM_OF_MATERIALS, sizeof(float));
	(*diel).Cexh = (float *)calloc(MAX_NUM_OF_MATERIALS, sizeof(float));
	(*diel).Ceye = (float *)calloc(MAX_NUM_OF_MATERIALS, sizeof(float));
	(*diel).Ceyh = (float *)calloc(MAX_NUM_OF_MATERIALS, sizeof(float));
	(*diel).Ceze = (float *)calloc(MAX_NUM_OF_MATERIALS, sizeof(float));
	(*diel).Cezh = (float *)calloc(MAX_NUM_OF_MATERIALS, sizeof(float));
	if(!(*diel).Cexe || !(*diel).Cexh || !(*diel).Ceye || !(*diel).Ceyh || !(*diel).Ceze || !(*diel).Cezh)
		return NULL;

	(*diel).py_Cexe = wrap_memory((*diel).Cexe, mat_size);
	(*diel).py_Cexh = wrap_memory((*diel).Cexh, mat_size);
	(*diel).py_Ceye = wrap_memory((*diel).Ceye, mat_size);
	(*diel).py_Ceyh = wrap_memory((*diel).Ceyh, mat_size);
	(*diel).py_Ceze = wrap_memory((*diel).Ceze, mat_size);
	(*diel).py_Cezh = wrap_memory((*diel).Cezh, mat_size);
	if(!(*diel).py_Cexe || !(*diel).py_Cexh || !(*diel).py_Ceye || !(*diel).py_Ceyh || !(*diel).py_Ceze || !(*diel).py_Cezh)
		return NULL;

	return diel;
}

LUC_abc_fields * allocate_abc(LUC_sim_pars *simp) {

	LUC_abc_fields *abc_f;
	size_t sizex = (size_t)(*simp).size_y*(*simp).size_z;
	size_t sizey = (size_t)(*simp).size_x*(*simp).size_z;
	size_t sizez = (size_t)(*simp).size_x*(*simp).size_y;

	abc_f = malloc(sizeof(LUC_abc_fields));
	if(!abc_f)
		return NULL;

	// calloc returns zeroed memory
	(*abc_f).Eyx0 = (float *)calloc(sizex, sizeof(float));
	(*abc_f).Ezx0 = (float *)calloc(sizex, sizeof(float));
	(*abc_f).Eyx1 = (float *)calloc(sizex, sizeof(float));
	(*abc_f).Ezx1 = (float *)calloc(sizex, sizeof(float));

	(*abc_f).Exy0 = (float *)calloc(sizey, sizeof(float));
	(*abc_f).Ezy0 = (float *)calloc(sizey, sizeof(float));
	(*abc_f).Exy1 = (float *)calloc(sizey, sizeof(float));
	(*abc_f).Ezy1 = (float *)calloc(sizey, sizeof(float));

	(*abc_f).Exz0 = (float *)calloc(sizez, sizeof(float));
	(*abc_f).Eyz0 = (float *)calloc(sizez, sizeof(float));
	(*abc_f).Exz1 = (float *)calloc(sizez, sizeof(float));
	(*abc_f).Eyz1 = (float *)calloc(sizez, sizeof(float));

	if(!(*abc_f).Eyx0 || !(*abc_f).Ezx0 || !(*abc_f).Eyx1 || !(*abc_f).Ezx1 ||
	   !(*abc_f).Exy0 || !(*abc_f).Ezy0 || !(*abc_f).Exy1 || !(*abc_f).Ezy1 ||
	   !(*abc_f).Exz0 || !(*abc_f).Eyz0 || !(*abc_f).Exz1 || !(*abc_f).Eyz1)
		return NULL;

	return abc_f;
}

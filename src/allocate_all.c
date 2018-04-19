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
#include <stdlib.h>
#include <string.h>
#include "lucifer_global_def.h"	

LUC_fields * allocate_fields(LUC_sim_pars *simp) {

	LUC_fields *fields;
	
	fields = malloc(sizeof(LUC_fields));
	
	int size = (*simp).size_x*(*simp).size_y*(*simp).size_z*sizeof(float);
	int i;	

	(*fields).Hx = (float *)malloc(size);
	(*fields).Hy = (float *)malloc(size);
	(*fields).Hz = (float *)malloc(size);
	(*fields).Ex = (float *)malloc(size);
	(*fields).Ey = (float *)malloc(size);
	(*fields).Ez = (float *)malloc(size);

	// set initial fields to 0
	/* memset((*fields).Hx, 0, size);
	memset((*fields).Hy, 0, size);
	memset((*fields).Hz, 0, size);
	memset((*fields).Ex, 0, size);
	memset((*fields).Ey, 0, size);
	memset((*fields).Ez, 0, size); */

	#pragma omp parallel for private(i)
	for(i = 0; i < (*simp).size_x*(*simp).size_y*(*simp).size_z; i++) {
		(*fields).Ex[i] = 0;
		(*fields).Ey[i] = 0;
		(*fields).Ez[i] = 0;
		(*fields).Hx[i] = 0;
		(*fields).Hy[i] = 0;
		(*fields).Hz[i] = 0;
	}

	return fields;
}

LUC_diel_mat * allocate_diel(LUC_sim_pars *simp) {

	LUC_diel_mat *diel;

	diel = malloc(sizeof(LUC_diel_mat));

	int xyzi_size = (*simp).size_x*(*simp).size_y*(*simp).size_z*sizeof(unsigned short);
	int mat_size = MAX_NUM_OF_MATERIALS*sizeof(float);
	Py_ssize_t s;

	(*diel).py_material = PyBuffer_New(xyzi_size);
	(*diel).material = (unsigned short *)malloc(xyzi_size);
	PyObject_AsWriteBuffer((*diel).py_material, (void **)&((*diel).material), &s);
	memset((*diel).material, 0, xyzi_size); // all is vacuum

	// update coefficients
	(*diel).py_Cexe = PyBuffer_New(mat_size);
	(*diel).py_Cexh = PyBuffer_New(mat_size);
	(*diel).py_Ceye = PyBuffer_New(mat_size);
	(*diel).py_Ceyh = PyBuffer_New(mat_size);
	(*diel).py_Ceze = PyBuffer_New(mat_size);
	(*diel).py_Cezh = PyBuffer_New(mat_size);

	(*diel).Cexe = (float *)malloc(mat_size);
	(*diel).Cexh = (float *)malloc(mat_size);
	(*diel).Ceye = (float *)malloc(mat_size);
	(*diel).Ceyh = (float *)malloc(mat_size);
	(*diel).Ceze = (float *)malloc(mat_size);
	(*diel).Cezh = (float *)malloc(mat_size);

	PyObject_AsWriteBuffer((*diel).py_Cexe, (void **)&((*diel).Cexe), &s);
	PyObject_AsWriteBuffer((*diel).py_Cexh, (void **)&((*diel).Cexh), &s);
	PyObject_AsWriteBuffer((*diel).py_Ceye, (void **)&((*diel).Ceye), &s);
	PyObject_AsWriteBuffer((*diel).py_Ceyh, (void **)&((*diel).Ceyh), &s);
	PyObject_AsWriteBuffer((*diel).py_Ceze, (void **)&((*diel).Ceze), &s);
	PyObject_AsWriteBuffer((*diel).py_Cezh, (void **)&((*diel).Cezh), &s);

	return diel;
}

LUC_abc_fields * allocate_abc(LUC_sim_pars *simp) {

	LUC_abc_fields *abc_f;
	
	abc_f = malloc(sizeof(LUC_abc_fields));

	int sizex = (*simp).size_y*(*simp).size_z*sizeof(float);
	int sizey = (*simp).size_x*(*simp).size_z*sizeof(float);
	int sizez = (*simp).size_x*(*simp).size_y*sizeof(float);

	(*abc_f).Eyx0 = (float *)malloc(sizex);
	(*abc_f).Ezx0 = (float *)malloc(sizex);
	(*abc_f).Eyx1 = (float *)malloc(sizex);
	(*abc_f).Ezx1 = (float *)malloc(sizex);
		    
	(*abc_f).Exy0 = (float *)malloc(sizey);
	(*abc_f).Ezy0 = (float *)malloc(sizey);
	(*abc_f).Exy1 = (float *)malloc(sizey);
	(*abc_f).Ezy1 = (float *)malloc(sizey);

	(*abc_f).Exz0 = (float *)malloc(sizez);
	(*abc_f).Eyz0 = (float *)malloc(sizez);
	(*abc_f).Exz1 = (float *)malloc(sizez);
	(*abc_f).Eyz1 = (float *)malloc(sizez);

	memset((*abc_f).Eyx0, 0, sizex);
	memset((*abc_f).Ezx0, 0, sizex);
	memset((*abc_f).Eyx1, 0, sizex);
	memset((*abc_f).Ezx1, 0, sizex);

	memset((*abc_f).Exy0, 0, sizey);
	memset((*abc_f).Ezy0, 0, sizey);
	memset((*abc_f).Exy1, 0, sizey);
	memset((*abc_f).Ezy1, 0, sizey);
	
    memset((*abc_f).Exz0, 0, sizez);
	memset((*abc_f).Eyz0, 0, sizez);
	memset((*abc_f).Exz1, 0, sizez);
	memset((*abc_f).Eyz1, 0, sizez);

	return abc_f;
}






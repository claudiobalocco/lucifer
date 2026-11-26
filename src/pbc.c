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

/*
 Periodic boundary conditions.

 A periodic axis of size N has period N-1 cells: plane N-1 is the image of
 plane 0. updateE.c updates plane 0 using plane N-2 as its "-1" neighbour;
 this function then copies plane 0 into plane N-1 for all six components, so
 the H update of the next step (which reads E at index+1) and the saved field
 views see the periodic values. It must run after the sources and the ABC.
 pbc has to be set on both faces of an axis (checked in par_config.c).
*/

static void copy_x(float *LUC_RESTRICT f, int sx, ptrdiff_t stride_x) {
	memcpy(f + (sx-1)*stride_x, f, (size_t)stride_x*sizeof(float));
}

static void copy_y(float *LUC_RESTRICT f, int sx, int sy, int sz) {
	int x;
	const ptrdiff_t stride_x = (ptrdiff_t)sy*sz;
	#pragma omp parallel for schedule(static)
	for (x = 0; x < sx; x++)
		memcpy(f + x*stride_x + (ptrdiff_t)(sy-1)*sz, f + x*stride_x, (size_t)sz*sizeof(float));
}

static void copy_z(float *LUC_RESTRICT f, int sx, int sy, int sz) {
	int x, y;
	const ptrdiff_t stride_x = (ptrdiff_t)sy*sz;
	#pragma omp parallel for collapse(2) schedule(static)
	for (x = 0; x < sx; x++)
		for (y = 0; y < sy; y++) {
			float *row = f + x*stride_x + (ptrdiff_t)y*sz;
			row[sz-1] = row[0];
		}
}

int pbc(LUC_fields *fields, LUC_sim_pars *simp) {

	const int sx = (*simp).size_x;
	const int sy = (*simp).size_y;
	const int sz = (*simp).size_z;
	float *f[6] = {(*fields).Ex, (*fields).Ey, (*fields).Ez, (*fields).Hx, (*fields).Hy, (*fields).Hz};
	int c;

	if ((*simp).pbc_f && (*simp).pbc_k)
		for (c = 0; c < 6; c++)
			copy_x(f[c], sx, (ptrdiff_t)sy*sz);

	if ((*simp).pbc_l && (*simp).pbc_r)
		for (c = 0; c < 6; c++)
			copy_y(f[c], sx, sy, sz);

	if ((*simp).pbc_b && (*simp).pbc_t)
		for (c = 0; c < 6; c++)
			copy_z(f[c], sx, sy, sz);

	return 1;
}

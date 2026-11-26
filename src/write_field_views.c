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

/*
 Writes the field views of the current time step, in the order of the
 configuration file. Data layout (float32, unchanged from earlier versions):
   point: 1 value
   xz (at y=pos): size_x*size_z values, x outer, z inner
   yz (at x=pos): size_y*size_z values, y outer, z inner
   xy (at z=pos): size_x*size_y values, x outer, y inner
 Contiguous data is written directly from the field array (no copy): one call
 per xz row and one call for a whole yz plane. An xy plane is strided in memory
 and is gathered into the small view_buf (size_x*size_y floats) first.
*/

int write_field_views(FILE *fp, LUC_fields *fields, LUC_sim_pars *simp) {

	const int sx = (*simp).size_x;
	const int sy = (*simp).size_y;
	const int sz = (*simp).size_z;
	const ptrdiff_t stride_x = (ptrdiff_t)sy*sz;
	const float *f[6] = {(*fields).Ex, (*fields).Ey, (*fields).Ez, (*fields).Hx, (*fields).Hy, (*fields).Hz};
	float *LUC_RESTRICT buf = (*simp).view_buf;
	const float *LUC_RESTRICT a;
	int cnt, i, j, k;

	for(cnt = 0; cnt < (*simp).n_field_views; cnt++) {
		const LUC_field_view *fv = &((*simp).field_views[cnt]);

		if((*fv).field < 0 || (*fv).field > 5)
			return 0;
		a = f[(*fv).field];

		switch((*fv).type) {

			case 0: // this is a point
				i = (*fv).pos[0];
				j = (*fv).pos[1];
				k = (*fv).pos[2];
				fwrite(a + i*stride_x + (ptrdiff_t)j*sz + k, sizeof(float), 1, fp);
				break;

			case 1: // xz at y = pos
				j = (*fv).pos[0];
				for (i = 0; i < sx; i++)
					fwrite(a + i*stride_x + (ptrdiff_t)j*sz, sizeof(float), (size_t)sz, fp);
				break;

			case 2: // yz at x = pos: the whole plane is contiguous
				i = (*fv).pos[0];
				fwrite(a + i*stride_x, sizeof(float), (size_t)stride_x, fp);
				break;

			case 3: // xy at z = pos
				k = (*fv).pos[0];
				#pragma omp parallel for private(j)
				for (i = 0; i < sx; i++)
					for (j = 0; j < sy; j++)
						buf[(ptrdiff_t)i*sy + j] = a[i*stride_x + (ptrdiff_t)j*sz + k];
				fwrite(buf, sizeof(float), (size_t)sx*sy, fp);
				break;

			default:
				return 0;
		} // switch
	} // for
	return 1;
} // function

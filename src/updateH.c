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

/*
 H update, one pass over the (x,y) rows (see updateE.c).
 Update ranges (same as the original three loops):
   Hx: x in [0,sx)   y in [0,sy-1) z in [0,sz-1)
   Hy: x in [0,sx-1) y in [0,sy)   z in [0,sz-1)
   Hz: x in [0,sx-1) y in [0,sy-1) z in [0,sz)
*/

int updateH(LUC_fields *fields, LUC_diel_mat *diel, LUC_sim_pars *simp) {

	const float coef = (float)((*simp).courant/ETA0);

	const int sx = (*simp).size_x;
	const int sy = (*simp).size_y;
	const int sz = (*simp).size_z;
	const ptrdiff_t stride_x = (ptrdiff_t)sy*sz;
	const ptrdiff_t stride_y = sz;

	float *LUC_RESTRICT hx = (*fields).Hx;
	float *LUC_RESTRICT hy = (*fields).Hy;
	float *LUC_RESTRICT hz = (*fields).Hz;
	const float *LUC_RESTRICT ex = (*fields).Ex;
	const float *LUC_RESTRICT ey = (*fields).Ey;
	const float *LUC_RESTRICT ez = (*fields).Ez;

	int x, y;
	(void)diel;

	#pragma omp parallel for collapse(2) schedule(static)
	for (x = 0; x < sx; x++) {
		for (y = 0; y < sy; y++) {
			const ptrdiff_t r   = x*stride_x + y*stride_y; // row (x,y)
			const ptrdiff_t rxp = r + stride_x;            // row (x+1,y), used only if x < sx-1
			const ptrdiff_t ryp = r + stride_y;            // row (x,y+1), used only if y < sy-1
			int z;

			if (y < sy-1) // Hx
				for (z = 0; z < sz-1; z++)
					hx[r+z] = hx[r+z]+coef*(ey[r+z+1]-ey[r+z]-ez[ryp+z]+ez[r+z]);

			if (x < sx-1) // Hy
				for (z = 0; z < sz-1; z++)
					hy[r+z] = hy[r+z]+coef*(ez[rxp+z]-ez[r+z]-ex[r+z+1]+ex[r+z]);

			if (x < sx-1 && y < sy-1) // Hz
				for (z = 0; z < sz; z++)
					hz[r+z] = hz[r+z]+coef*(ex[ryp+z]-ex[r+z]-ey[rxp+z]+ey[r+z]);
		}
	}

	return 1;
}

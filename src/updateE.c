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
 E update. Arrays are stored as a[x*sy*sz + y*sz + z] (z contiguous).

 The three components are updated in one pass over the (x,y) rows: for each row
 Ex, Ey and Ez are updated one after the other, so the H rows they share are
 still in cache. Each inner z loop has no aliasing (restrict) and can be vectorized.
 Offsets are ptrdiff_t, so domains above 2^31 cells work.

 Update ranges (same as the original three loops):
   Ex: x in [0,sx-1)   y in [ylo,sy-1)  z in [zlo,sz-1)
   Ey: x in [xlo,sx-1) y in [0,sy-1)    z in [zlo,sz-1)
   Ez: x in [xlo,sx-1) y in [ylo,sy-1)  z in [0,sz-1)
 where xlo/ylo/zlo is 1, or 0 for a periodic axis. On a periodic axis the plane
 at index size-1 is the image of plane 0 (period = size-1 cells), so the "-1"
 neighbour of index 0 is index size-2. pbc.c copies plane 0 into plane size-1.
*/

int updateE(LUC_fields *fields, LUC_diel_mat *diel, LUC_sim_pars *simp) {

	const int sx = (*simp).size_x;
	const int sy = (*simp).size_y;
	const int sz = (*simp).size_z;
	const ptrdiff_t stride_x = (ptrdiff_t)sy*sz;
	const ptrdiff_t stride_y = sz;

	const int px = (*simp).pbc_f && (*simp).pbc_k;
	const int py = (*simp).pbc_l && (*simp).pbc_r;
	const int pz = (*simp).pbc_b && (*simp).pbc_t;
	const int xlo = px ? 0 : 1;
	const int ylo = py ? 0 : 1;

	float *LUC_RESTRICT ex = (*fields).Ex;
	float *LUC_RESTRICT ey = (*fields).Ey;
	float *LUC_RESTRICT ez = (*fields).Ez;
	const float *LUC_RESTRICT hx = (*fields).Hx;
	const float *LUC_RESTRICT hy = (*fields).Hy;
	const float *LUC_RESTRICT hz = (*fields).Hz;
	const unsigned short *LUC_RESTRICT mat = (*diel).material;
	const float *LUC_RESTRICT cexe = (*diel).Cexe;
	const float *LUC_RESTRICT cexh = (*diel).Cexh;
	const float *LUC_RESTRICT ceye = (*diel).Ceye;
	const float *LUC_RESTRICT ceyh = (*diel).Ceyh;
	const float *LUC_RESTRICT ceze = (*diel).Ceze;
	const float *LUC_RESTRICT cezh = (*diel).Cezh;

	int x, y;

	#pragma omp parallel for collapse(2) schedule(static)
	for (x = 0; x < sx-1; x++) {
		for (y = 0; y < sy-1; y++) {
			const ptrdiff_t r   = x*stride_x + y*stride_y;                     // row (x,y)
			const ptrdiff_t rxm = (x > 0 ? x-1 : sx-2)*stride_x + y*stride_y;  // row (x-1,y)
			const ptrdiff_t rym = x*stride_x + (y > 0 ? y-1 : sy-2)*stride_y;  // row (x,y-1)
			int z, m;

			if (y >= ylo) { // Ex
				for (z = 1; z < sz-1; z++) {
					m = mat[r+z];
					ex[r+z] = cexe[m]*ex[r+z]+cexh[m]*(hz[r+z]-hz[rym+z]-hy[r+z]+hy[r+z-1]);
				}
				if (pz) {
					m = mat[r];
					ex[r] = cexe[m]*ex[r]+cexh[m]*(hz[r]-hz[rym]-hy[r]+hy[r+sz-2]);
				}
			}

			if (x >= xlo) { // Ey
				for (z = 1; z < sz-1; z++) {
					m = mat[r+z];
					ey[r+z] = ceye[m]*ey[r+z]+ceyh[m]*(hx[r+z]-hx[r+z-1]-hz[r+z]+hz[rxm+z]);
				}
				if (pz) {
					m = mat[r];
					ey[r] = ceye[m]*ey[r]+ceyh[m]*(hx[r]-hx[r+sz-2]-hz[r]+hz[rxm]);
				}
			}

			if (x >= xlo && y >= ylo) { // Ez
				for (z = 0; z < sz-1; z++) {
					m = mat[r+z];
					ez[r+z] = ceze[m]*ez[r+z]+cezh[m]*(hy[r+z]-hy[rxm+z]-hx[r+z]+hx[rym+z]);
				}
			}
		}
	}

	return 1;
}

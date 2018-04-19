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

#define EX (*fields).Ex
#define EY (*fields).Ey
#define EZ (*fields).Ez
#define HX (*fields).Hx
#define HY (*fields).Hy
#define HZ (*fields).Hz

#define XYZ x*sy*sz+y*sz+z
#define xYZ (x-1)*sy*sz+y*sz+z
#define XyZ x*sy*sz+(y-1)*sz+z
#define XYz x*sy*sz+y*sz+(z-1)

#define CEXE (*diel).Cexe[(*diel).material[x*sy*sz+y*sz+z]]
#define CEXH (*diel).Cexh[(*diel).material[x*sy*sz+y*sz+z]]
#define CEYE (*diel).Ceye[(*diel).material[x*sy*sz+y*sz+z]]
#define CEYH (*diel).Ceyh[(*diel).material[x*sy*sz+y*sz+z]]
#define CEZE (*diel).Ceze[(*diel).material[x*sy*sz+y*sz+z]]
#define CEZH (*diel).Cezh[(*diel).material[x*sy*sz+y*sz+z]]

int updateE(LUC_fields *fields, LUC_diel_mat *diel, LUC_sim_pars *simp) {

	int x, y, z;
	int sx = (*simp).size_x;
	int sy = (*simp).size_y;
	int sz = (*simp).size_z;


	#pragma omp parallel for private(x,y,z)
	for (x=0;x<sx-1;x++)
		for (y=1;y<sy-1;y++)
			for (z=1;z<sz-1;z++)
				EX[XYZ] = CEXE*EX[XYZ]+CEXH*(HZ[XYZ]-HZ[XyZ]-HY[XYZ]+HY[XYz]);

	#pragma omp parallel for private(x,y,z)
	for (x=1;x<sx-1;x++)
		for (y=0;y<sy-1;y++)
			for (z=1;z<sz-1;z++)
				EY[XYZ] = CEYE*EY[XYZ]+CEYH*(HX[XYZ]-HX[XYz]-HZ[XYZ]+HZ[xYZ]);

	#pragma omp parallel for private(x,y,z)
	for (x=1;x<sx-1;x++)
		for (y=1;y<sy-1;y++)
			for (z=0;z<sz-1;z++)
				EZ[XYZ] = CEZE*EZ[XYZ]+CEZH*(HY[XYZ]-HY[xYZ]-HX[XYZ]+HX[XyZ]);

	return 1;
}






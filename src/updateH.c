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

#define xyz x*sy*sz+y*sz+z
#define Xyz (x+1)*sy*sz+y*sz+z
#define xYz x*sy*sz+(y+1)*sz+z
#define xyZ x*sy*sz+y*sz+(z+1)

int updateH(LUC_fields *fields, LUC_diel_mat *diel, LUC_sim_pars *simp) {

	float coef = (*simp).courant/ETA0;

	int sx = (*simp).size_x;
	int sy = (*simp).size_y;
	int sz = (*simp).size_z;

	int x, y, z;
	
	#pragma omp parallel for private(x,y,z)
	for (x = 0; x < sx; x++)
		for (y = 0; y < sy-1; y++)
			for (z = 0; z < sz-1; z++)
				HX[xyz] = HX[xyz]+coef*(EY[xyZ]-EY[xyz]-EZ[xYz]+EZ[xyz]);
    	
	#pragma omp parallel for private(x,y,z)
	for (x=0;x<sx-1;x++)
		for (y=0;y<sy;y++)
			for (z=0;z<sz-1;z++)
				HY[xyz] = HY[xyz]+coef*(EZ[Xyz]-EZ[xyz]-EX[xyZ]+EX[xyz]);

	#pragma omp parallel for private(x,y,z)
	for (x=0;x<sx-1;x++)
		for (y=0;y<sy-1;y++)
			for (z=0;z<sz;z++)
				HZ[xyz] = HZ[xyz]+coef*(EX[xYz]-EX[xyz]-EY[Xyz]+EY[xyz]);

	return 1;

}



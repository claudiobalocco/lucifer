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

#define sX (*simp).size_x
#define sZ (*simp).size_z
#define sY (*simp).size_y
#define sYsZ (*simp).size_y*(*simp).size_z

#define EX (*fields).Ex
#define EY (*fields).Ey
#define EZ (*fields).Ez

#define EYX0 (*abc_fields).Eyx0
#define EZX0 (*abc_fields).Ezx0
#define EYX1 (*abc_fields).Eyx1
#define EZX1 (*abc_fields).Ezx1

#define EXY0 (*abc_fields).Exy0
#define EZY0 (*abc_fields).Ezy0
#define EXY1 (*abc_fields).Exy1
#define EZY1 (*abc_fields).Ezy1

#define EXZ0 (*abc_fields).Exz0
#define EYZ0 (*abc_fields).Eyz0
#define EXZ1 (*abc_fields).Exz1
#define EYZ1 (*abc_fields).Eyz1

int abc(LUC_fields *fields, LUC_abc_fields *abc_fields, LUC_sim_pars *simp) {
	
	float abccoef = ((*simp).courant-1)/((*simp).courant+1);
	int i,j,k;

	if((*simp).abc_k) {
	//back
		for(j=0;j<(sY-1);j++)
			for(k=0;k<sZ;k++) {
				EY[0*sYsZ+j*sZ+k] = EYX0[j*sZ+k]+abccoef*(EY[1*sYsZ+j*sZ+k]-EY[0*sYsZ+j*sZ+k]);
				EYX0[j*sZ+k] = EY[1*sYsZ+j*sZ+k];
			}
		for(j=0;j<sY;j++)
			for(k=0;k<(sZ-1);k++) {
				EZ[0*sYsZ+j*sZ+k] = EZX0[j*sZ+k]+abccoef*(EZ[1*sYsZ+j*sZ+k]-EZ[0*sYsZ+j*sZ+k]);
				EZX0[j*sZ+k] = EZ[1*sYsZ+j*sZ+k];
			}
	}
	
	if((*simp).abc_f) {
	//front
		for(j=0;j<(sY-1);j++)
			for(k=0;k<sZ;k++) {
				EY[(sX-1)*sYsZ+j*sZ+k] = EYX1[j*sZ+k]+abccoef*(EY[(sX-2)*sYsZ+j*sZ+k]-EY[(sX-1)*sYsZ+j*sZ+k]);
				EYX1[j*sZ+k] = EY[(sX-2)*sYsZ+j*sZ+k];
			}
		for(j=0;j<sY;j++)
			for(k=0;k<(sZ-1);k++) {
				EZ[(sX-1)*sYsZ+j*sZ+k] = EZX1[j*sZ+k]+abccoef*(EZ[(sX-2)*sYsZ+j*sZ+k]-EZ[(sX-1)*sYsZ+j*sZ+k]);
				EZX1[j*sZ+k] = EZ[(sX-2)*sYsZ+j*sZ+k];
			}
	}
	
	if((*simp).abc_l) {
	//left
		for(i=0;i<(sX-1);i++)
			for(k=0;k<sZ;k++) {
				EX[i*sYsZ+0*sZ+k] = EXY0[i*sZ+k]+abccoef*(EX[i*sYsZ+1*sZ+k]-EX[i*sYsZ+0*sZ+k]);
				EXY0[i*sZ+k] = EX[i*sYsZ+1*sZ+k];
			}
		for(i=0;i<sX;i++)
			for(k=0;k<(sZ-1);k++) {
				EZ[i*sYsZ+0*sZ+k] = EZY0[i*sZ+k]+abccoef*(EZ[i*sYsZ+1*sZ+k]-EZ[i*sYsZ+0*sZ+k]);
				EZY0[i*sZ+k] = EZ[i*sYsZ+1*sZ+k];
			}
	}
	
	if((*simp).abc_r) {
	//right
		for(i=0;i<(sX-1);i++)
			for(k=0;k<sZ;k++) {
				EX[i*sYsZ+(sY-1)*sZ+k] = EXY1[i*sZ+k]+abccoef*(EX[i*sYsZ+(sY-2)*sZ+k]-EX[i*sYsZ+(sY-1)*sZ+k]);
				EXY1[i*sZ+k] = EX[i*sYsZ+(sY-2)*sZ+k];
			}
		for(i=0;i<sX;i++)
			for(k=0;k<(sZ-1);k++) {
				EZ[i*sYsZ+(sY-1)*sZ+k] = EZY1[i*sZ+k]+abccoef*(EZ[i*sYsZ+(sY-2)*sZ+k]-EZ[i*sYsZ+(sY-1)*sZ+k]);
				EZY1[i*sZ+k] = EZ[i*sYsZ+(sY-2)*sZ+k];
			}
	}
	
	if((*simp).abc_b) {
	//bottom
		for(i=0;i<(sX-1);i++)
			for(j=0;j<sY;j++) {
				EX[i*sYsZ+j*sZ+0] = EXZ0[i*sY+j]+abccoef*(EX[i*sYsZ+j*sZ+1]-EX[i*sYsZ+j*sZ+0]);
				EXZ0[i*sY+j] = EX[i*sYsZ+j*sZ+1];
			}
		for(i=0;i<sX;i++)
			for(j=0;j<(sY-1);j++) {
				EY[i*sYsZ+j*sZ+0] = EYZ0[i*sY+j]+abccoef*(EY[i*sYsZ+j*sZ+1]-EY[i*sYsZ+j*sZ+0]);
				EYZ0[i*sY+j] = EY[i*sYsZ+j*sZ+1];
			}
	}
	
	if((*simp).abc_t) {
	//top
		for(i=0;i<(sX-1);i++)
			for(j=0;j<sY;j++) {
				EX[i*sYsZ+j*sZ+sZ-1] = EXZ1[i*sY+j]+abccoef*(EX[i*sYsZ+j*sZ+sZ-2]-EX[i*sYsZ+j*sZ+sZ-1]);
				EXZ1[i*sY+j] = EX[i*sYsZ+j*sZ+sZ-2];
			}
		for(i=0;i<sX;i++)
			for(j=0;j<(sY-1);j++) {
				EY[i*sYsZ+j*sZ+sZ-1] = EYZ1[i*sY+j]+abccoef*(EY[i*sYsZ+j*sZ+sZ-2]-EY[i*sYsZ+j*sZ+sZ-1]);
				EYZ1[i*sY+j] = EY[i*sYsZ+j*sZ+sZ-2];
			}
	}

	return 1;
}





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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "lucifer_global_def.h"

int write_header(FILE *fp, LUC_sim_pars *simp) {

	char header_text[128];
	unsigned short SX,SY,SZ;
	unsigned short n_timesteps;
	float CN;
	unsigned short number_of_slices = 1;
	char field[3], ch;
	unsigned short pos;
	
	SX = (unsigned short)(*simp).size_x;
	SY = (unsigned short)(*simp).size_y;
	SZ = (unsigned short)(*simp).size_z;
	CN = (float)(*simp).courant;
	n_timesteps = (unsigned short)(*simp).max_time;
	
	// ------- header structure ----------
	// ------ free text (128 bytes)-------
	//
	// ------- sim size (3*2 bytes)-------
	// 3 integer*2 SizeX, SizeY, and SizeZ
	// ------- max time (2 bytes) --------
	// integer*2
	// ----- courant number (4 bytes)-----
	// real*4
	// ------- # of slices (2 bytes) -----
	// 1 integer*2
	// -------- 1 slice (5 bytes) --------
	// 1 char [x|y|z] + 2 char field + location (integer*2)
	// ------------- [...] ---------------
	// ----------- data start ------------
	
	memset(header_text,0,128);
	strcpy(header_text, "LUCIFER v0.3\n");
	fwrite(header_text, 128, 1, fp);
	
	fwrite(&SX, 2, 1, fp);
	fwrite(&SY, 2, 1, fp);
	fwrite(&SZ, 2, 1, fp);
	fwrite(&n_timesteps, 2, 1, fp);
	fwrite(&CN, 4, 1, fp);
	fwrite(&number_of_slices, 2, 1, fp);
	strcpy(field, "Ex");
	fwrite(&field, 2, 1, fp);
	ch = 'z';
	fwrite(&ch, 1, 1, fp);
	pos = 5;
	fwrite(&pos, 2, 1, fp);
	
	return 1;
}

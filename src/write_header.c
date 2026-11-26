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
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
 ------------- data file, format 2 (Lucifer 1.0.0) -------------
 all values little endian, no padding

 ------ free text (128 bytes) ------
 "Lucifer 1.0.0\n" padded with zeros
 ------ int32 format ---------------  = 2
 ------ int32 size_x, size_y, size_z
 ------ int32 # of time steps ------
 ------ float32 courant number -----
 ------ float32 cell size (m) ------
 ------ float32 time step dt (s) ---
 ------ int32 # of field views -----
 ------ 1 field view (15 bytes) ----
   2 char field ("Ex", "Ey", "Ez", "Hx", "Hy", "Hz")
   1 char axis: plane normal 'x' (yz plane), 'y' (xz plane), 'z' (xy plane), or 'p' (point)
   3 int32 position: (x, y, z) for a point; (index, 0, 0) for a plane
 ------------- [...] ---------------
 ----------- data start ------------
 for each time step, for each field view in order (float32):
   point: 1 value
   xz: size_x*size_z values, x outer, z inner
   yz: size_y*size_z values, y outer, z inner
   xy: size_x*size_y values, x outer, y inner

 Format 1 (Lucifer <= 0.4.2) used 2 byte sizes and # of time steps, which
 overflow above 65535, and had no cell size or dt.
*/

static void write_i32(FILE *fp, int v) {
	int32_t w = (int32_t)v;
	fwrite(&w, 4, 1, fp);
}

static void write_f32(FILE *fp, float v) {
	fwrite(&v, 4, 1, fp);
}

int write_header(FILE *fp, LUC_sim_pars *simp) {

	static const char *field_names[6] = {"Ex", "Ey", "Ez", "Hx", "Hy", "Hz"};
	static const char view_axis[4] = {'p', 'y', 'x', 'z'}; // point, xz (at y), yz (at x), xy (at z)
	char header_text[128];
	char ch;
	int cnt, p;

	memset(header_text,0,128);
	snprintf(header_text, sizeof(header_text), "%s\n", LUC_VER);
	fwrite(header_text, 128, 1, fp);

	write_i32(fp, LUC_DATA_FORMAT);
	write_i32(fp, (*simp).size_x);
	write_i32(fp, (*simp).size_y);
	write_i32(fp, (*simp).size_z);
	write_i32(fp, (*simp).max_time);
	write_f32(fp, (*simp).courant);
	write_f32(fp, (*simp).cell_size);
	write_f32(fp, (*simp).dt);
	write_i32(fp, (*simp).n_field_views);

	for(cnt = 0; cnt < (*simp).n_field_views; cnt++) {
		const LUC_field_view *fv = &((*simp).field_views[cnt]);
		fwrite(field_names[(*fv).field], 2, 1, fp);
		ch = view_axis[(*fv).type];
		fwrite(&ch, 1, 1, fp);
		for(p = 0; p < 3; p++)
			write_i32(fp, (*fv).type == 0 || p == 0 ? (*fv).pos[p] : 0);
	}

	return 1;
}

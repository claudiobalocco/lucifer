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

#ifndef LUCIFER_GLOBAL_DEF_H
#define LUCIFER_GLOBAL_DEF_H

#define LUC_VER "Lucifer 0.4.2"

#define ETA0 376.73031 // vacuum impedance (ohm)
#define C0 299792458   // speed of light in vacuum (m/s)
#define MAX_NUM_OF_MATERIALS 1024
#define MAX_LEN_FILENAME 1024

#define C_RED     "\x1b[31;1m"
#define C_GREEN   "\x1b[32;1m"
#define C_YELLOW  "\x1b[33;1m"
#define C_BLUE    "\x1b[34;1m"
#define C_MAGENTA "\x1b[35;1m"
#define C_CYAN    "\x1b[36;1m"
#define C_RES     "\x1b[0m"

#include <Python.h>
#include <stdio.h> // for FILE type

typedef struct {
	int field;         // 0:Ex, 1:Ey, 2:Ez, 3:Hx, 4:Hy, 5:Hz
	int type;          // 0:point, 1:xz, 2:yz, 3:xy
	int pos[3];        // 3 int values (x, y, z) for point type, 1 int at [0] for plane type
} LUC_field_view;

typedef struct {
	// simulation domain size (in # of cells)
	int size_x;
	int size_y;
	int size_z;

	float courant;    // Courant number

	float cell_size;  // in m - assume cubic cells
	float sim_time;   // in s
	float dt;         // time step in s
	int max_time;     // # of time steps
	int t_step;       // current time step

	int n_sources;    // # of sources
	int n_field_views;// # of field views to save

	// boundary conditions
	int abc_k, abc_f, abc_l, abc_r, abc_b, abc_t;  // (0||1) for abc type
	int pbc_k, pbc_f, pbc_l, pbc_r, pbc_b, pbc_t;  // (0||1) for pbc type

	// file names
	const char *data_file;
	const char *geom_file;
	const char **src_files;

	// field views to save
	LUC_field_view *field_views; // array of field views

	// python dictionary from conf script
	PyObject *py_simp_dict;

} LUC_sim_pars;

typedef struct {
	float *Hx;
	float *Hy;
	float *Hz;
	float *Ex;
	float *Ey;
	float *Ez;
} LUC_fields;

typedef struct {	
	float *Cexe; // update coefficients - length equal to MAX_NUM_OF_MATERIALS - 0 is always vacuum
	float *Cexh;
	float *Ceye;
	float *Ceyh;
	float *Ceze;
	float *Cezh;

	PyObject *py_Cexe;
	PyObject *py_Cexh;
	PyObject *py_Ceye;
	PyObject *py_Ceyh;
	PyObject *py_Ceze;
	PyObject *py_Cezh;	

	unsigned short *material; // size_x * size_y * size_z - material[x,y,z] is the material index at (x,y,z)
	PyObject *py_material;
} LUC_diel_mat;

typedef struct {
	// abc boundary
	// (sizey,sizez)
	float *Eyx0, *Ezx0, *Eyx1, *Ezx1;
	// (sizex,sizez)
	float *Exy0, *Ezy0, *Exy1, *Ezy1;
	// (sizex,sizey)
	float *Exz0, *Eyz0, *Exz1, *Eyz1;
} LUC_abc_fields;

typedef struct {
	int field;         // 0:Ex, 1:Ey, 2:Ez, 3:Hx, 4:Hy, 5:Hz
	int type;          // 0:point, 1:plane
	int direction;     // for plane type only 0:x, 1:y, 2:z
	int location[3];   // 3 int values (x, y, z) for point type, 1 in value for plane type
	float *sp_profile; // beam profile for plane only
	float *t_profile;  // intensity in time domain
	PyObject *py_sp_profile;
	PyObject *py_t_profile;
} LUC_source;

LUC_sim_pars * par_config(const char *luc_name); // par_config.c
LUC_fields * allocate_fields(LUC_sim_pars *simp); // allocate_all.c
LUC_diel_mat * allocate_diel(LUC_sim_pars *simp); // allocate_all.c
LUC_abc_fields * allocate_abc(LUC_sim_pars *simp); // allocate_all.c
int build_geometry(const char *geom_file, LUC_sim_pars *simp, LUC_diel_mat *diel); // build_geometry.c
int write_header(FILE *fp,LUC_sim_pars *simp); // write_header.c
int updateE(LUC_fields *fields, LUC_diel_mat *diel, LUC_sim_pars *simp); // updateE.c
int updateH(LUC_fields *fields, LUC_diel_mat *diel, LUC_sim_pars *simp); // updateH.c
int source_init(const char *source_file, LUC_source **src, LUC_sim_pars *simp); // add_source.c
int add_source(LUC_source *src, LUC_fields *fields, LUC_sim_pars *simp); // add_source.c
int abc(LUC_fields *fields, LUC_abc_fields *abc_fields, LUC_sim_pars *simp); // abc.c
int write_field_views(FILE *fp, LUC_fields *fields, LUC_sim_pars *simp); // write_field_views.c
#endif




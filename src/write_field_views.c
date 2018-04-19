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

int write_field_views(FILE *fp, LUC_fields *fields, LUC_sim_pars *simp) {
	
	int cnt, i, j, k;
	
	for(cnt = 0; cnt < (*simp).n_field_views; cnt++) {
		
		switch( ((*simp).field_views[cnt]).type) {
			
			case 0: // this is a point
				i = ((*simp).field_views[cnt]).pos[0];
				j = ((*simp).field_views[cnt]).pos[1];
				k = ((*simp).field_views[cnt]).pos[2];

				switch(((*simp).field_views[cnt]).field) {
					case 0: // Ex
						fwrite(&((*fields).Ex[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 1: // Ey
						fwrite(&((*fields).Ey[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 2: // Ez
						fwrite(&((*fields).Ez[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 3: // Hx
						fwrite(&((*fields).Hx[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 4: // Hy
						fwrite(&((*fields).Hy[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 5: // Hz
						fwrite(&((*fields).Hz[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					default:
						return 0;
						break;
				}
				break;

			case 1: //xz
				j = ((*simp).field_views[cnt]).pos[0];
				switch(((*simp).field_views[cnt]).field) {
					case 0:
						for (i=0;i<(*simp).size_x;i++)
							for(k=0;k<(*simp).size_z;k++)
								fwrite(&((*fields).Ex[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 1:
						for (i=0;i<(*simp).size_x;i++)
							for(k=0;k<(*simp).size_z;k++)
								fwrite(&((*fields).Ey[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 2:
						for (i=0;i<(*simp).size_x;i++)
							for(k=0;k<(*simp).size_z;k++)
								fwrite(&((*fields).Ez[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 3:
						for (i=0;i<(*simp).size_x;i++)
							for(k=0;k<(*simp).size_z;k++)
								fwrite(&((*fields).Hx[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 4:
						for (i=0;i<(*simp).size_x;i++)
							for(k=0;k<(*simp).size_z;k++)
								fwrite(&((*fields).Hy[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 5:
						for (i=0;i<(*simp).size_x;i++)
							for(k=0;k<(*simp).size_z;k++)
								fwrite(&((*fields).Hz[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					default:
						return 0;
						break;
				}
				break;

			case 2: //yz
				printf("yz\n");
				i = ((*simp).field_views[cnt]).pos[0];
				switch(((*simp).field_views[cnt]).field) {
					case 0:
						for (j=0;j<(*simp).size_y;j++)
							for(k=0;k<(*simp).size_z;k++)
								fwrite(&((*fields).Ex[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 1:
						for (j=0;j<(*simp).size_y;j++)
							for(k=0;k<(*simp).size_z;k++)
								fwrite(&((*fields).Ey[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 2:
						for (j=0;j<(*simp).size_y;j++)
							for(k=0;k<(*simp).size_z;k++)
								fwrite(&((*fields).Ez[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 3:
						for (j=0;j<(*simp).size_y;j++)
							for(k=0;k<(*simp).size_z;k++)
								fwrite(&((*fields).Hx[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 4:
						for (j=0;j<(*simp).size_y;j++)
							for(k=0;k<(*simp).size_z;k++)
								fwrite(&((*fields).Hy[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 5:
						for (j=0;j<(*simp).size_y;j++)
							for(k=0;k<(*simp).size_z;k++)
								fwrite(&((*fields).Hz[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;

					default:
						return 0;
						break;				
				}
				break;
				
			case 3: //xy
				k = ((*simp).field_views[cnt]).pos[0];
				switch(((*simp).field_views[cnt]).field) {
					case 0:
						for (i=0;i<(*simp).size_x;i++)
							for(j=0;j<(*simp).size_y;j++)
								fwrite(&((*fields).Ex[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 1:
						for (i=0;i<(*simp).size_x;i++)
							for(j=0;j<(*simp).size_y;j++)
								fwrite(&((*fields).Ey[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 2:
						for (i=0;i<(*simp).size_x;i++)
							for(j=0;j<(*simp).size_y;j++)
								fwrite(&((*fields).Ez[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 3:
						for (i=0;i<(*simp).size_x;i++)
							for(j=0;j<(*simp).size_y;j++)
								fwrite(&((*fields).Hx[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 4:
						for (i=0;i<(*simp).size_x;i++)
							for(j=0;j<(*simp).size_y;j++)
								fwrite(&((*fields).Hy[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;
					case 5:
						for (i=0;i<(*simp).size_x;i++)
							for(j=0;j<(*simp).size_y;j++)
								fwrite(&((*fields).Hz[i*(*simp).size_y*(*simp).size_z+j*(*simp).size_z+k]), 4, 1, fp);
						break;

					default:
						return 0;
						break;
				}
			default:
				return 0;
				break;			
		} // switch
	} // for
	return 1;
} // function


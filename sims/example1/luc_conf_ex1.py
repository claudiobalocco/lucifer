# ------------------------------------
# ------ LUCIFER CONFIGURATION -------
# ------------------------------------

import os

# folder of this file: the data file is written next to it, on Linux and Windows
_here = os.path.dirname(os.path.abspath(__file__))

def get_pars():
	pars = {
		# simulation domain in number of cells
		'size_x': 200,
		'size_y': 200,
		'size_z': 200,
		#
		# edge dimension of the cell in m - assume cubic cell
		'cell_size': 5e-6,
		#
		# simulation time in s
		'sim_time': 5e-12,
		#
		# Courant number (must be <= 1/sqrt(3) = 0.577 in 3D)
		'courant': 0.5,
		#
		# boundaries
		# pbc: periodic; abc: absorbing
		# pbc must be set on both faces of an axis (front/back = x, left/right = y,
		# top/bottom = z); the period is size-1 cells
		'front'  : 'pbc',
		'back'   : 'pbc',
		'left'   : 'pbc',
		'right'  : 'pbc',
		'top'    : 'abc',
		'bottom' : 'abc',
		#
		# geometry and material file
		'geom_file': 'luc_geometry_ex1', # do not use .py extension
		#
		# source files
		#'sources': ('source', 'source2'), # do not use .py extension
		'sources': ('luc_source_ex1', ), # do not use .py extension
		#
		# fields to be saved in data_file
		'field_views': (
						{'field':'Ex', 'type':'xz', 'pos':(100,)},
						#{'field':'Ex', 'type':'point', 'pos':(50,50,50)}
						),
		#
		# data file
		'data_file': os.path.join(_here, 'luc_data_ex1.dat')
	}
	return pars

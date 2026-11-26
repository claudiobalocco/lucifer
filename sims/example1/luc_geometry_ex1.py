# ------------------------------------
# -------- LUCIFER GEOMETRY ----------
# ------------------------------------

import numpy

def _span(lo, hi, n):
	# integer range [lo, hi] (both included) clipped to the array size n
	return range(max(lo, 0), min(hi, n-1)+1)

class mat_builder:
	# all shapes write into self.m in place (no copy) and are clipped
	# to the simulation domain, so shapes can cross the boundaries
	def __init__(self, np_array):
		self.m = np_array
		self.sx = self.m.shape[0]
		self.sy = self.m.shape[1]
		self.sz = self.m.shape[2]
		self.m[:, :, :] = 0 # all is vacuum... (in place, no copy)

	def cell(self, pos, midx):
		i, j, k = pos
		self.m[i,j,k] = midx

	def box(self, pmin, pmax, midx):
		# cells xmin <= x < xmax, ymin <= y < ymax, zmin <= z < zmax
		xmin, ymin, zmin = pmin
		xmax, ymax, zmax = pmax
		self.m[max(xmin,0):xmax,max(ymin,0):ymax,max(zmin,0):zmax] = midx

	def cylinder(self, ax, center, rad, hc, midx):
		# circle of radius rad around (xc, yc) in the two axes normal to ax,
		# height hc cells along ax centred on zc; the tuple is (normal 1, normal 2, axis)
		# ax='z': m[i,j,k]   ax='y': m[i,k,j]   ax='x': m[k,i,j]
		xc, yc, zc = center
		n_i = {'z': self.sx, 'y': self.sx, 'x': self.sy}[ax]
		n_j = {'z': self.sy, 'y': self.sz, 'x': self.sz}[ax]
		n_k = {'z': self.sz, 'y': self.sy, 'x': self.sx}[ax]
		for i in _span(xc-rad, xc+rad, n_i):
			for j in _span(yc-rad, yc+rad, n_j):
				if (((i-xc)**2+(j-yc)**2) <= rad**2):
					for k in _span(zc-hc//2, zc-hc//2+hc-1, n_k):
						if (ax == 'z'):
								self.m[i,j,k] = midx
						if (ax == 'y'):
								self.m[i,k,j] = midx
						if (ax == 'x'):
								self.m[k,i,j] = midx

	def sphere(self, center, rad, midx):
		xc, yc, zc = center
		for i in _span(xc-rad, xc+rad, self.sx):
			for j in _span(yc-rad, yc+rad, self.sy):
				for k in _span(zc-rad, zc+rad, self.sz):
					if (((i-xc)**2+(j-yc)**2+(k-zc)**2) <= rad**2):
						self.m[i,j,k] = midx

	def ellipsoid(self, center, radx, rady, radz, midx):
		xc, yc, zc = center
		for i in _span(xc-radx, xc+radx, self.sx):
			for j in _span(yc-rady, yc+rady, self.sy):
				for k in _span(zc-radz, zc+radz, self.sz):
					if (((i-xc)**2/radx**2+(j-yc)**2/rady**2+(k-zc)**2/radz**2) <= 1):
						self.m[i,j,k] = midx

	def img2plane(self, img_name, ax, p, t, midx):
		# image on the plane normal to ax, from index p with a thickness of t cells
		# white pixels (value 255 after conversion to grey) get material midx
		# image rows/columns: ax='x': (y, z)  ax='y': (x, z)  ax='z': (x, y)
		# the image is clipped (or used partially) if its size differs from the plane
		from PIL import Image # only needed here
		np_img = numpy.array(Image.open(img_name).convert('L'))
		n1, n2 = {'x': (self.sy, self.sz), 'y': (self.sx, self.sz), 'z': (self.sx, self.sy)}[ax]
		if np_img.shape != (n1, n2):
			print("img2plane: image is {}x{}, plane is {}x{}: using the overlap".format(np_img.shape[0], np_img.shape[1], n1, n2))
		r1 = min(n1, np_img.shape[0])
		r2 = min(n2, np_img.shape[1])
		mask = (np_img[:r1, :r2] == 255)
		lo = max(p, 0)
		hi = p + t
		# basic slicing and transpose give views: the assignment writes into self.m
		if (ax == 'x'):
			v = self.m[lo:hi, :r1, :r2]
		if (ax == 'y'):
			v = self.m[:r1, lo:hi, :r2].transpose(1, 0, 2)
		if (ax == 'z'):
			v = self.m[:r1, :r2, lo:hi].transpose(2, 0, 1)
		v[:, mask] = midx


def build_geometry(m_buf,Cexe_buf,Cexh_buf,Ceye_buf,Ceyh_buf,Ceze_buf,Cezh_buf,simp):

	ETA0 = simp['ETA0']
	courant = simp['courant']

	# the buffers are memoryviews on the C arrays: numpy.frombuffer
	# and reshape create views, no data is copied
	dt_mat = numpy.dtype(numpy.uint16) # unsigned short in C
	dt_mat = dt_mat.newbyteorder('<')
	mat = numpy.frombuffer(m_buf,dtype=dt_mat)
	mat = mat.reshape((simp['size_x'],simp['size_y'],simp['size_z']))

	dt_C = numpy.dtype(numpy.float32)
	dt_C = dt_C.newbyteorder('<')
	Cexe = numpy.frombuffer(Cexe_buf,dtype=dt_C)
	Cexh = numpy.frombuffer(Cexh_buf,dtype=dt_C)
	Ceye = numpy.frombuffer(Ceye_buf,dtype=dt_C)
	Ceyh = numpy.frombuffer(Ceyh_buf,dtype=dt_C)
	Ceze = numpy.frombuffer(Ceze_buf,dtype=dt_C)
	Cezh = numpy.frombuffer(Cezh_buf,dtype=dt_C)

	########### START EDITING MATERIALS HERE ###########
	#######################
	# material dictionary (indices 0..1023, 0 is vacuum)
	# (name, relative_dielectric_constant)
	# use None as dielectric constant for a perfect electric conductor (E = 0)
	mat_dict = {
		0:('vacuum',1),
		1:('silicon',11.70),
		2:('metal',None)
	}

	for mat_idx, v in mat_dict.items():
		print("{} is {}".format(v[0], mat_idx))
		if v[1] is None: # perfect electric conductor
			ce, ch = 0, 0
		else:
			ce, ch = 1, (courant*ETA0)/v[1]
		Cexe[mat_idx] = ce
		Cexh[mat_idx] = ch
		Ceye[mat_idx] = ce
		Ceyh[mat_idx] = ch
		Ceze[mat_idx] = ce
		Cezh[mat_idx] = ch

	############ START EDITING GEOMETRY HERE ############
	mbuild = mat_builder(mat) #leave it here!
	#
	# mbuild.m is the matrix with the marials in
	# indeces from 0 to size-1
	#
	# mbuild.box((xmin,ymin,zmin),(xmax,ymax,zmax),material_number)
	#
	# EXAMPLES
	#mbuild.cylinder('z',(80,50,50),10,60,1)
	mbuild.sphere((100,100,100),30,2)
	#mbuild.ellipsoid((50,50,66),6,10,20,1)
	#mbuild.img2plane('tv_rcv_small.png','z',10,1,2)
	#mbuild.box((1,1,11),(100,100,50),1)

	return 1

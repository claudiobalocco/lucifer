# ------------------------------------
# -------- LUCIFER GEOMETRY ----------
# ------------------------------------

import numpy
from PIL import Image

class mat_builder:
	def __init__(self, np_array):
		self.m = np_array
		self.sx = self.m.shape[0]
		self.sy = self.m.shape[1]
		self.sz = self.m.shape[2]
		self.m[0:(self.sx-1),0:(self.sy-1),0:(self.sz-1)] = 0 # all is vacuum...

	def cell(self,(i,j,k),v):
		self.m[i,j,k] = midx

	def box(self,(xmin,ymin,zmin),(xmax,ymax,zmax),midx):
		self.m[xmin:xmax,ymin:ymax,zmin:zmax] = midx

	def cylinder(self,ax,(xc,yc,zc),rad,hc,midx):
		for i in xrange(xc-rad, xc+rad):
			for j in xrange(yc-rad, yc+rad):
				if (((i-xc)**2+(j-yc)**2) <= rad**2):
					for k in xrange((zc-hc/2), (zc-hc/2+hc)):
						if (ax == 'z'):
								self.m[i,j,k] = midx
						if (ax == 'y'):
								self.m[i,k,j] = midx
						if (ax == 'x'):
								self.m[k,i,j] = midx

	def sphere(self,(xc,yc,zc),rad,midx):
		for i in xrange(xc-rad, xc+rad):
			for j in xrange(yc-rad, yc+rad):
				for k in xrange(zc-rad, zc+rad):
					if (((i-xc)**2+(j-yc)**2+(k-zc)**2) <= rad**2):
						self.m[i,j,k] = midx

	def ellipsoid(self,(xc,yc,zc),radx,rady,radz,midx):
		for i in xrange(xc-radx, xc+radx):
			for j in xrange(yc-rady, yc+rady):
				for k in xrange(zc-radz, zc+radz):
					#find equation for ellipsoid
					if (((i-xc)**2/radx**2+(j-yc)**2/rady**2+(k-zc)**2/radz**2) <= 1):
						self.m[i,j,k] = midx

	def img2plane(self,img_name,ax,p,t,midx): #TO BE FIXED
		# Convert Image to array
		img = Image.open(img_name).convert('L')
		np_img = numpy.array(img)
		if (ax == 'x'):
			for j in xrange(0,self.sy-1):
				for k in xrange(0,self.sz-1):
					if (np_img[j,k] == 255):
						self.m[p:(p+t-1),j,k] = midx
		if (ax == 'y'):
			for i in xrange(0,self.sx-1):
				for k in xrange(0,self.sz-1):
					if (np_img[i,k] == 255):
						self.m[i,p:(p+t-1),k] = midx
		if (ax == 'z'):
			for i in xrange(0,self.sx-1):
				for j in xrange(0,self.sy-1):
					if (np_img[i,j] == 255):
						self.m[i,j,p] = midx
						#print self.m[i,j,p]


def build_geometry(m_buf,Cexe_buf,Cexh_buf,Ceye_buf,Ceyh_buf,Ceze_buf,Cezh_buf,simp):

	ETA0 = simp['ETA0']
	courant = simp['courant']

	dt_mat = numpy.dtype(numpy.int16)
	dt_mat = dt_mat.newbyteorder('<')
	mat = numpy.frombuffer(m_buf,dtype=dt_mat)
        ############ PUT SIZE HERE ########################
	mat = mat.reshape((200,200,200))
	
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
	# material dictionary 
	# (name, relative_dielectric_constant)
	mat_dict = {
		0:('vacuum',1),
		1:('silicon',11.70),
		2:('metal',-1000000)
	}

	for mat_idx, v in mat_dict.iteritems():
		print "{} is {}".format(v[0], mat_idx)
		Cexe[mat_idx] = 1
		Cexh[mat_idx] = (courant*ETA0)/v[1]
		Ceye[mat_idx] = 1
		Ceyh[mat_idx] = (courant*ETA0)/v[1]
		Ceze[mat_idx] = 1
		Cezh[mat_idx] = (courant*ETA0)/v[1]

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



























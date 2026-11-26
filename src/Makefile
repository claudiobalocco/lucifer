# Start of the makefile
# Linux (and MSYS2/MinGW on Windows): make
# Windows with Visual C++: use build_windows.bat instead
#
# Variables that can be overridden on the command line, e.g.
#   make PYTHON_CONFIG=python3.12-config
#   make ARCH=-march=x86-64-v3      (portable build for a cluster, instead of native)
#   make OPENMP=                    (build without OpenMP)

# Defining variables
srcs = lucifer.c allocate_all.c par_config.c updateE.c updateH.c build_geometry.c write_header.c add_source.c abc.c pbc.c write_field_views.c
objects = $(srcs:.c=.o)

ccomp = gcc

# -march=native: use all the vector instructions of this CPU (AVX2/AVX-512).
# The binary may not run on older CPUs: override ARCH to build for another machine.
ARCH ?= -march=native
OPENMP ?= -fopenmp

# Python 3 flags come from python3-config
# --embed is needed for Python >= 3.8 to link libpython into an executable
PYTHON_CONFIG ?= python3-config
py_cflags = $(shell $(PYTHON_CONFIG) --includes)
py_ldflags = $(shell $(PYTHON_CONFIG) --ldflags --embed 2>/dev/null || $(PYTHON_CONFIG) --ldflags)

cflags = -Wall -O3 -fstrict-aliasing $(ARCH) $(OPENMP) $(py_cflags)

ldflags = -O3 $(OPENMP) $(py_ldflags) -lm

target = ../bin/lucifer

all: $(objects)
	mkdir -p $(dir $(target))
	$(ccomp) -o $(target) $(objects) $(ldflags)

%.o: %.c lucifer_global_def.h
	$(ccomp) -c -o $@ $< $(cflags)

# Cleaning everything
clean:
	rm -f $(objects)
# End of the makefile

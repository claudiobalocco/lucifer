# Start of the makefile
# Defining variables
srcs = lucifer.c allocate_all.c par_config.c updateE.c updateH.c build_geometry.c write_header.c add_source.c abc.c write_field_views.c
objects = $(srcs:.c=.o)

ccomp = gcc

cflags = -Wall -O3 -fstrict-aliasing -fopenmp -I/usr/include/python2.7
#cflags = -Wall -O3 -fstrict-aliasing -I/usr/include/python2.7

ldflags = -O3 -fopenmp -lm -L/usr/lib/python2.7/config-x86_64-linux-gnu -L/usr/lib -lpython2.7
#ldflags = -O3 -lm -L/usr/lib/python2.7/config-x86_64-linux-gnu -L/usr/lib -lpython2.7

target = ../bin/lucifer

all: $(objects)
	$(ccomp) -o $(target) $(objects) $(ldflags)

%.o: %.c
	$(ccomp) -c -o $@ $< $(cflags)

# Cleaning everything
clean:
	rm -f $(objects) $(mod_objects)
# End of the makefile

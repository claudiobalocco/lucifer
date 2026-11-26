Lucifer - a 3D FDTD simulator
Copyright (C) 2015-2018 Claudio Balocco

Version 1.0.0

Requirements
  Python >= 3.8 (64 bit) with numpy. Pillow is needed only for img2plane.
  Linux:   gcc with OpenMP and the Python development files (e.g. python3-dev).
  Windows: Visual Studio 2019 16.10 or newer with "Desktop development with C++".

Build
  Linux:   cd src && make
           (bin/lucifer; -march=native is used, build with
            make ARCH=-march=x86-64-v3 for a binary that runs on other machines)
  Windows: src\build_windows.bat [path\to\python.exe]
           (bin\lucifer.exe; it uses the Python it was built with, or PYTHONHOME)

Run
  bin/lucifer -t <threads> -c sims/example1/luc_conf_ex1.py
  Options: -t # of threads, -q quiet (no questions), -v version, -h help.
  On machines with several CPU sockets also set, before running:
    export OMP_PROC_BIND=close OMP_PLACES=cores         (Linux)
    set OMP_PROC_BIND=close & set OMP_PLACES=cores      (Windows)

Boundaries
  abc: first order Mur absorbing boundary.
  pbc: periodic. It must be set on both faces of an axis (front/back = x,
  left/right = y, top/bottom = z). The period is size-1 cells: the last plane
  (index size-1) is a copy of plane 0.

Materials
  Up to 1024 materials (index 0 is vacuum). In the geometry script a dielectric
  constant of None gives a perfect electric conductor (all coefficients 0).

Data file
  Format 2: header with int32 sizes, # of time steps, courant, cell size, dt
  and the list of field views, then the views of each time step (float32).
  The layout is described in src/write_header.c. The readers in matlab/ and
  octave/ read format 2 and also the format 1 files of Lucifer <= 0.4.2.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3 as 
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

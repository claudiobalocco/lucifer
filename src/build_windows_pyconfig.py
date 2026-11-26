# Helper for build_windows.bat: writes the Python 3 paths needed to build and
# run lucifer.exe with Microsoft Visual C++.
#   <out_dir>\pyconfig.bat   SET lines read by build_windows.bat
#   <out_dir>\luc_pyhome.h   #define LUC_PYTHONHOME, so lucifer.exe finds the
#                            Python standard library without PYTHONHOME
import json
import os
import sys
import sysconfig

out_dir = sys.argv[1]
base = sys.base_prefix                      # the real installation, also inside a venv
ver = "{}{}".format(*sys.version_info[:2])  # e.g. 312
if sys.version_info < (3, 8):
	sys.exit("Python >= 3.8 is required")

try:
	import numpy  # noqa: F401  (needed by the geometry and source scripts)
except ImportError:
	print("[WARNING] numpy is not installed for " + sys.executable)

with open(os.path.join(out_dir, "pyconfig.bat"), "w") as f:
	f.write('set "PY_INC={}"\n'.format(sysconfig.get_paths()["include"]))
	f.write('set "PY_LIBDIR={}"\n'.format(os.path.join(base, "libs")))
	f.write('set "PY_LIB=python{}.lib"\n'.format(ver))
	f.write('set "PY_DLLNAME=python{}.dll"\n'.format(ver))

with open(os.path.join(out_dir, "luc_pyhome.h"), "w") as f:
	# json.dumps gives a valid C string literal for a path (escapes \ and ")
	f.write("#define LUC_PYTHONHOME {}\n".format(json.dumps(base)))

print("Python {} at {}".format(sys.version.split()[0], base))

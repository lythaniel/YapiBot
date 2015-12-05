import pybindgen
import sys
import os

mod = pybindgen.Module('YapiBot')

mod.add_include('"../src/PythonApi.h"')
PythonBinding = mod.add_class('CPythonApi')
PythonBinding.add_constructor([])
PythonBinding.add_method('compassCalibration', pybindgen.retval ('int'), [])
PythonBinding.add_method('moveStraight', pybindgen.retval ('int'), [pybindgen.param ('int', 'distance')])
PythonBinding.add_method('alignBearing', pybindgen.retval ('int'), [pybindgen.param ('int', 'bearing')])
PythonBinding.add_method('moveBearing', pybindgen.retval ('int'), [pybindgen.param ('int', 'bearing'),pybindgen.param ('int', 'distance')])
PythonBinding.add_method('rotate', pybindgen.retval ('int'), [pybindgen.param ('int', 'rot')])


module_fname = os.path.join("../src","PythonBinding.cpp")

with open(module_fname,"w+t")as file_:
	print("Generating file {}".format(module_fname))
	mod.generate(file_)



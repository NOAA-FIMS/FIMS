#!/usr/bin/env python3

from pydefs.do_call import do_call

import os
import platform
import subprocess
import sys

do_call([sys.executable, 'clean.py'])

if platform.system() == 'Windows':
  do_call(['where', 'cmake'], fix_message='Install cmake, add cmake to PATH')
  do_call(['where', 'R'], fix_message='Install R, add R.exe to PATH')
  do_call(['where', 'sh'], fix_message='Install Rtools, add sh.exe to PATH')
  do_call(['where', 'g++'], fix_message='Install Rtools, add g++.exe to PATH')
else:
  do_call(['which', 'cmake'])

local_library = os.path.join(os.getcwd(), '_library')
os.mkdir(local_library)

packname = 'FIMS_0.0.0.9000.tar.gz'

logfiles = ['00install.out']
logfiles = [os.path.join('FIMS.Rcheck', x) for x in logfiles]

do_call(['R', 'CMD', 'build', '.'])
do_call(['R', 'CMD', 'check', packname], info_files=logfiles)
do_call(['R', 'CMD', 'INSTALL', '-l', local_library, packname])

result_out = os.path.join(os.getcwd(), 'mytest.Rout')
os.environ['R_LIBS_USER'] = local_library




#!/usr/bin/env python3

import os
import subprocess
import sys

def do_call(args, fix_message='', info_files=[]):
  oneline = ' '.join(['"{}"'.format(x) for x in args])
  print("[{}]> {}".format(os.getcwd(), oneline))
  try:
    subprocess.check_call(args)
  except subprocess.CalledProcessError as error:
    print(error)
    print(error.output)
    if fix_message:
      print("FIX: {}".format(fix_message))
    if len(info_files) != 0:
      for info_file in reversed(info_files):
        if os.path.exists(info_file):
          print('Content of `{}`:'.format(info_file))
          print(open(info_file).read())
          break
    sys.exit(1)
  except OSError as error:
    print(error)
    sys.exit(1)

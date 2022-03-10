#!/usr/bin/env python3

import os
import shutil

def do_rm(obj):
  print("[{}]> REMOVE {}".format(os.getcwd(), obj))
  if not os.path.exists(obj):
    print('not exists')
    return
  if os.path.isdir(obj):
    shutil.rmtree(obj)
    return
  os.remove(obj)

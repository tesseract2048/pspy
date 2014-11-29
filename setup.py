#!/usr/bin/python
# -*- coding: utf-8 -*-

VERSION='0.1'

from distutils.core import setup, Extension

sources = ['src/hash_table.c', 'src/peer_storage.c', 'src/list.c', 'src/peer_storage_py.c']

setup(name='pspy',
      description='High performance peer storage',
      long_description=',',
      author='hty0807',
      author_email='hty0807@gmail.com',
      version=VERSION,
      url='https://github.com/tesseract2048/pspy',
      download_url='',
      license='MIT',
      keywords='',
      ext_modules=[Extension('pspy', sources)],
      classifiers=[],
      )


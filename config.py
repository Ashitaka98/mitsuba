import os, sys

BUILDDIR = '#build/release'
DISTDIR = '#dist'
CXX = 'g++'
CC = 'gcc'
CXXFLAGS = [
    '-std=gnu++11', '-O3', '-Wall', '-g', '-pipe', '-march=nocona', '-msse2',
    '-ftree-vectorize', '-mfpmath=sse', '-funsafe-math-optimizations',
    '-fno-rounding-math', '-fno-signaling-nans', '-fno-math-errno',
    '-fomit-frame-pointer', '-DMTS_DEBUG', '-DSINGLE_PRECISION',
    '-DSPECTRUM_SAMPLES=3', '-DMTS_SSE', '-DMTS_HAS_COHERENT_RT', '-fopenmp',
    '-fvisibility=hidden', '-mtls-dialect=gnu2'
]
LINKFLAGS = []
SHLINKFLAGS = ['-rdynamic', '-shared', '-fPIC', '-lstdc++']
BASEINCLUDE = [
    '#include', '/home/lzr/Tools/anaconda3/envs/mitsubaPy27/include'
]
BASELIB = ['dl', 'm', 'pthread', 'gomp']
EIGENINCLUDE = ['/usr/include/eigen3']
ORTINCLUDE = ['/home/lzr/Projects/onnxruntime-linux-x64-1.4.0/include']
ORTLIB = ['onnxruntime']
ORTLIBDIR = ['/home/lzr/Projects/onnxruntime-linux-x64-1.4.0/lib']
OEXRINCLUDE = ['/usr/include/OpenEXR']
OEXRLIB = ['Half', 'IlmImf', 'z']
PNGLIB = ['png']
JPEGLIB = ['jpeg']
XERCESINCLUDE = []
XERCESLIB = ['xerces-c']
XERCESLIBDIR = ['/home/lzr/Tools/anaconda3/envs/mitsubaPy27/lib']
GLLIB = ['GL', 'GLU', 'GLEWmx', 'Xxf86vm', 'X11']
GLFLAGS = ['-DGLEW_MX']
BOOSTLIB = ['boost_system', 'boost_filesystem', 'boost_thread']
COLLADAINCLUDE = [
    '/usr/include/collada-dom2.4', '/usr/include/collada-dom2.4/1.4'
]
COLLADALIB = ['collada-dom2.4-dp']
FFTWLIB = ['fftw3_threads', 'fftw3']

# The following runs a helper script to search for installed Python
# packages that have a Boost Python library of matching version.
# A Mitsuba binding library will be compiled for each such pair.
# Alternatively, you could also specify the paths and libraries manually
# using the variables PYTHON27INCLUDE, PYTHON27LIB, PYTHON27LIBDIR etc.

import sys, os

sys.path.append(os.path.abspath('../data/scons'))
from detect_python import detect_python

locals().update(detect_python())

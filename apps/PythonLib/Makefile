# To match the version of python
# $ ls -lht /usr/lib/libpython2.7.dylib
# lrwxr-xr-x  1 root  wheel    68B Sep 24 07:11 /usr/lib/libpython2.7.dylib -> ../../System/Library/Frameworks/Python.framework/Versions/2.7/Python
# which is python 2.7.10
# So we should use this library with /System/Library/Frameworks/Python.framework/Versions/2.7/bin/python
PROJ_ROOT=../..

CXX=g++
CXXFLAGS=-std=c++11 -Wall -O2 -ffast-math -shared -fPIC
INCLUDES=-I${PROJ_ROOT}/include -I${PROJ_ROOT}/src -I/usr/include/python2.7
LDFLAGS=-lboost_python -lpython2.7

mcemlda.so: pylib.cpp $(filter %.h %.hpp, $(PROJ_ROOT)/src)
	$(CXX) $< -o $@ $(CXXFLAGS) $(INCLUDES) $(LDFLAGS)

clean:
	-rm -f *.csv
	-rm -f mcemlda.so
	-rm -rf *.dSYM

.PHONY: clean


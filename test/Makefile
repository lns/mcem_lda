PROJ_ROOT=..

CXX=g++
CXXFLAGS=-std=c++11 -O2 -ffast-math -fPIC -m64 -Wall #-g #-DDEBUG
INCLUDES=-I${PROJ_ROOT}/include -I${PROJ_ROOT}/src
LDFLAGS=-lgtest -lssl -lcrypto -lboost_system -lboost_date_time
UNAME = $(shell uname -s)
ifeq ($(UNAME), Linux) # Linux
	INCLUDES += -I/home/users/wangqing09/.jumbo/include/openssl/
	LDFLAGS += -L/home/users/wangqing09/.jumbo/lib/ -lpthread -lrt -lz -lm
endif
ifeq ($(UNAME), Darwin) # Mac OS
	INCLUDES += -I/usr/local/opt/openssl/include/
	LDFLAGS += -L/usr/local/opt/openssl/lib/ -lz -lm
endif

all: test

test: main-test.cpp *.cpp $(filter %.h %.hpp, ${PROJ_ROOT}/src/*)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)

clean:
	$(RM) test
	$(RM) -r *.dSYM

.PHONY:
	clean


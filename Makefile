# CMake-driver Makefile version 4

CMAKE_DIR := _obj

default: build

ifdef WINDIR
# Running on Windows...
CMAKE_NAME := "MSYS Makefiles"
else
# Linux or OS X, presumably
CMAKE_NAME := "Unix Makefiles"
endif

help:
	@echo "make build -- invoke CMake, then make with ${CMAKE_NAME}, in objdir ${CMAKE_DIR}/"
	@echo "make install -- make build, then make install in objdir"

${CMAKE_DIR}:
	mkdir ${CMAKE_DIR}

${CMAKE_DIR}/Makefile: ${CMAKE_DIR}
	cd ${CMAKE_DIR} ; cmake .. -G ${CMAKE_NAME}

build: ${CMAKE_DIR}/Makefile
	cd ${CMAKE_DIR} ; make

install: build
	cd ${CMAKE_DIR} ; make install

clean:
	rm *~

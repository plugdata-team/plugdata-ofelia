# Attempt to load a config.make file.
# If none is found, project defaults in config.project.make will be used.

PLATFORM_CXXFLAGS = -std=c++17
	
ifneq ($(wildcard config.make),)
	include config.make
endif

# make sure the the OF_ROOT location is defined
ifndef OF_ROOT
    OF_ROOT=$(realpath ../../Libraries/openFrameworks)
endif

# PLATFORM_PKG_CONFIG="pkg-config --static"

# call the project makefile!
include $(OF_ROOT)/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk

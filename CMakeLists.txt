cmake_minimum_required(VERSION 3.15)

set (CMAKE_CXX_STANDARD 14)
project(plugdata-ofelia VERSION 0.8.0 LANGUAGES C CXX)


set(EXTERNAL_COMPILE_DEFINITIONS PD=1 USEAPI_DUMMY=1 PD_INTERNAL=1)
# COMPILE DEFINITIONS OS
# ------------------------------------------------------------------------------#
if(WIN32)
    list(APPEND EXTERNAL_COMPILE_DEFINITIONS WINVER=0x502 WIN32=1 _WIN32=1)

    if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
        list(APPEND EXTERNAL_COMPILE_DEFINITIONS "PD_LONGINTTYPE=long long")
    endif()

    if(MSVC)
        list(APPEND EXTERNAL_COMPILE_DEFINITIONS HAVE_STRUCT_TIMESPEC=1 _CRT_SECURE_NO_WARNINGS=1 HAVE_ALLOCA=0)
    endif()
elseif(UNIX)
    list(APPEND EXTERNAL_COMPILE_DEFINITIONS HAVE_LIBDL=1 HAVE_UNISTD_H=1)
endif()

file(GLOB ofelia_source ./Source/Objects/*.cpp ./Source/Objects/*.h)
add_library(ofelia STATIC ${ofelia_source})

if(MSVC)
add_custom_command(TARGET ofelia PRE_BUILD 
                    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/Resources/build.bat
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
else()
add_custom_command(TARGET ofelia PRE_BUILD 
                    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/Resources/build.sh
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
endif()

target_include_directories(ofelia PRIVATE ./Libraries/ofxPd/libs/libpd/pure-data/src)
target_compile_definitions(ofelia PRIVATE ${EXTERNAL_COMPILE_DEFINITIONS})

#TODO: 
# Create a cmake target for the externals
# Then, run the build scripts for openFrameworks
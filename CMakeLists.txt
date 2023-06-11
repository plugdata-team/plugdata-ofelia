cmake_minimum_required(VERSION 3.15)

set (CMAKE_CXX_STANDARD 14)
project(plugdata-ofelia VERSION 0.8.0 LANGUAGES C CXX)


# OFELIA REMOTE APPLICATION
# ------------------------------------------------------------------------------#

# Call build script
if(MSVC)
add_custom_target(ofelia ALL 
                    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/Resources/build.bat
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
else()
add_custom_target(ofelia ALL 
                    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/Resources/build.sh
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
endif()

# Convert build product into binary data
add_custom_command(
    TARGET ofelia POST_BUILD
    DEPENDS ofelia
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/Resources/scripts/create-binary-data.py ${CMAKE_CURRENT_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# Create an empty list to store the generated C++ source files
set(GENERATED_SOURCE_FILES)

# Add a custom command to generate each C++ source file
foreach(chunk_index RANGE 15)
    set(generated_cpp_file ${CMAKE_CURRENT_BINARY_DIR}/src/ofelia_binary_data_${chunk_index}.cpp)
    list(APPEND GENERATED_SOURCE_FILES ${generated_cpp_file})
    
    add_custom_command(
        OUTPUT ${generated_cpp_file}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/src/ofelia_binary_data_${chunk_index}.cpp ${generated_cpp_file}
        DEPENDS ofelia
        COMMENT "Copying generated C++ source file ${chunk_index}"
    )

endforeach()

# Create a static library target and include the generated C++ source files
add_library(ofelia_binary_data STATIC ${GENERATED_SOURCE_FILES})


# OFELIA EXTERNAL STATIC LIB
# ------------------------------------------------------------------------------#

set(EXTERNAL_COMPILE_DEFINITIONS PD=1 USEAPI_DUMMY=1 PD_INTERNAL=1)

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
add_library(ofelia-external STATIC ${ofelia_source})

target_include_directories(ofelia-external PRIVATE ./Libraries/ofxPd/libs/libpd/pure-data/src)
target_compile_definitions(ofelia-external PRIVATE ${EXTERNAL_COMPILE_DEFINITIONS})
add_dependencies(ofelia-external ofelia_binary_data)
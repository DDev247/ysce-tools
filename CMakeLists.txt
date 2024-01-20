# CMakeList.txt : CMake project for ysce-decompiler, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("ysce-tools")

# Add source to this project's executable.
add_executable (ysce-decompiler
	"src/ysce_decompiler.cpp"
	"src/yds_math.cpp"
	"include/ysce.h"
	"include/yds_math.h")

add_executable (dia-compiler
	"src/dia_compiler.cpp"
	"src/yds_math.cpp"
	"src/yds_interchange_object.cpp"
	"src/common.cpp"

	"include/ysce.h"
	"include/yds_math.h"
	"include/dia.h"
	"include/common.h")

	
add_executable (dia-decompiler
	"src/dia_decompiler.cpp"
	"src/yds_math.cpp"
	"src/yds_interchange_object.cpp"
	"src/common.cpp"

	"include/ysce.h"
	"include/yds_math.h"
	"include/dia.h"
	"include/common.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ysce-decompiler PROPERTY CXX_STANDARD 20)
  set_property(TARGET dia-compiler PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

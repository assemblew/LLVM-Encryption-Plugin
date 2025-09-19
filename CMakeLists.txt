cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "")
project(llvm-plugins)

#===============================================================================
# 1. LOAD Clang CONFIGURATION
#===============================================================================
# Set this to a valid LLVM installation dir
set(LLVM_PATH "" CACHE PATH "Clang installation directory")

# Add the location of ClangConfig.cmake to CMake search paths (so that
# find_package can locate it)
list(APPEND CMAKE_PREFIX_PATH "${LLVM_PATH}/lib/cmake/clang/")

find_package(Clang REQUIRED CONFIG)
if("${LLVM_VERSION_MAJOR}" VERSION_LESS 19)
  message(FATAL_ERROR "Found LLVM ${LLVM_VERSION_MAJOR}, but need LLVM 19 or above")
endif()

# Plugins include headers from LLVM and Clang - update the include paths
# accordingly
include_directories(SYSTEM "${LLVM_INCLUDE_DIRS}")

#===============================================================================
# 2. BUILD CONFIGURATION
#===============================================================================
# Use the same C++ standard as LLVM does
set(CMAKE_CXX_STANDARD 17 CACHE STRING "")

# write executables to bin
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# set the include directory
set(CMAKE_PLUGIN_INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include")

# LLVM is normally built without RTTI. Be consistent with that.
if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

# -fvisibility-inlines-hidden is set when building LLVM and on Darwin warnings
# are triggered if llvm-tutor is built without this flag (though otherwise it
# builds fine). For consistency, add it here too.
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-fvisibility-inlines-hidden"
  SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG)
if(${SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG} EQUAL "1")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
endif()

#===============================================================================
# 3. ADD THE TARGETS
#===============================================================================
add_subdirectory(lib)
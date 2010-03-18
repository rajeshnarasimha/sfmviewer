cmake_minimum_required(VERSION 2.6)
include("../buildtools/gt.cmake")
project(sfmviewer CXX C)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Turn off debugging checks in uBLAS
add_definitions(-DNDEBUG -O2 -Wall)

# Use packages
gt_use_boost()
gt_use_gtsam()

gt_use_qt4(QtOpenGL)
    
gt_main_lib("*.cpp")
gt_install_headers("*.h")

# Build exe's from each file matching this pattern
gt_auto_exes("exes/*.cpp")

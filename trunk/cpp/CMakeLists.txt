cmake_minimum_required(VERSION 2.6)
include("../buildtools/gt.cmake")
project(sfmviewer CXX C)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Turn off debugging checks in uBLAS
add_definitions(-DNDEBUG -O2 -Wall)

# find src files and header files
file(GLOB srcs RELATIVE "${PROJECT_SOURCE_DIR}" *.cpp)
file(GLOB headers RELATIVE "${PROJECT_SOURCE_DIR}" "*.h")

# QT
#gt_use_qt4(QtOpenGL)
find_package(Qt4 REQUIRED COMPONENTS QtCore QtGui QtOpenGL)
include(${QT_USE_FILE})
link_libraries(${QT_LIBRARIES})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
qt4_automoc(${srcs})

# link with other 3rd party libraries
find_package(Boost 1.40 REQUIRED)
find_package(GLUT)
find_package(OpenGL)
include_directories(${Boost_INCLUDE_DIRS})
link_libraries(${Boost_LIBRARIES} ${GLUT_LIBRARY} ${OPENGL_LIBRARY})

# build and install library
add_library(${PROJECT_NAME}-shared SHARED ${srcs})
SET_TARGET_PROPERTIES(${PROJECT_NAME}-shared PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")
SET_TARGET_PROPERTIES(${PROJECT_NAME}-shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)
install(TARGETS ${PROJECT_NAME}-shared LIBRARY DESTINATION "lib" ARCHIVE DESTINATION "lib")

# install header files
install(FILES ${headers} DESTINATION "include/${PROJECT_NAME}")


# gtsam related

if(false)

include_directories("$ENV{HOME}/borg/gtsam")
link_libraries("-L$ENV{HOME}/borg/gtsam/build/gtsam/.libs -lgtsam")

# kai01 exectuable
add_executable(kai01 exes/kai01.cpp)
add_custom_target(kai01.run kai01)
target_link_libraries(kai01 sfmviewer-shared)

# kai02 exectuable
add_executable(kai02 exes/kai02.cpp)
add_custom_target(kai02.run kai02)
target_link_libraries(kai02 sfmviewer-shared)

endif()

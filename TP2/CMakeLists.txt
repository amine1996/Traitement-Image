cmake_minimum_required (VERSION 2.8)
project (modeleProj)
add_definitions("-DPROJECT_DIRECTORY=\"${PROJECT_SOURCE_DIR}\"")
set (CMAKE_CXX_FLAGS_RELEASE " -O3 -DNDEBUG -ggdb -Wall -std=c++11 -Wextra")

set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# OpenCV
find_package(OpenCV REQUIRED)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE} ${OpenMP_CXX_FLAGS}")
endif()


include_directories (${OPENCV_INCLUDE_DIR})

include_directories (${CMAKE_CURRENT_BINARY_DIR})

include_directories (${CMAKE_CURRENT_SOURCE_DIR})


add_executable (hough
   hough.cpp
)

target_link_libraries (hough
   ${OpenCV_LIBS}
)

add_executable (fastHough
   fastHough.cpp
)

target_link_libraries (fastHough
   ${OpenCV_LIBS}
)

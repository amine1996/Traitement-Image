cmake_minimum_required (VERSION 2.8)
project (modeleProj)
add_definitions("-DPROJECT_DIRECTORY=\"${PROJECT_SOURCE_DIR}\"")
set (CMAKE_CXX_FLAGS_RELEASE " -O3 -DNDEBUG -Wall -Wextra")

set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# OpenCV
find_package(OpenCV REQUIRED)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()


include_directories (${OPENCV_INCLUDE_DIR})

include_directories (${CMAKE_CURRENT_BINARY_DIR})

include_directories (${CMAKE_CURRENT_SOURCE_DIR})


add_executable (seuilGris
  seuilGris.cpp
)


target_link_libraries (seuilGris	
  ${OpenCV_LIBS}
)

add_executable (sobel
  sobel.cpp
)


target_link_libraries (sobel	
  ${OpenCV_LIBS}
)

add_executable (evaluation
   evaluation.cpp
)

target_link_libraries (evaluation	
  ${OpenCV_LIBS}
)

add_executable (combinaison
   combinaison.cpp
)

target_link_libraries (combinaison
   ${OpenCV_LIBS}
)

add_executable (bestResult
   bestResult.cpp
)

target_link_libraries (bestResult
   ${OpenCV_LIBS}
)

project( flow-free-solver )

cmake_minimum_required(VERSION 2.6)

ADD_DEFINITIONS(
  -std=c++11
  -w
  -lpthread
  # -Ofast
  -g
  # -ljpeg
  )

find_package(OpenCV REQUIRED)

set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})

file(

  GLOB_RECURSE

  source_files

  src/*

  )

add_executable(

  flow-free-solver

  ${source_files}

  )

target_link_libraries(flow-free-solver ${OpenCV_LIBS} -lpthread -lX11)

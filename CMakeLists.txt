# Copyright (c) 2019 Michael de Gans
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

project(birbcam C)

cmake_minimum_required(VERSION 3.10.2)
set(CMAKE_C_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
#pkg_check_modules(PROTOBUF_C REQUIRED libprotobuf-c>=1.0.0)

include_directories(${GSTREAMER_INCLUDE_DIRS})
#include_directories(${PROTOBUF_C_INCLUDE_DIRS})

# project includes
include_directories(includes)
include_directories(/opt/nvidia/deepstream/deepstream-4.0/sources/includes)

# deepstream lib path
link_directories(/opt/nvidia/deepstream/deepstream-4.0/lib/)

file(GLOB SRC src/*)
add_executable(${PROJECT_NAME} main.c ${SRC})
#target_link_libraries(${PROJECT_NAME} ${GSTREAMER_LIBRARIES} ${PROTOBUF_C_LIBRARIES} nvds_meta nvdsgst_meta)
target_link_libraries(${PROJECT_NAME} ${GSTREAMER_LIBRARIES} nvds_meta nvdsgst_meta)

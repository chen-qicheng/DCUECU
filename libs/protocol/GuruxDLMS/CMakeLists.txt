add_definitions(-w -fno-exceptions -s -O3 -fPIC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++11")

SET(INC_LIST
./
../include)

# 自动查找所有.cpp文件
file(GLOB_RECURSE DLMS_LIB_SRC_LIST "*.cpp")

INCLUDE_DIRECTORIES(${INC_LIST})

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
add_library(GuruxDLMS SHARED ${DLMS_LIB_SRC_LIST})




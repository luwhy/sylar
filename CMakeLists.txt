cmake_minimum_required(VERSION 3.11)
project(sylar)

#显示make命令
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")
include_directories(/app/sylar/include)
link_directories(/app/sylar/lib)
find_library(YAMLCPP libyaml-cpp.a)



set(LIB_SRC sylar/log.cc sylar/util.cc sylar/config.cc)
add_library(sylar SHARED ${LIB_SRC})

add_executable(test test/test.cc)
add_dependencies(test sylar)
target_link_libraries(test sylar)
# target_include_directories(test PUBLIC ${PROJECT_SOURCE_DIR}/sylar)
add_executable(testConfig test/testConfig.cc)
add_dependencies(testConfig sylar)
target_link_libraries(testConfig sylar -L/apps/sylar/lib -lyaml-cpp) 


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
cmake_minimum_required(VERSION 3.11)
project(sylar)

include(cmake/utils.cmake)

#显示make命令
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
include_directories(/app/sylar/include)
link_directories(/app/sylar/lib)
find_library(YAMLCPP libyaml-cpp.a)

set(LIBS
    sylar
    pthread
)


set(LIB_SRC sylar/log.cc sylar/util.cc sylar/config.cc sylar/thread.cc sylar/fiber.cc sylar/scheduler.cc)
add_library(sylar SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(sylar)

add_executable(test test/test.cc)
add_dependencies(test sylar)
force_redefine_file_macro_for_sources(test)
target_link_libraries(test ${LIBS} -lyaml-cpp)

# target_include_directories(test PUBLIC ${PROJECT_SOURCE_DIR}/sylar)
add_executable(testConfig test/testConfig.cc)
add_dependencies(testConfig sylar)
force_redefine_file_macro_for_sources(testConfig)
target_link_libraries(testConfig ${LIBS} -lyaml-cpp) 

add_executable(testThread test/test_thread.cc)
add_dependencies(testThread sylar)
force_redefine_file_macro_for_sources(testThread)
target_link_libraries(testThread ${LIBS} -lyaml-cpp) 

add_executable(testUtil test/test_util.cc)
add_dependencies(testUtil sylar)
force_redefine_file_macro_for_sources(testUtil)
target_link_libraries(testUtil ${LIBS} -lyaml-cpp) 

add_executable(testFiber test/test_fiber.cc)
add_dependencies(testFiber sylar)
force_redefine_file_macro_for_sources(testFiber)
target_link_libraries(testFiber ${LIBS} -lyaml-cpp) 



SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
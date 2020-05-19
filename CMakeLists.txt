cmake_minimum_required(VERSION 3.12...3.16 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDER ON)

#Make sure that custom modules like FindRapidJSON are found
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

project(libmetaprops VERSION 0.0.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add_subdirectory(src)
file(GLOB_RECURSE LIB_SOURCES LIST_DIRECTORIES true src/*.h src/*.cpp)

add_library(metaprops
    ${LIB_SOURCES}
)

#Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(MetaProps::metaprops ALIAS metaprops)

#Set target properties
target_include_directories(metaprops
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(metaprops PRIVATE cxx_auto_type)
target_compile_options(metaprops PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Wall -Wextra -Wpedantic>)



#https://github.com/pabloariasal/modern-cmake-sample/blob/master/libjsonutils/CMakeLists.txt

##############################################
## Add test
enable_testing()
add_subdirectory(test)

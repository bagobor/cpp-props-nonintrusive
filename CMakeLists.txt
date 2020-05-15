cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(projectName CppNonIvasivePropsExperiment)
 
# We don't use the C++ compiler, so don't let project() # test for it in case the platform doesn't have one 
project(${projectName} VERSION 0.0.3 LANGUAGES CXX)

# Primary tool for this project
add_executable(test_app 
    main.cpp
    #debug.c # Optimized away for release builds 
)

# Helpful diagnostic tool for development and testing
#add_executable(TestTool testTool.c)
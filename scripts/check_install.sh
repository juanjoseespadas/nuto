#!/bin/bash
set -ev
mkdir -p testproject && cd testproject
printf "project(testproject)\ncmake_minimum_required(VERSION 3.5)\nset(CMAKE_CXX_STANDARD 14)\nfind_package(NuTo)\nadd_executable(printVersion printVersion.cpp)\ntarget_link_libraries(printVersion NuTo::Base)" > CMakeLists.txt
printf "#include <iostream>\n#include <nuto/base/Version.h>\nint main(){\nstd::cout << NuTo::Version << std::endl;\nreturn 0;\n}" > printVersion.cpp
mkdir -p build && cd build
cmake ..
make
./printVersion

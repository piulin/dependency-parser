cmake_minimum_required(VERSION 3.15)
project(c__)

set(CMAKE_CXX_STANDARD 17)

add_executable(c__ main.cpp units.h set.h assessment.h parsers/arcstandard/arcstandard.h parsers/arcstandard/model.h parsers/arcstandard/oracle.h parsers/arcstandard/model.cpp parsers/set_parser.h parsers/parser.h parsers/chu_liu_edmonds/chu_liu_edmonds.h parsers/chu_liu_edmonds/model.h parsers/chu_liu_edmonds/feat.h parsers/chu_liu_edmonds/perceptron.cpp parsers/chu_liu_edmonds/perceptron.h parsers/chu_liu_edmonds/matrix.h options.h engine.h assessment.cpp parsers/set_parser.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif(OPENMP_FOUND)
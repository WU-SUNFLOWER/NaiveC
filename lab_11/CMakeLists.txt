cmake_minimum_required(VERSION 3.18)

project(NaiveC)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

# Load LLVM
find_package(LLVM REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH ${LLVM_CMAKE_DIR})

include(AddLLVM)
include_directories( . "${LLVM_BINARY_DIR}/include", "${LLVM_INCLUDE_DIR}")
add_definitions(${LLVM_DEFINITIONS})

if (NOT ${LLVM_ENABLE_RTTI})
    # For non-MSVC compiler
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

set(LLVM_LINK_COMPONENTS Support Core ExecutionEngine MC MCJIT OrcJit native)

aux_source_directory(. DIR_SRCS)
add_llvm_executable(${PROJECT_NAME} ${DIR_SRCS})

add_subdirectory(unit_test)
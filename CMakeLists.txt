cmake_minimum_required(VERSION 3.28)
project(Baregear)
set(APP_VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(LLVM 19.1 REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(include)

find_package(ZLIB REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Network)

# Documentation target
find_program(SPHINX_EXECUTABLE sphinx-build)
if(SPHINX_EXECUTABLE)
    add_custom_target(docs
        COMMAND ${SPHINX_EXECUTABLE} -b html ${CMAKE_SOURCE_DIR}/docs ${CMAKE_BINARY_DIR}/docs
        COMMENT "Building documentation with Sphinx"
        VERBATIM
    )
else()
    message(WARNING "Sphinx not found. Documentation target will not be available.")
endif()

add_subdirectory(lib)
add_subdirectory(src)
project(QTCudaTest)
cmake_minimum_required(VERSION 2.8)
#aux_source_directory(. SRC_LIST)

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
set(CMAKE AUTOUIC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# DeckLink
#find_package(DeckLink REQUIRED)

# QT5
find_package(Qt5Core)

set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-arcs -ftest-coverage")

# set up a mapping so that the Release configuration for the Qt imported target is
# used in the COVERAGE CMake configuration.
set_target_properties(Qt5::Core PROPERTIES MAP_IMPORTED_CONFIG_COVERAGE "DEBUG")

find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)

include_directories(
  ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

if(CMAKE_COMPILER_IS_GNUCXX)
  message("Using GnuCXX compiler.")
  add_definitions("-O0 -std=c++0x -lSOIL -llapacke -lblas")
endif()

find_package(CUDA REQUIRED)
set(CUDA_ATTACH_VS_BUILD_RULE_TO_CUDA_FILE OFF)
set(LIB_TYPE SHARED)
set(CUDA_SEPARABLE_COMPILATION ON)
cuda_add_library(vectorAdd_kernels
    vectorAdd.h
    vectorAdd.cu
)

qt5_wrap_ui(UI_HEADERS mainwindow.ui)

#Build the rest of the executable.
add_executable(${PROJECT_NAME} main.cpp mainwindow.cpp ${UI_HEADERS} vectorAdd.h)

#Remember dependencies!
target_link_libraries(${PROJECT_NAME} Qt5::Gui Qt5::Core Qt5::Widgets vectorAdd_kernels)















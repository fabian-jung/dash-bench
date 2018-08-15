cmake_minimum_required(VERSION 2.6)
project(DashBenchmarks)

set(DASH_DIR ~/lib/dash)


include_directories(${DASH_DIR}/include)
link_directories(${DASH_DIR}/lib)

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

option(ENABLE_LOGGING "Enable Logging" OFF)
option(ENABLE_TRACE_LOGGING "Enable Tracing" OFF)
option(ENABLE_DART_LOGGING "Enable Dart Logging" OFF)

add_executable(dashBench src/main.cpp)
target_link_libraries(dashBench ${MPI_LIBRARIES} dash-mpi dart-mpi dart-base)

if(MPI_COMPILE_FLAGS)
  set_target_properties(dashBench PROPERTIES
    COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
  set_target_properties(dashBench PROPERTIES
    LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()

cmake_minimum_required(VERSION 3.13.4)

find_package(deal.II 9.8.0
  HINTS ${deal.II_DIR}
        ${DEAL_II_DIR}
        ../
        ../../
        $ENV{DEAL_II_DIR}
        /usr/local/deal.II
  )
if(NOT ${deal.II_FOUND})
  message(FATAL_ERROR "\n"
    "*** Could not locate a (sufficiently recent) version of deal.II. ***\n\n"
    "You may want to either pass a flag -DDEAL_II_DIR=/path/to/deal.II to cmake\n"
    "or set an environment variable \"DEAL_II_DIR\" that contains this path."
    )
endif()

if(NOT DEAL_II_WITH_PETSC OR NOT DEAL_II_WITH_SLEPC)
  message(FATAL_ERROR "\n"
    "*** deal.II has to be configured with PETSc and SLEPc (needed for the eigenmode solver). ***\n"
    "See the README for how to rebuild deal.II."
    )
endif()

deal_ii_initialize_cached_variables()
project(chladni CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# All simulation code lives in one library that is shared by the app and the tests.
add_library(chladni_core
  src/parameters.cc
  src/plate.cc
  src/eigenmode_solver.cc
  src/forced_response_solver.cc
  src/output.cc
  src/stress.cc
  )
target_include_directories(chladni_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
deal_ii_setup_target(chladni_core)

add_executable(chladni app/main.cc)
target_link_libraries(chladni chladni_core)
deal_ii_setup_target(chladni)

enable_testing()
add_subdirectory(tests)


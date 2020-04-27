cmake_minimum_required(VERSION 3.9)

set(PROJ_NAME fft_bench)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
project(${PROJ_NAME}
        VERSION ${VERSION} DESCRIPTION
		"Simple FFT bench tests")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#
# Imports
#

set(IMPORT_DIR ${PROJECT_SOURCE_DIR}/import)
include(${PROJECT_SOURCE_DIR}/cmake/ffts.cmake)

#
# Library
#

set(MY_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(MY_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(MY_TEST_DIR ${PROJECT_SOURCE_DIR}/test)

set(MY_SOURCE_FILES
	${MY_SOURCE_DIR}/FftWrapper.cpp
	${PROJECT_SOURCE_DIR}/import/kissfft/kiss_fft.c
	)

set(MY_HEADER_FILES
	)

set(MY_TEST_SOURCES
	${MY_TEST_DIR}/BenchFft.cpp
	${MY_TEST_DIR}/TestFft.cpp
	)

if(MSVC)
  add_compile_options("/W4" "$<$<CONFIG:RELEASE>:/O2>")
else()
  add_compile_options("-Wall"
                      "-Wextra"
                      "-Wold-style-cast"
					  "-Wcast-align"
					  "-Wdouble-promotion"
                      "$<$<CONFIG:RELEASE>:-O3>"
					  )
endif()

add_library(${PROJ_NAME} ${MY_SOURCE_FILES} ${MY_HEADER_FILES})

target_compile_features(
  ${PROJ_NAME} PRIVATE cxx_lambdas cxx_override cxx_range_for)
target_compile_features(
  ${PROJ_NAME} PUBLIC cxx_attributes cxx_func_identifier cxx_noexcept)
target_include_directories(${PROJ_NAME}
                           PUBLIC
                           $<BUILD_INTERFACE:${MY_INCLUDE_DIR}>
                           ${PROJECT_SOURCE_DIR}/import)

option(BUILD_SHARED_LIBS
       "If checked, generate dynamic instead of static libraries." OFF)
include(GenerateExportHeader)
generate_export_header(
  ${PROJ_NAME}
  EXPORT_FILE_NAME
  ${PROJECT_BINARY_DIR}/generated/${PROJ_NAME}/${PROJ_NAME}_export.hpp
  PREFIX_NAME
  DAB_SDR_LIBRARY_)
set(GENERATED_HEADER_FILES
    ${PROJECT_BINARY_DIR}/generated/${PROJ_NAME}/${PROJ_NAME}_export.hpp)
target_include_directories(${PROJ_NAME} PUBLIC
                           $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated>)
if(BUILD_SHARED_LIBS)
  set_property(TARGET ${PROJ_NAME} PROPERTY CXX_VISIBILITY_PRESET hidden)
  set_property(TARGET ${PROJ_NAME} PROPERTY VISIBILITY_INLINES_HIDDEN ON)
  set_property(TARGET ${PROJ_NAME} PROPERTY VERSION ${VERSION})
  set_property(TARGET ${PROJ_NAME} PROPERTY SOVERSION ${VERSION_MAJOR})
endif()

add_executable(${PROJ_NAME}_test ${MY_TEST_SOURCES})
target_link_libraries(${PROJ_NAME}_test PUBLIC ${PROJ_NAME} fftw3f ${FFTS_STATIC})
target_include_directories(${PROJ_NAME}_test
	PUBLIC
	${MY_TEST_DIR}
	${MY_INCLUDE_DIR}
	${IMPORT_DIR}/catch)
#
# Testing
#

option(COVERAGE_INFO
       "If set, line coverage info will be generated from debug test runs." OFF)
option(RUN_TESTS "Determines whether to run tests." OFF)
# option(BUILD_TESTS "Determines whether to build tests." OFF)
# coverage info is only generated after running tests
if(COVERAGE_INFO)
  set(RUN_TESTS On)
endif()
if(RUN_TESTS)
  enable_testing()

  add_test(NAME TestDabSdr COMMAND ${PROJ_NAME}_test)

  if(COVERAGE_INFO AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
    target_compile_options(${PROJ_NAME} PRIVATE --coverage)
    target_link_libraries(${PROJ_NAME} PRIVATE --coverage)
  endif()
endif()

if(RUN_TESTS)
  find_program(LCOV_PATH lcov) # in case of coverage generation we need to reset
                               # counters before running target
  if(COVERAGE_INFO AND LCOV_PATH)
    add_custom_command(TARGET ${PROJ_NAME}_test POST_BUILD
                       COMMAND lcov --directory . --zerocounters
                       COMMENT "Resetting coverage counters..")
  endif()
  add_custom_command(TARGET ${PROJ_NAME}_test POST_BUILD
                     COMMAND "${PROJ_NAME}_test"
                     COMMENT "Running unit tests..") # uses gcovr for "tabular
                                                     # output" and lcov+genhtml
                                                     # for html generation
  if(COVERAGE_INFO)
    find_program(GENHTML_PATH genhtml)
    find_program(GCOVR_PATH gcovr)
    if(NOT LCOV_PATH)
      message(FATAL_ERROR "lcov not found! Aborting...")
    endif()
    if(NOT GENHTML_PATH)
      message(FATAL_ERROR "genhtml not found! Aborting...")
    endif()
    if(NOT GCOVR_PATH)
      message(FATAL_ERROR "gcovr not found! Aborting...")
    endif()
    add_custom_command(TARGET ${PROJ_NAME}_test POST_BUILD
                       COMMAND
                         gcovr
                         --object-directory=CMakeFiles/fft_bench.dir/src/baseband
                         -r ../src
                       WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                       COMMENT "Running gcovr.." VERBATIM)
    add_custom_command(TARGET ${PROJ_NAME}_test POST_BUILD
                       COMMAND lcov -o ${PROJ_NAME}_test.info -c -d
                               CMakeFiles/fft_bench.dir/src/baseband
                       COMMAND genhtml -o coverage ${PROJ_NAME}_test.info
                       WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                       COMMENT "Running lcov and genhtml.." VERBATIM)
    add_custom_command(
      TARGET ${PROJ_NAME}_test POST_BUILD
      COMMAND ;
      COMMENT
        "Open ${CMAKE_BINARY_DIR}/coverage/index.html to view the result in your browser"
      )
  endif()
endif()


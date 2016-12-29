cmake_minimum_required(VERSION 2.8)

project(tcpstream LANGUAGES C)

enable_testing()


set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")
set(LIBRARY_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH})

include_directories(include ${CMAKE_SOURCE_DIR})

if (${COVERAGE})
  if (NOT ${CMAKE_COMPILER_IS_GNUCC})
    message(FATAL_ERROR "Only gcc compiler supported for coverage builds")
  endif()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")
elseif(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3 /GL /Gw /Zi /TC /D_CRT_SECURE_NO_WARNINGS")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT /O2 /Ox")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd /Od")
    set(CMAKE_EXE_LINKER_FLAGS "/INCREMENTAL:NO /LTCG /OPT:REF /OPT:ICF /DEBUG ${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS "/LTCG ${CMAKE_STATIC_LINKER_FLAGS}")
endif()

set(COMMON_INCLUDES "include/vector.h" "include/packet_processing.h")

aux_source_directory(src LIBRARY_SOURCES)

add_library(tcpstream STATIC ${LIBRARY_SOURCES} ${COMMON_INCLUDES})
set_target_properties(tcpstream PROPERTIES LINKER_LANGUAGE C)

set(TEST_NAME ${PROJECT_NAME}_test)
file(GLOB TESTS_SOURCES "tests/*.h" "tests/*.c")
add_executable(${TEST_NAME} ${TESTS_SOURCES} ${COMMON_INCLUDES})
target_link_libraries(${TEST_NAME} tcpstream)
set_target_properties(${TEST_NAME} PROPERTIES LINKER_LANGUAGE C)
add_test(${TEST_NAME} ${EXECUTABLE_OUTPUT_PATH}/${TEST_NAME})

if (${BUILD_EXAMPLES})
    add_subdirectory("example/tcp_stream_dump")
endif()
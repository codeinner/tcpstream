cmake_minimum_required(VERSION 3.0)

project(tcp_stream_dump LANGUAGES C)

if (MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3 /GL /Gw /Zi /TC /D_CRT_SECURE_NO_WARNINGS")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT /O2 /Ox")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd /Od")
    set(CMAKE_EXE_LINKER_FLAGS "/INCREMENTAL:NO /LTCG /OPT:REF /OPT:ICF /DEBUG ${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS "/LTCG ${CMAKE_STATIC_LINKER_FLAGS}")

    include_directories(ext/wpcap/include)

    set(CMAKE_LIBRARY_PATH "ext/wpcap/lib")

    find_library(PCAP_LIBRARY wpcap)
else()
    find_library(PCAP_LIBRARY pcap)
endif()

add_executable( tcp_stream_dump
                main.c
                sniffer.c
                sniffer.h)
set_target_properties(tcp_stream_dump PROPERTIES LINKER_LANGUAGE C)
if (MSVC)
    set_target_properties(tcp_stream_dump PROPERTIES LINK_FLAGS "/level='requireAdministrator' /uiAccess='false'")
endif()
target_link_libraries(tcp_stream_dump tcpstream ${PCAP_LIBRARY})
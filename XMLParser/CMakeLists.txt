cmake_minimum_required(VERSION 3.15)
project(JSBSimEnfusionUtils LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(JSBSimXMLParser
    src/main.cpp
    src/AerodynamicsParser.hpp
    src/FlightControlParser.hpp
    src/GuidGenerator.hpp
    src/MetricsParser.hpp
    src/XmlParser.hpp
)

target_include_directories(JSBSimXMLParser PRIVATE src)

if (MSVC)
    target_compile_options(JSBSimXMLParser PRIVATE /W3 /EHsc /utf-8)
    target_link_libraries(JSBSimXMLParser PRIVATE Shell32)
endif()

set(CMAKE_BUILD_TYPE Release)

cmake_minimum_required(VERSION 2.6)
project(SIMRAM)

set(MAIN_SRCS main.cpp)
add_executable(simram ${MAIN_SRCS})

set(BOOST_LIBS program_options system)
find_package(Boost COMPONENTS ${BOOST_LIBS} REQUIRED)

target_link_libraries(simram ${Boost_LIBRARIES})

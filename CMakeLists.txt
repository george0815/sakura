cmake_minimum_required(VERSION 3.16)


project(sakura)

set (CMAKE_CXX_STANDARD 17)

set (BOOST_REQUIRED_COMPONENTS
program_options)

find_package(SDL2 REQUIRED)
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED
  COMPONENTS ${BOOST_REQUIRED_COMPONENTS})


#collect all surce file
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS 
  "MAIN/*.cpp"
  "CORE/*.cpp"
)


add_executable(sakura ${SRC})

target_link_libraries(sakura SDL2::SDL2 Boost::program_options)


#include directories
target_include_directories(sakura PRIVATE CORE)

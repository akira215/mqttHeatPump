# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)


#set(EXTRA_COMPONENT_DIRS "../components/periodicTask" "../components/cppgpio")

set(COMPONENTS   main)
set(EXCLUDE_COMPONENTS matter zigbee modbus)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(testWifi)
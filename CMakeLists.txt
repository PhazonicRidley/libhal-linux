# Copyright 2024 Khalil Estell
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.20)

project(linux_demos VERSION 0.0.1 LANGUAGES CXX)

# Generate compile commands for anyone using our libraries.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Always run this custom target by making it depend on ALL
add_custom_target(copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json
    DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json)


find_package(libhal REQUIRED CONFIG)
find_package(libhal-util REQUIRED CONFIG)

set(DEMOS gpio hello i2c_test uart steady_clock_test)
foreach(DEMO ${DEMOS})
    message(STATUS "Generating Demo for \"${PROJECT_NAME}_${DEMO}")
    add_executable(${PROJECT_NAME}_${DEMO} demos/${DEMO}.cpp)
    target_include_directories(${PROJECT_NAME}_${DEMO} PUBLIC .)
    target_compile_features(${PROJECT_NAME}_${DEMO} PRIVATE cxx_std_23)
    target_link_libraries(${PROJECT_NAME}_${DEMO} PRIVATE libhal::libhal libhal::util -static-libstdc++)
#target_link_options(${PROJECT_NAME}_${DEMO} PRIVATE "-lgpiodcxx")
    
endforeach()


